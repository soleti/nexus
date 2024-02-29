// ----------------------------------------------------------------------------
// nexus | PETCsI.cc
//
// This class describes a pair of PETCsI crystals with a radioactive source in the middle
//
// ----------------------------------------------------------------------------

#include "PETCsI.h"
#include "MaterialsList.h"
#include "Visibilities.h"
#include "OpticalMaterialProperties.h"
#include "IonizationSD.h"

#include <G4Box.hh>
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4GenericMessenger.hh>
#include <G4Box.hh>
#include <G4SDManager.hh>

#include "FactoryBase.h"

#include <CLHEP/Units/SystemOfUnits.h>

using namespace nexus;

REGISTER_CLASS(PETCsI, GeometryBase)

namespace nexus
{

  using namespace CLHEP;

  PETCsI::PETCsI() : GeometryBase(),
                                   crystal_material_("CsI"),
                                   crystal_width_(48 * mm),
                                   crystal_length_(2.),
                                   pet_diameter_(60 * cm),
                                   pet_length_(76.8 * cm)
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/PETCsI/",
                                  "Control commands of geometry PETCsI.");
    msg_->DeclareProperty("crystal_material", crystal_material_, "Crystal material (CsI, CsITl, LYSO, BGO)");
    G4GenericMessenger::Command&  width_cmd = msg_->DeclareProperty("crystal_width", crystal_width_, "Crystal width");
    width_cmd.SetUnitCategory("Length");
    msg_->DeclareProperty("crystal_length", crystal_length_, "Crystal length (in radiation lengths)");
    G4GenericMessenger::Command& diameter_cmd = msg_->DeclareProperty("pet_diameter", pet_diameter_, "PET inner diameter");
    diameter_cmd.SetUnitCategory("Length");
    G4GenericMessenger::Command& pet_length_cmd = msg_->DeclareProperty("pet_length", pet_length_, "PET length");
    pet_length_cmd.SetUnitCategory("Length");

  }

  PETCsI::~PETCsI()
  {
    delete msg_;
  }

  void PETCsI::Construct()
  {
    sphere1_ = new SpherePointSampler(10 * mm, 0, G4ThreeVector(-5 * cm ,  0 * cm, 0));
    sphere2_ = new SpherePointSampler(20 * mm, 0, G4ThreeVector(-10 * cm , 0 * cm, 0));
    G4double size = 2 * m;
    G4Box *lab_solid =
        new G4Box("LAB", size / 2., size / 2., size / 2.);

    G4Material *air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    G4LogicalVolume *lab_logic = new G4LogicalVolume(lab_solid, air, "LAB");

    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
    new G4PVPlacement(
            0, G4ThreeVector(), lab_logic, "lab", 0, false, 0, true);

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic);

    G4Material *material = nullptr;

    if (crystal_material_ == "CsI") {
      crystal_length_ *= 18.6 * mm;
      material = G4NistManager::Instance()->FindOrBuildMaterial("G4_CESIUM_IODIDE");
      material->SetMaterialPropertiesTable(opticalprops::CsI());
    } else if (crystal_material_ == "BGO") {
      crystal_length_ *= 11.4 * mm;
      material = G4NistManager::Instance()->FindOrBuildMaterial("G4_BGO");
      material->SetMaterialPropertiesTable(opticalprops::BGO());
    } else if (crystal_material_ == "LYSO") {
      crystal_length_ *= 11.4 * mm;
      material = materials::LYSO();
      material->SetMaterialPropertiesTable(opticalprops::LYSO());
    } else if (crystal_material_ == "CsITl") {
      crystal_length_ *= 18.6 * mm;
      material = G4NistManager::Instance()->FindOrBuildMaterial("G4_CESIUM_IODIDE");
      material->SetMaterialPropertiesTable(opticalprops::CsITl());
    } else {
      G4Exception("[PETCsI]", "Construct()", FatalException,
                  "Unknown crystal material!");
    }

    G4Box *crystal =
        new G4Box("CRYSTAL", crystal_width_ / 2., crystal_width_ / 2., crystal_length_ / 2.);

    G4LogicalVolume *crystal_logic =
        new G4LogicalVolume(crystal,
                            material,
                            "CRYSTAL");
    crystal_logic->SetVisAttributes(nexus::LightBlueAlpha());

    G4int angles = floor(pet_diameter_ * M_PI / crystal_width_);
    G4double step = 2. * pi / angles;
    G4RotationMatrix *rot = new G4RotationMatrix();
    rot->rotateX(90 * deg);

    G4int rings = floor(pet_length_ / crystal_width_);

    for (G4int iring=0; iring < rings; iring++) {
      for (G4int itheta=0; itheta < angles; itheta++) {
        G4float theta = 2 * M_PI / angles * itheta;
        std::string label = std::to_string(iring*rings + itheta);

            G4double y = (pet_diameter_ / 2. + crystal_length_ / 2) * std::cos(theta);
            G4double x = (pet_diameter_ / 2. + crystal_length_ / 2) * std::sin(theta);
            G4double z = -pet_length_ / 2 + iring * crystal_width_ + crystal_length_ / 2;
            new G4PVPlacement(G4Transform3D(*rot, G4ThreeVector(x, y, z)),
                              crystal_logic, "CRYSTAL" + label, lab_logic,
                              false, iring*rings + itheta, true);
            G4cout << "CRYSTAL" << label << " " << x << " " << y << " " << z << G4endl;
        rot->rotateZ(-step);
      }
    }

    G4SDManager* sdmgr = G4SDManager::GetSDMpointer();
    IonizationSD* ionisd = new IonizationSD("PET");
    // ionisd->IncludeInTotalEnergyDeposit(false);
    sdmgr->AddNewDetector(ionisd);
    crystal_logic->SetSensitiveDetector(ionisd);

    jas_phantom_ = new JaszczakPhantom();
    jas_phantom_->Construct();
    G4LogicalVolume* phantom_logic = jas_phantom_->GetLogicalVolume();
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), phantom_logic, "JASZCZAK",
                      lab_logic, false, 0, true);
    // G4VPhysicalVolume *crystal_position = new G4PVPlacement(0, G4ThreeVector(0, 0, 25. / 2 * mm + crystal_length_ / 2),
    //                                                         crystal_logic, "CRYSTAL", lab_logic,
    //                                                         true, 2, true);

