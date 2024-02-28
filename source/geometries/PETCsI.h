// ----------------------------------------------------------------------------
// nexus | PETCsI.h
//

#ifndef PETCsI_H
#define PETCsI_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"
#include "SpherePointSampler.h"
#include "JaszczakPhantom.h"

class G4GenericMessenger;

namespace nexus {

  class PETCsI: public GeometryBase {
  public:
    /// Constructor
    PETCsI();

    /// Destructor
    ~PETCsI();

    void Construct();
    G4ThreeVector GenerateVertex(const G4String& region) const;


  private:

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;
    SpherePointSampler* sphere1_;
    SpherePointSampler* sphere2_;
    JaszczakPhantom* jas_phantom_;

    // Dimension of the crystals
    G4String crystal_material_;
    G4double crystal_width_;
    G4double crystal_length_;
    G4double pet_diameter_;
    G4double pet_length_;
  };
}
#endif
