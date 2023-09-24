// ----------------------------------------------------------------------------
// nexus | FibBarrMeth.h
//
// Box-shaped box of material with a coating.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef FibBarrMeth_H
#define FibBarrMeth_H

#include "GeometryBase.h"
#include "GenericWLSFiber.h"
#include "GenericCircularPhotosensor.h"
#include "CylinderPointSampler2020.h"
#include "MaterialsList.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }


namespace nexus {

  /// Fiber Barrel

  class FibBarrMeth: public GeometryBase
  {
  public:
    /// Constructor
    FibBarrMeth();
    /// Destructor
    ~FibBarrMeth();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& region) const;

    void Construct();

  private:
    G4double world_z_;             // World dimensions
    G4double world_xy_;
    G4double radius_;
    G4double fiber_diameter_;
    G4double length_;
    G4String coating_;
    G4String fiber_type_;
    G4bool coated_;

    // cylinder
    G4double teflon_thickness_;   // thickness of the outer teflon cover
    G4bool caps_visibility_;
    G4bool teflon_visibility_;

    // methacrylate
    G4bool methacrylate_;
    G4double window_thickness_;

    // sensor
    GenericCircularPhotosensor* photo_sensor_;
    G4String sensor_type_;        // SiPM, PMT, PERFECT, ...
    G4bool sensor_visibility_;

    GenericWLSFiber* fiber_;
    CylinderPointSampler2020* cyl_vertex_gen_;
    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
