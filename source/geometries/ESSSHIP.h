#ifndef ESSSHIP_H
#define ESSSHIP_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"

class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }

class G4GenericMessenger;

namespace nexus {

  class ESSSHIP : public GeometryBase
  {
  public:
    ///Constructor
    ESSSHIP();

    ///Destructor
    ~ESSSHIP();

    void Construct();

    /// Generates a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    
  private:

    G4bool beam_;
    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };
}

#endif