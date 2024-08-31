#include "COCOA.h"

#include <G4Box.hh>
#include <G4GenericMessenger.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4VisAttributes.hh>
#include "FactoryBase.h"
#include "MaterialsList.h"
#include "Visibilities.h"
#include "OpticalMaterialProperties.h"
#include "IonizationSD.h"

using namespace nexus;

REGISTER_CLASS(COCOA, GeometryBase)

namespace nexus
{

  using namespace CLHEP;

  COCOA::COCOA() : GeometryBase(),
                   detector_height_(40 * cm),
                   detector_width_(30 * cm),
                   detector_thickness_(20 * cm)
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/COCOA/",
                                  "Control commands of geometry COCOA.");

    G4GenericMessenger::Command &height_cmd =
        msg_->DeclareProperty("detector_height", detector_height_,
                              "Detector height");
    height_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command &width_cmd =
        msg_->DeclareProperty("detector_width", detector_width_,
                              "Detector width");
    width_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command &thickness_cmd =
        msg_->DeclareProperty("detector_thickness", detector_thickness_,
                              "Detector thickness");
    thickness_cmd.SetUnitCategory("Length");
  }

  COCOA::~COCOA()
  {
    delete msg_;
  }

  void COCOA::Construct()
  {

    G4double size = 1. * m;
    G4Box *lab_solid =
        new G4Box("LAB", size / 2., size / 2., size / 2.);

    G4Material *air = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

    G4LogicalVolume *lab_logic = new G4LogicalVolume(lab_solid, air, "LAB");

    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic);

    gamma_plane_ = new BoxPointSampler(detector_width_, 0, detector_thickness_, 0, G4ThreeVector(0., detector_height_ + 10 * cm, 0.), nullptr);

    G4Box *scatterer =
        new G4Box("SCATTERER", detector_width_ / 2., detector_height_ / 2., detector_thickness_ / 2.);

    G4Material *detector_mat = materials::GdLS();
    detector_mat->SetMaterialPropertiesTable(opticalprops::LiquidO());

    G4LogicalVolume *scatterer_logic =
        new G4LogicalVolume(scatterer, detector_mat, "SCATTERER");
    scatterer_logic->SetVisAttributes(nexus::LightBlueAlpha());

    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.),
                      scatterer_logic, "SCATTERER", lab_logic, false, 0, true);

    G4double absorber_width = 37.6 * mm;
    G4Box *absorber = new G4Box("ABSORBER", detector_width_ / 2., absorber_width / 2, detector_thickness_ / 2.);
    G4Material *absorber_mat = materials::LaBr3();
    G4LogicalVolume *absorber_logic = new G4LogicalVolume(absorber, absorber_mat, "ABSORBER");
    absorber_logic->SetVisAttributes(nexus::RedAlpha());

    new G4PVPlacement(0, G4ThreeVector(0., -detector_height_ / 2 - absorber_width / 2 - 5 * mm, 0.),
                      absorber_logic, "ABSORBER", lab_logic, false, 0, true);


    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    IonizationSD* ionization_absorber = new IonizationSD("ABSORBER");
    sdman->AddNewDetector(ionization_absorber);
    absorber_logic->SetSensitiveDetector(ionization_absorber);

    IonizationSD* ionization_scatterer = new IonizationSD("SCATTERER");
    sdman->AddNewDetector(ionization_scatterer);
    scatterer_logic->SetSensitiveDetector(ionization_scatterer);

  }

  G4ThreeVector COCOA::GenerateVertex(const G4String &region) const
  {

    return gamma_plane_->GenerateVertex(region);

    // return vertex;
  }

}
