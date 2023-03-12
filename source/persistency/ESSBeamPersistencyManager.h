#ifndef ESSBEAM_PERSISTENCY_MANAGER_H
#define ESSBEAM_PERSISTENCY_MANAGER_H

#include "PersistencyManagerBase.h"
#include "ESSBeamHDF5Writer.h"

class G4GenericMessenger;
class G4TrajectoryContainer;
class G4HCofThisEvent;
class G4VHitsCollection;

// namespace nexus { class ESSBeamHDF5Writer; }

namespace nexus {

  class ESSBeamPersistencyManager: public PersistencyManagerBase
  {
  public:
    ESSBeamPersistencyManager();
    ~ESSBeamPersistencyManager();
    ESSBeamPersistencyManager(const ESSBeamPersistencyManager&);
    /// Create the singleton instance of the persistency manager
    // static void Initialize(G4String historyFile_init, G4String historyFile_conf);

    /// Set whether to store or not the current event
    void StoreCurrentEvent(G4bool);
    void InteractingEvent(G4bool);

    ///
    virtual G4bool Store(const G4Event*);
    virtual G4bool Store(const G4Run*);
    virtual G4bool Store(const G4VPhysicalVolume*);

    virtual G4bool Retrieve(G4Event*&);
    virtual G4bool Retrieve(G4Run*&);
    virtual G4bool Retrieve(G4VPhysicalVolume*&);

  public:
    void OpenFile(G4String);
    void CloseFile();


  private:

    void StoreTrajectories(G4TrajectoryContainer*);
    void SaveConfigurationInfo(G4String history);


  private:
    G4GenericMessenger* _msg; ///< User configuration messenger


    G4bool _ready;     ///< Is the ESSBeamPersistencyManager ready to go?
    G4bool _store_evt; ///< Should we store the current event?
    G4bool _interacting_evt; ///< Has the current event interacted in ACTIVE?

    G4String event_type_; ///< event type: bb0nu, bb2nu, background or not set

    G4int _saved_evts; ///< number of events to be saved

    G4int _nevt; ///< Event ID
    G4int _start_id; ///< ID for the first event in file
    G4bool _first_evt; ///< true only for the first event of the run

    ESSBeamHDF5Writer* _h5writer;  ///< Event writer to hdf5 file

    G4double _bin_size, _tof_bin_size;

  };


  // INLINE DEFINITIONS //////////////////////////////////////////////

  inline void ESSBeamPersistencyManager::StoreCurrentEvent(G4bool sce)
  { _store_evt = sce; }
  inline void ESSBeamPersistencyManager::InteractingEvent(G4bool ie)
  { _interacting_evt = ie; }
  inline G4bool ESSBeamPersistencyManager::Store(const G4VPhysicalVolume*)
  { return false; }
  inline G4bool ESSBeamPersistencyManager::Retrieve(G4Event*&)
  { return false; }
  inline G4bool ESSBeamPersistencyManager::Retrieve(G4Run*&)
  { return false; }
  inline G4bool ESSBeamPersistencyManager::Retrieve(G4VPhysicalVolume*&)
  { return false; }

} // namespace nexus

#endif