#ifndef ESSBeam_H
#define ESSBeam_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"

class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }

class G4GenericMessenger;

namespace nexus {

  class ESSBeam : public GeometryBase
  {
  public:
    ///Constructor
    ESSBeam();

    ///Destructor
    ~ESSBeam();

    void Construct();

    /// Generates a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    
  private:

    G4double detector_height_;
    G4double detector_diam_;
    G4bool beam_;

    CylinderPointSampler2020* inside_cylinder_;
    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };
}

#endif