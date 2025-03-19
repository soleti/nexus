// ----------------------------------------------------------------------------
// nexus | KZenChimney.cc
//
// KamLAND-Zen chimney
//
// ----------------------------------------------------------------------------

#include "KZenChimney.h"

#include "SpherePointSampler.h"
#include "MaterialsList.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
#include "Visibilities.h"
#include "GenericWLSFiber.h"
#include "OpticalMaterialProperties.h"
#include "GenericPhotosensor.h"

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
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>

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

    // Add optical surface to chimney_solid
    G4OpticalSurface *op_surface = new G4OpticalSurface("CHIMNEY_OPSURFACE", unified, ground, dielectric_metal);
    op_surface->SetMaterialPropertiesTable(opticalprops::Steel());
    new G4LogicalSkinSurface("CHIMNEY_OPSURFACE", chimney_logic, op_surface);

    chimney_logic->SetVisAttributes(nexus::WhiteAlpha());
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), chimney_logic, "CHIMNEY", lab_logic, false, 0, false);

    G4Tubs *ls_solid = new G4Tubs("LS", 0., chimney_radius_, chimney_height_, 0., twopi);
    G4Material *ls_material = materials::KZenLS();
    ls_material->SetMaterialPropertiesTable(opticalprops::KZenLS());
    G4LogicalVolume *ls_logic = new G4LogicalVolume(ls_solid, ls_material, "LS");
    ls_logic->SetVisAttributes(nexus::LightBlueAlpha());
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), ls_logic, "LS", lab_logic, false, 0, false);

    G4double panel_size = 12 * cm;
    G4double panel_padding = 0.5 * cm;
    G4double panel_total = panel_size + panel_padding * 2;
    G4double panel_thickness = 1 * cm;

    G4int n_panels = chimney_radius_ * 2 / panel_total;
    G4double chimney_padding = 5 * cm;

    G4Material *ptfe_material = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
    ptfe_material->SetMaterialPropertiesTable(opticalprops::PTFE());
    G4double fiber_diameter = 1 * mm;

    GenericPhotosensor *photosensor = new GenericPhotosensor("SIPM_FIBER", panel_size, fiber_diameter, 1 * mm);
    const G4int entries = 9;
    G4double eff_energies[entries] =
        {
            h_Planck * c_light / (900.59 * nm), h_Planck * c_light / (732.07 * nm),
            h_Planck * c_light / (598.58 * nm), h_Planck * c_light / (501.07 * nm),
            h_Planck * c_light / (457.51 * nm), h_Planck * c_light / (419.64 * nm),
            h_Planck * c_light / (371.36 * nm), h_Planck * c_light / (339.17 * nm),
            h_Planck * c_light / (319.29 * nm)};
    G4double eff[entries] =
        {
            0.04, 0.15,
            0.33, 0.48,
            0.50, 0.47,
            0.34, 0.17,
            0.03};

    G4MaterialPropertiesTable *photosensor_mpt = new G4MaterialPropertiesTable();
    photosensor_mpt->AddProperty("EFFICIENCY", eff_energies, eff, entries);
    photosensor->SetVisibility(true);
    photosensor->SetWindowRefractiveIndex(opticalprops::OptCoupler()->GetProperty("RINDEX"));
    photosensor->SetOpticalProperties(photosensor_mpt);
    photosensor->SetSensorDepth(1);
    photosensor->Construct();
    G4LogicalVolume *photosensor_logic = photosensor->GetLogicalVolume();
    G4RotationMatrix *pRot = new G4RotationMatrix();
    pRot->rotateX(90 * deg);
    G4RotationMatrix *pRotz = new G4RotationMatrix();
    pRotz->rotateX(-90 * deg);

    for (int i = -n_panels / 2; i < n_panels / 2; i++)
    {
      G4double x_coord = i * (panel_size + panel_padding) + panel_total / 2;
      G4double y_edge;

      if (x_coord < 0)
        y_edge = sqrt(pow(chimney_radius_ - chimney_padding, 2) - pow(x_coord - panel_size / 2, 2));
      else
        y_edge = sqrt(pow(chimney_radius_ - chimney_padding, 2) - pow(x_coord + panel_size / 2, 2));

      G4double y_cut = 0;
      if ((i == 0) || (i == -1))
      {
        y_cut = 25 * cm;
      }
      else if ((i == 1) || (i == -2))
      {
        y_cut = 20 * cm;
      }

      G4Box *virtual_panel = new G4Box("VIRTUAL_PANEL", panel_size / 2, y_edge / 2 - y_cut / 2, panel_thickness / 2 + fiber_diameter * 2);
      G4LogicalVolume *vp_logic = new G4LogicalVolume(virtual_panel, ls_material, "VIRTUAL_PANEL");
      vp_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

      G4Box *ptfe_panel = new G4Box("PTFE_PANEL", panel_size / 2, y_edge / 2 - y_cut / 2, panel_thickness / 2);
      G4LogicalVolume *ptfe_logic = new G4LogicalVolume(ptfe_panel, ptfe_material, "PTFE_PANEL");
      ptfe_logic->SetVisAttributes(nexus::White());

      G4double fiber_length = y_edge / 2 - y_cut / 2 - panel_thickness / 2;
      GenericWLSFiber *fiber_ = new GenericWLSFiber("B2", true, true,
        fiber_diameter, fiber_length * 2, true,
        false, materials::PS(), nullptr,
        true);
      fiber_->SetCoreOpticalProperties(opticalprops::B2());
      fiber_->Construct();
      G4LogicalVolume *fiber_logic = fiber_->GetLogicalVolume();

      fiber_logic->SetVisAttributes(nexus::BlueAlpha());

      // for (int j = -60; j < 60; j++)
      // {
      //   new G4PVPlacement(pRot, G4ThreeVector(j * fiber_diameter + fiber_diameter / 2, 0, panel_thickness / 2 + fiber_diameter / 2),
      //                     fiber_logic, "FIBER", vp_logic, true, 0, true);
      //   new G4PVPlacement(pRot, G4ThreeVector(j * fiber_diameter + fiber_diameter / 2, 0, -panel_thickness / 2 - fiber_diameter / 2),
      //                     fiber_logic, "FIBER", vp_logic, true, 0, true);

      // }
      new G4PVPlacement(0, G4ThreeVector(0, 0, 0), ptfe_logic, "PTFE_PANEL", vp_logic, false, 0, false);
      new G4PVPlacement(pRotz, G4ThreeVector(0, fiber_length + photosensor->GetThickness() / 2, panel_thickness / 2 + fiber_diameter / 2),
                        photosensor_logic, "FIBERSENSOR", vp_logic,
                        true, 1, true);
      new G4PVPlacement(pRotz, G4ThreeVector(0, fiber_length + photosensor->GetThickness() / 2, -panel_thickness / 2 - fiber_diameter / 2),
                        photosensor_logic, "FIBERSENSOR", vp_logic,
                        true, 1, true);
      new G4PVPlacement(0, G4ThreeVector(x_coord, y_edge / 2 + 0.5 * cm + y_cut / 2, 0),
                        vp_logic, "VIRTUAL_PANEL", ls_logic, false, 0, false);
      new G4PVPlacement(0, G4ThreeVector(x_coord, -y_edge / 2 - 0.5 * cm - y_cut / 2, 0),
                        vp_logic, "VIRTUAL_PANEL", ls_logic, false, 0, false);
    }

    // // Black flapper

    G4double flapper_width = panel_size * 4;
    G4double flapper_height = 19.5 * cm;
    G4double flapper_thickness = 2 * cm;
    G4Box *flapper_solid = new G4Box("FLAPPER", flapper_width / 2, flapper_height / 2, flapper_thickness / 2);
    G4Material *flapper_material = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
    G4LogicalVolume *flapper_logic = new G4LogicalVolume(flapper_solid, flapper_material, "FLAPPER");
    flapper_material->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());
    G4Color black(0., 0., 0.);
    flapper_logic->SetVisAttributes(G4VisAttributes(black));
    new G4PVPlacement(0, G4ThreeVector(panel_padding / 2, flapper_height / 2 + 0.5 * cm, 0), flapper_logic, "FLAPPER", ls_logic, false, 0, false);
    new G4PVPlacement(0, G4ThreeVector(panel_padding / 2, -flapper_height / 2 - 0.5 * cm, 0), flapper_logic, "FLAPPER", ls_logic, false, 0, false);

    G4double flapper_tops_height = 5 * cm;
    G4double flapper_tops_width = panel_size * 2;
    G4double flapper_tops_thickness = 2 * cm;
    G4Box *flapper_tops_solid = new G4Box("FLAPPER_TOPS", flapper_tops_width / 2, flapper_tops_height / 2, flapper_tops_thickness / 2);
    G4LogicalVolume *flapper_tops_logic = new G4LogicalVolume(flapper_tops_solid, flapper_material, "FLAPPER_TOPS");

    flapper_tops_logic->SetVisAttributes(G4VisAttributes(black));
    new G4PVPlacement(0, G4ThreeVector(panel_padding / 2, flapper_height + flapper_tops_height / 2 + 0.5 * cm, 0), flapper_tops_logic, "FLAPPER_TOPS", ls_logic, false, 0, false);
    new G4PVPlacement(0, G4ThreeVector(panel_padding / 2, -flapper_height - flapper_tops_height / 2 - 0.5 * cm, 0), flapper_tops_logic, "FLAPPER_TOPS", ls_logic, false, 0, false);


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
    return G4ThreeVector(0., 0., 10 * cm);
  }

} // end namespace nexus
