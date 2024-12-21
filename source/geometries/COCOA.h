#ifndef COCOA_H
#define COCOA_H

#include "GeometryBase.h"
#include "BoxPointSampler.h"

class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }

class G4GenericMessenger;

namespace nexus {

  class COCOA : public GeometryBase
  {
  public:
    ///Constructor
    COCOA();

    ///Destructor
    ~COCOA();

    void Construct();

    /// Generates a vertex within a given region of the geometry
    G4ThreeVector GenerateVertex(const G4String& region) const;

    
  private:

    G4double detector_height_;
    G4double detector_width_;
    G4double detector_thickness_;

    BoxPointSampler* gamma_plane_;
    BoxPointSampler* scatterer_volume_;
    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };
}

#endif