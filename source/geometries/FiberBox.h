// ----------------------------------------------------------------------------
// nexus | FiberBox.h
//
// Box-shaped box of material with a coating.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef FiberBox_H
#define FiberBox_H

#include "GeometryBase.h"
#include "GenericWLSFiber.h"
#include "PmtR11410.h"
#include "CylinderPointSampler2020.h"
#include "MaterialsList.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }


namespace nexus {

  /// Spherical chamber filled with xenon (liquid or gas)

  class FiberBox: public GeometryBase
  {
  public:
    /// Constructor
    FiberBox();
    /// Destructor
    ~FiberBox();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& region) const;

    void Construct();

  private:
    G4double world_z_;             // World dimensions
    G4double world_xy_;
    G4double radius_;
    G4double fiber_radius_;
    G4double length_;
    G4String coating_;
    G4String fiber_type_;
    G4bool coated_;

    GenericWLSFiber* fiber_;
    CylinderPointSampler2020* inside_cylinder_;
    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif