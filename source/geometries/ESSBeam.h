#ifndef ESSBeam_H
#define ESSBeam_H

#include "GeometryBase.h"
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

    // Messenger for the definition of control commands
    G4GenericMessenger* _msg;

  };
}

#endif