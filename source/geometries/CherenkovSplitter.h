#ifndef CherenkovSplitter_H
#define CherenkovSplitter_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"

class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }

class G4GenericMessenger;

namespace nexus {

  class CherenkovSplitter : public GeometryBase
  {
  public:
    ///Constructor
    CherenkovSplitter();

    ///Destructor
    ~CherenkovSplitter();

    void Construct();

    /// Generates a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    
  private:


    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };
}

#endif