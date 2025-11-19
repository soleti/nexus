// ----------------------------------------------------------------------------
// nexus | DefaultEventAction.h
//
// This is the default event action of the NEXT simulations. Only events with
// deposited energy larger than 0 are saved in the nexus output file.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef DEFAULT_EVENT_ACTION_H
#define DEFAULT_EVENT_ACTION_H

#include <G4UserEventAction.hh>
#include <globals.hh>

#include <vector>

class G4Event;
class G4GenericMessenger;

namespace nexus {
  class IonizationHit;

  /// This class is a general-purpose event run action.

  class DefaultEventAction: public G4UserEventAction
  {
  public:
    /// Constructor
    DefaultEventAction();
    /// Destructor
    ~DefaultEventAction();

    /// Hook at the beginning of the event loop
    void BeginOfEventAction(const G4Event*);
    /// Hook at the end of the event loop
    void EndOfEventAction(const G4Event*);

  private:
    std::vector<IonizationHit*> CollectIonizationHits(const G4Event*) const;
    std::vector<size_t> RegionQuery(const std::vector<IonizationHit*>& hits,
                                    size_t point_index,
                                    G4double eps_sq) const;
    void ExpandCluster(size_t seed_index,
                       std::vector<size_t>& neighbor_indices,
                       G4int cluster_id,
                       std::vector<G4int>& labels,
                       std::vector<IonizationHit*>& hits,
                       G4double eps_sq) const;
    G4int RunDbscan(std::vector<IonizationHit*>& hits) const;

  private:
    G4GenericMessenger* msg_;
    G4int nevt_, nupdate_;
    G4double energy_min_;
    G4double energy_max_;
    G4bool clustering_enabled_;
    G4double cluster_radius_;
    G4int cluster_min_points_;
    G4int store_max_clusters_;
  };

} // namespace nexus

#endif
