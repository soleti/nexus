// ----------------------------------------------------------------------------
// nexus | KZenChimney.cc
//
// Sphere filled with xenon.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "KZenChimney.h"

#include "SpherePointSampler.h"
#include "MaterialsList.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
#include "Visibilities.h"
#include "GenericWLSFiber.h"

#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4VUserDetectorConstruction.hh>

#include <CLHEP/Units/SystemOfUnits.h>

using namespace nexus;
using namespace CLHEP;

REGISTER_CLASS(KZenChimney, GeometryBase)

namespace nexus
{

  KZenChimney::KZenChimney() : GeometryBase(),
                               chimney_radius_(1900. / 2. * mm),
                               chimney_thickness_(1 * cm),
                               chimney_height_(3065 * mm)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/KZenChimney/",
                                  "Control commands of geometry KZenChimney.");

    G4GenericMessenger::Command &chimney_height_cmd =
        msg_->DeclareProperty("chimney_height", chimney_height_, "Height of the chimney.");
    chimney_height_cmd.SetUnitCategory("Length");
    chimney_height_cmd.SetParameterName("chimney_height", false);
    chimney_height_cmd.SetRange("chimney_height>0.");

    G4GenericMessenger::Command &chimney_radius_cmd =
        msg_->DeclareProperty("chimney_radius", chimney_radius_, "Radius of the chimney.");
    chimney_radius_cmd.SetUnitCategory("Length");
    chimney_radius_cmd.SetParameterName("chimney_radius", false);
    chimney_radius_cmd.SetRange("chimney_radius>0.");

    G4GenericMessenger::Command &chimney_thickness_cmd =
        msg_->DeclareProperty("chimney_thickness", chimney_thickness_, "Thickness of the chimney wall.");
    chimney_thickness_cmd.SetUnitCategory("Length");
    chimney_thickness_cmd.SetParameterName("chimney_thickness", false);
    chimney_thickness_cmd.SetRange("chimney_thickness>0.");

  }

  KZenChimney::~KZenChimney()
  {
    delete msg_;
  }

  void KZenChimney::Construct()
  {
    G4String name = "KZEN_CHIMNEY";

    G4Tubs *lab = new G4Tubs("LAB", 0., chimney_radius_ + chimney_thickness_, chimney_height_ + 1 * cm, 0., twopi);
    G4Material *lab_material = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    G4LogicalVolume *lab_logic = new G4LogicalVolume(lab, lab_material, "LAB");
    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    GeometryBase::SetLogicalVolume(lab_logic);

    // Define solid volume as a sphere
    G4Tubs *chimney_solid = new G4Tubs(name, chimney_radius_, chimney_radius_ + chimney_thickness_ / 2., chimney_height_, 0., twopi);

    // Define the material (LXe or GXe) for the sphere.
    // We use for this the NIST manager or the nexus materials list.
    G4Material *steel = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    // Define the logical volume of the sphere using the material
    // and the solid volume defined above
    G4LogicalVolume *chimney_logic =
        new G4LogicalVolume(chimney_solid, steel, name);

    // Define the visual attributes of the sphere
    chimney_logic->SetVisAttributes(nexus::WhiteAlpha());
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), chimney_logic, "CHIMNEY", lab_logic, false, 0, false);

    G4Tubs *ls_solid = new G4Tubs("LS", 0., chimney_radius_, chimney_height_, 0., twopi);
    G4Material *ls_material = materials::KZenLS();
    G4LogicalVolume *ls_logic = new G4LogicalVolume(ls_solid, ls_material, "LS");
    ls_logic->SetVisAttributes(nexus::LightBlueAlpha());
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), ls_logic, "LS", lab_logic, false, 0, false);


    G4Box *ptfe_panel = new G4Box("PTFE_PANEL", 12 * cm / 2, chimney_radius_ / 2, 1 * cm);
    G4Material *ptfe_material = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
    G4LogicalVolume *ptfe_logic = new G4LogicalVolume(ptfe_panel, ptfe_material, "PTFE_PANEL");
    ptfe_logic->SetVisAttributes(nexus::White());
    for (int i = 0; i < 12; i++)
    {
      new G4PVPlacement(0, G4ThreeVector(i*14*cm, -chimney_radius_ /2, chimney_height_ / 2), ptfe_logic, "PTFE_PANEL", lab_logic, false, 0, false);
    }

    // G4double fiber_length = chimney_radius_ / 2;
    // G4double fiber_diameter = 1 * mm;
    // GenericWLSFiber *fiber_ = new GenericWLSFiber("B2", true, true, fiber_diameter, fiber_length, true, false, materials::OpticalSilicone(), materials::OpticalSilicone(), true);
    // // fiber_->SetCoreOpticalProperties(opticalprops::B2());
    // fiber_->Construct();
    // G4LogicalVolume *fiber_logic = fiber_->GetLogicalVolume();
    // fiber_logic->SetVisAttributes(nexus::LightBlueAlpha());
    // G4RotationMatrix* pRot = new G4RotationMatrix();
    // pRot->rotateX(90 * deg);
    // for (int i = 0; i < 8; i++)
    // {
    //   new G4PVPlacement(pRot, G4ThreeVector(i * 1 * cm, 0, 0), fiber_logic, "FIBER", ls_logic, true, 0, true);
    // }


    // // Set the logical volume of the sphere as an ionization
    // // sensitive detector, i.e. position, time and energy deposition
    // // will be stored for each step of any charged particle crossing
    // // the volume.
    // IonizationSD *ionizsd = new IonizationSD("/XE_SPHERE");
    // G4SDManager::GetSDMpointer()->AddNewDetector(ionizsd);
    // sphere_logic->SetSensitiveDetector(ionizsd);
  }

  G4ThreeVector KZenChimney::GenerateVertex(const G4String & /*region*/) const
  {
    return G4ThreeVector(0., 0., 0.);
  }

} // end namespace nexus