//     G4double teflon_thickness = 0.08 * mm;
//     G4int teflon_coatings = 5;
//     G4double teflon_thickness_tot = teflon_thickness * teflon_coatings;
//     G4Box *teflon_coating_full = new G4Box("TEFLON_FULL", crystal_width_ / 2 + teflon_thickness_tot / 2, crystal_width_ / 2 + teflon_thickness_tot / 2, crystal_length_ / 2.);

//     G4SubtractionSolid *teflon_coating = new G4SubtractionSolid("TEFLON", teflon_coating_full, crystal);
//     G4LogicalVolume *teflon_logic =
//         new G4LogicalVolume(teflon_coating,
//                             G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON"),
//                             "TEFLON_SIDES");

//     teflon_logic->SetVisAttributes(nexus::White());

//     G4VPhysicalVolume *teflon_full_position = new G4PVPlacement(0, G4ThreeVector(0, 0, 25. / 2 * mm + crystal_length_ / 2),
//                                                                 teflon_logic, "TEFLON_SIDES", lab_logic,
//                                                                 true, 1, true);

//     G4OpticalSurface *ptfe_surface = new G4OpticalSurface("PTFE_SURFACE");
//     ptfe_surface->SetType(dielectric_LUT);
//     ptfe_surface->SetFinish(groundteflonair);
//     // ptfe_surface->SetFinish(RoughTeflon_LUT);
//     ptfe_surface->SetModel(LUT);
//     // ptfe_surface->SetType(dielectric_dielectric);
//     // ptfe_surface->SetFinish(polishedvm2000air);
//     // ptfe_surface->SetFinish(polishedfrontpainted);
//     // ptfe_surface->SetModel(unified);
//     // ptfe_surface->SetMaterialPropertiesTable(opticalprops::PTFE());
// //
//     // G4OpticalSurface *air_surface = new G4OpticalSurface("CRYSTAL_SURFACE");
//     // ptfe_surface->SetType(dielectric_LUTDAVIS);
//     // ptfe_surface->SetFinish(Polished_LUT);
//     // ptfe_surface->SetModel(DAVIS);

//     new G4LogicalBorderSurface(
//         "CRYSTAL_PTFE", crystal_right, teflon_full_position, ptfe_surface);

//     if (back_wrapping_) {
//       G4Box *teflon_back = new G4Box("TEFLON_BACK", crystal_width_ / 2 + teflon_thickness_tot / 2, crystal_width_ / 2 + teflon_thickness_tot / 2, teflon_thickness_tot / 2);
//       G4LogicalVolume *teflon_back_logic =
//           new G4LogicalVolume(teflon_back,
//                               G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON"),
//                               "TEFLON_BACK");
//       teflon_back_logic->SetVisAttributes(nexus::White());
//       G4VPhysicalVolume *teflon_back_position = new G4PVPlacement(0, G4ThreeVector(0, 0, 25. / 2 * mm - teflon_thickness_tot / 2),
//                                                                   teflon_back_logic, "TEFLON_BACK", lab_logic,
//                                                                   true, 2, true);
//       new G4LogicalBorderSurface(
//           "CRYSTAL_PTFE_BACK", crystal_right, teflon_back_position, ptfe_surface);
//     }

    // SiPM66NoCasing *sipm_geom = new SiPM66NoCasing();

    // sipm_geom->Construct();
    // G4LogicalVolume *sipm_logic = sipm_geom->GetLogicalVolume();
    // G4int n_rows = (int)crystal_width_ / sipm_geom->GetDimensions().x();
    // G4int n_cols = (int)crystal_width_ / sipm_geom->GetDimensions().y();
    // for (G4int irow = 0; irow < n_rows; irow++)
    // {
    //   for (G4int icol = 0; icol < n_cols; icol++)
    //   {
    //     std::string label = std::to_string(irow * n_rows + icol);
    //     new G4PVPlacement(0, G4ThreeVector(irow * sipm_geom->GetDimensions().x() - crystal_width_ / 2 + sipm_geom->GetDimensions().x() / 2, icol * sipm_geom->GetDimensions().x() - crystal_width_ / 2 + sipm_geom->GetDimensions().x() / 2, 25. / 2 * mm + crystal_length_ + sipm_geom->GetDimensions().z() / 2), sipm_logic,
    //                       "SiPM" + label, lab_logic, true, irow * n_rows + icol);
    //   }
    // }
  }

  G4ThreeVector PETCsI::GenerateVertex(const G4String &region) const
  {
    // G4ThreeVector vertex(0, 0, 0);
    // return vertex;
    // return box_source_->GenerateVertex("INSIDE");
    // vertex =
    // sphere1_->GenerateVertex("INSIDE");
    return jas_phantom_->GenerateVertex("JPHANTOM");
  }

}
