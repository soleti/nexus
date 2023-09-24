#ifndef FluxMonitor_H
#define FluxMonitor_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"

class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }

class G4GenericMessenger;

namespace nexus {

  class FluxMonitor : public GeometryBase
  {
  public:
    ///Constructor
    FluxMonitor();

    ///Destructor
    ~FluxMonitor();

    void Construct();

    /// Generates a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    
  private:

    G4double detector_height_;
    G4double detector_diam_;

    CylinderPointSampler2020* inside_cylinder_;
    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };
}

#endif