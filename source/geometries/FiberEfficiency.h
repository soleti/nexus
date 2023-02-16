// ----------------------------------------------------------------------------
// nexus | FiberEfficiency.h
//
// Box-shaped box of material with a coating.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef FiberEfficiency_H
#define FiberEfficiency_H

#include "GeometryBase.h"
#include "GenericWLSFiber.h"
#include "PmtR11410.h"
#include "CylinderPointSampler2020.h"
#include "MaterialsList.h"
#include "PmtR11410.h"
#include "PmtR7378A.h"
#include "GenericPhotosensor.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }


namespace nexus {

  /// Spherical chamber filled with xenon (liquid or gas)

  class FiberEfficiency: public GeometryBase
  {
  public:
    /// Constructor
    FiberEfficiency();
    /// Destructor
    ~FiberEfficiency();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& region) const;

    void Construct();

  private:
    G4double world_z_;             // World dimensions
    G4double world_xy_;
    G4double fiber_diameter_;
    G4double length_;
    G4String coating_;
    G4String fiber_type_;
    G4String detector_type_;
    G4bool with_grease_;
    G4bool coated_;

    GenericPhotosensor* pmt_;
    GenericWLSFiber* fiber_;
    CylinderPointSampler2020* inside_cylinder_;
    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif