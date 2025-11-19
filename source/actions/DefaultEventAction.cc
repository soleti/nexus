// ----------------------------------------------------------------------------
// nexus | DefaultEventAction.cc
//
// This is the default event action of the NEXT simulations. Only events with
// deposited energy larger than 0 are saved in the nexus output file.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "DefaultEventAction.h"
#include "Trajectory.h"
#include "PersistencyManager.h"
#include "IonizationHit.h"
#include "IonizationSD.h"
#include "FactoryBase.h"

#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Event.hh>
#include <G4VVisManager.hh>
#include <G4Trajectory.hh>
#include <G4GenericMessenger.hh>
#include <G4HCofThisEvent.hh>
#include <G4SDManager.hh>
#include <G4HCtable.hh>
#include <globals.hh>


namespace nexus {

namespace {
  constexpr G4int DBSCAN_NOISE = -1;
  constexpr G4int DBSCAN_UNCLASSIFIED = -2;
}

REGISTER_CLASS(DefaultEventAction, G4UserEventAction)

  DefaultEventAction::DefaultEventAction():
    G4UserEventAction(), nevt_(0), nupdate_(10), energy_min_(0.), energy_max_(DBL_MAX),
    clustering_enabled_(true), cluster_radius_(5.*CLHEP::mm), cluster_min_points_(3),
    store_max_clusters_(-1)
  {
    msg_ = new G4GenericMessenger(this, "/Actions/DefaultEventAction/");

    G4GenericMessenger::Command& thresh_cmd =
       msg_->DeclareProperty("min_energy", energy_min_,
                             "Minimum deposited energy to save the event to file.");
    thresh_cmd.SetParameterName("min_energy", true);
    thresh_cmd.SetUnitCategory("Energy");
    thresh_cmd.SetRange("min_energy>0.");

    G4GenericMessenger::Command& max_energy_cmd =
      msg_->DeclareProperty("max_energy", energy_max_,
                            "Maximum deposited energy to save the event to file.");
    max_energy_cmd.SetParameterName("max_energy", true);
   max_energy_cmd.SetUnitCategory("Energy");
    max_energy_cmd.SetRange("max_energy>0.");

    msg_->DeclareProperty("enable_clustering", clustering_enabled_,
                          "Enable DBSCAN clustering of ionization hits.");

    G4GenericMessenger::Command& radius_cmd =
      msg_->DeclarePropertyWithUnit("cluster_radius", "mm", cluster_radius_,
                                    "Spatial radius used by the DBSCAN algorithm.");
    radius_cmd.SetParameterName("cluster_radius", true);
    radius_cmd.SetRange("cluster_radius>0.");

    G4GenericMessenger::Command& min_hits_cmd =
      msg_->DeclareProperty("cluster_min_hits", cluster_min_points_,
                            "Minimum number of hits required to seed a DBSCAN cluster.");
    min_hits_cmd.SetParameterName("cluster_min_hits", true);
    min_hits_cmd.SetRange("cluster_min_hits>=1");

    G4GenericMessenger::Command& store_cluster_cmd =
      msg_->DeclareProperty("store_max_clusters", store_max_clusters_,
                            "Store events only if the DBSCAN cluster count is below this value (-1 disables).");
    store_cluster_cmd.SetParameterName("store_max_clusters", true);
    store_cluster_cmd.SetRange("store_max_clusters>=-1");

    PersistencyManager* pm = dynamic_cast<PersistencyManager*>
      (G4VPersistencyManager::GetPersistencyManager());

    pm->SaveNumbOfInteractingEvents(true);
  }



  DefaultEventAction::~DefaultEventAction()
  {
  }



  void DefaultEventAction::BeginOfEventAction(const G4Event* /*event*/)
  {
    // Print out event number info
    if ((nevt_ % nupdate_) == 0) {
      G4cout << " >> Event no. " << nevt_  << G4endl;
      if (nevt_  == (10 * nupdate_)) nupdate_ *= 10;
    }
  }



  void DefaultEventAction::EndOfEventAction(const G4Event* event)
  {
    nevt_++;

    PersistencyManager* pm = dynamic_cast<PersistencyManager*>
      (G4VPersistencyManager::GetPersistencyManager());

    if (pm) {
      pm->ClusteredEvent(0);
    }

    // Determine whether total energy deposit in ionization sensitive
    // detectors is above threshold
    if (energy_min_ >= 0.) {

      // Get the trajectories stored for this event and loop through them
      // to calculate the total energy deposit

      G4double edep = 0.;

      G4TrajectoryContainer* tc = event->GetTrajectoryContainer();
      if (tc) {
        // in interactive mode, a G4TrajectoryContainer would exist
        // but the trajectories will not cast to Trajectory
        Trajectory* trj = dynamic_cast<Trajectory*>((*tc)[0]);
        if (trj == nullptr){
          G4Exception("[DefaultEventAction]", "EndOfEventAction()", FatalException,
                      "The trajectory container is empty. If you are simulating optical photons as primary particles,"
                      " and not using OpticalTrackingAction, you should use the G4 default event action.");
        }
        for (unsigned int i=0; i<tc->size(); ++i) {
          Trajectory* tr = dynamic_cast<Trajectory*>((*tc)[i]);
          edep += tr->GetEnergyDeposit();
        }
      }
      else {
        G4Exception("[DefaultEventAction]", "EndOfEventAction()", FatalException,
                    "The trajectory container doesn't exist. Check that you are using DefaultTrackingAction."
                    " Notice that, if you are simulating optical photons as primary particles, "
                    "and not using OpticalTrackingAction, you should not specify any event actions.");
      }

      if (pm) {
        if (!event->IsAborted() && edep>0) {
          pm->InteractingEvent(true);
        } else {
          pm->InteractingEvent(false);
        }
        if (!event->IsAborted() && edep > energy_min_ && edep < energy_max_) {
          pm->StoreCurrentEvent(true);
        } else {
          pm->StoreCurrentEvent(false);
        }
      }

    }

    if (!pm || !clustering_enabled_ || event->IsAborted())
      return;

    if (cluster_radius_ <= 0. || cluster_min_points_ <= 0)
      return;

    std::vector<IonizationHit*> hits = CollectIonizationHits(event);
    if (hits.empty())
      return;
    G4int cluster_count = RunDbscan(hits);
    // G4cout << "clusters: " << cluster_count << G4endl;
    pm->ClusteredEvent(cluster_count);
    if (store_max_clusters_ >= 0 && cluster_count >= store_max_clusters_) {
      pm->StoreCurrentEvent(false);
    }
  }

