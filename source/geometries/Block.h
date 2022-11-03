// ----------------------------------------------------------------------------
// nexus | Block.h
//
// Box-shaped box of material with a coating.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef BLOCK_H
#define BLOCK_H

#include "GeometryBase.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }


namespace nexus {

  /// Spherical chamber filled with xenon (liquid or gas)

  class Block: public GeometryBase
  {
  public:
    /// Constructor
    Block();
    /// Destructor
    ~Block();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& region) const;

    void Construct();

  private:
    G4double world_z_;             // World dimensions
    G4double world_xy_;
    G4double box_x_;               // Box three dimensions
    G4double box_y_;
    G4double box_z_;
    G4String box_material_;        // Box material
    G4String box_optical_coating_; // Box optical coating

    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