  std::vector<IonizationHit*> DefaultEventAction::CollectIonizationHits(const G4Event* event) const
  {
    std::vector<IonizationHit*> hits;
    G4HCofThisEvent* hce = event->GetHCofThisEvent();
    if (!hce)
      return hits;

    G4SDManager* sdmgr = G4SDManager::GetSDMpointer();
    G4HCtable* hct = sdmgr->GetHCtable();
    if (!hct)
      return hits;

    for (int i=0; i<hct->entries(); ++i) {
      G4String hcname = hct->GetHCname(i);
      if (hcname != IonizationSD::GetCollectionUniqueName())
        continue;

      G4String sdname = hct->GetSDname(i);
      int hcid = sdmgr->GetCollectionID(sdname+"/"+hcname);
      if (hcid < 0)
        continue;

      G4VHitsCollection* collection = hce->GetHC(hcid);
      auto ion_hits = dynamic_cast<IonizationHitsCollection*>(collection);
      if (!ion_hits)
        continue;

      for (size_t j=0; j<ion_hits->entries(); ++j) {
        IonizationHit* hit = dynamic_cast<IonizationHit*>(ion_hits->GetHit(j));
        if (!hit)
          continue;
        hits.push_back(hit);
      }
    }
    return hits;
  }

  std::vector<size_t> DefaultEventAction::RegionQuery(const std::vector<IonizationHit*>& hits,
                                                      size_t point_index,
                                                      G4double eps_sq) const
  {
    std::vector<size_t> neighbors;
    const G4ThreeVector reference = hits[point_index]->GetPosition();
    for (size_t i=0; i<hits.size(); ++i) {
      const G4ThreeVector delta = hits[i]->GetPosition() - reference;
      if (delta.mag2() <= eps_sq) {
        neighbors.push_back(i);
      }
    }
    return neighbors;
  }

  void DefaultEventAction::ExpandCluster(size_t seed_index,
                                         std::vector<size_t>& neighbor_indices,
                                         G4int cluster_id,
                                         std::vector<G4int>& labels,
                                         std::vector<IonizationHit*>& hits,
                                         G4double eps_sq) const
  {
    labels[seed_index] = cluster_id;
    hits[seed_index]->SetClusterID(cluster_id);

    for (size_t idx = 0; idx < neighbor_indices.size(); ++idx) {
      const size_t neighbor = neighbor_indices[idx];

      if (labels[neighbor] == DBSCAN_NOISE) {
        labels[neighbor] = cluster_id;
        hits[neighbor]->SetClusterID(cluster_id);
      }

      if (labels[neighbor] != DBSCAN_UNCLASSIFIED)
        continue;

      labels[neighbor] = cluster_id;
      hits[neighbor]->SetClusterID(cluster_id);

      std::vector<size_t> neighbor_neighbors =
        RegionQuery(hits, neighbor, eps_sq);

      if (neighbor_neighbors.size() >= static_cast<size_t>(cluster_min_points_)) {
        neighbor_indices.insert(neighbor_indices.end(),
                                neighbor_neighbors.begin(),
                                neighbor_neighbors.end());
      }
    }
  }

  G4int DefaultEventAction::RunDbscan(std::vector<IonizationHit*>& hits) const
  {
    const size_t num_hits = hits.size();
    if (num_hits == 0)
      return 0;

    const G4double eps_sq = cluster_radius_ * cluster_radius_;
    std::vector<G4int> labels(num_hits, DBSCAN_UNCLASSIFIED);

    for (IonizationHit* hit : hits) {
      hit->SetClusterID(DBSCAN_NOISE);
    }

    G4int current_cluster = 0;
    for (size_t i=0; i<num_hits; ++i) {
      if (labels[i] != DBSCAN_UNCLASSIFIED)
        continue;

      std::vector<size_t> neighbors = RegionQuery(hits, i, eps_sq);
      if (neighbors.size() < static_cast<size_t>(cluster_min_points_)) {
        labels[i] = DBSCAN_NOISE;
        hits[i]->SetClusterID(DBSCAN_NOISE);
        continue;
      }

      ExpandCluster(i, neighbors, current_cluster, labels, hits, eps_sq);
      current_cluster++;
    }

    for (size_t i=0; i<num_hits; ++i) {
      if (labels[i] == DBSCAN_UNCLASSIFIED) {
        labels[i] = DBSCAN_NOISE;
        hits[i]->SetClusterID(DBSCAN_NOISE);
      }
    }

    return current_cluster;
  }

} // end namespace nexus
