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
#include "GenericWLSFiber.h"
#include "GenericCircularPhotosensor.h"
#include "G4Tubs.hh"
using namespace nexus;

REGISTER_CLASS(COCOA, GeometryBase)

namespace nexus
{

  using namespace CLHEP;

  COCOA::COCOA() : GeometryBase(),
                   detector_height_(40 * cm),
                   detector_width_(30 * cm),
                   detector_thickness_(30 * cm)
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

    gamma_plane_ = new BoxPointSampler(detector_width_, 0, detector_thickness_, 0, G4ThreeVector(0., detector_height_ / 2 + 1 * cm, 0.), nullptr);
    scatterer_volume_ = new BoxPointSampler(detector_width_, detector_height_, detector_thickness_, 0, G4ThreeVector(0., 0., 0.), nullptr);

    G4double inclination = 12 * deg;
    G4double padding = detector_height_ * std::tan(inclination) / 2 + 1.5 * mm;
    G4Box *scatterer =
        new G4Box("SCATTERER", detector_width_ / 2. + padding, detector_height_ / 2., detector_thickness_ / 2. + 1.5 * mm);
    scatterer_volume_ = new BoxPointSampler(detector_width_ + padding * 2, detector_height_, detector_thickness_, 0, G4ThreeVector(0., 0., 0.), nullptr);

    G4Material *detector_mat = materials::LAB();
    detector_mat->SetMaterialPropertiesTable(opticalprops::LiquidO());

    G4LogicalVolume *scatterer_logic =
        new G4LogicalVolume(scatterer, detector_mat, "SCATTERER");
    scatterer_logic->SetVisAttributes(nexus::LightBlueAlpha());

    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.),
                      scatterer_logic, "SCATTERER", lab_logic, false, 99999, true);

    G4double absorber_thickness = 37.6 * mm;
    G4double absorber_width = 4.8 * cm;
    G4double wrapping_thickness = 0.2 * cm;
    G4Box *absorber = new G4Box("ABSORBER", absorber_width / 2, absorber_thickness / 2., absorber_width / 2);
    G4Material *absorber_mat = materials::LaBr3();
    G4LogicalVolume *absorber_logic = new G4LogicalVolume(absorber, absorber_mat, "ABSORBER");
    absorber_logic->SetVisAttributes(nexus::YellowAlpha());


    // G4Box *veto = new G4Box("VETO", detector_width_ / 2  + padding , detector_height_ / 2, 6 * mm);
    // G4Material *veto_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_PTFE");;
    // G4LogicalVolume *veto_logic = new G4LogicalVolume(veto, veto_mat, "VETO");
    // veto_logic->SetVisAttributes(nexus::WhiteAlpha());
    // new G4PVPlacement(0, G4ThreeVector(0., 0., detector_thickness_ / 2 + 5 * cm),
    //                   veto_logic, "VETO", lab_logic, false, 999999, true);

    for (uint ix = 0; ix < detector_width_ / (absorber_width + wrapping_thickness); ix++)
    {
      // G4cout << ix << "IX" << " " << absorber_width / detector_width_ << G4endl;
      for (uint iy = 0; iy < detector_width_ / (absorber_width + wrapping_thickness); iy++)
      {
        new G4PVPlacement(0, G4ThreeVector(ix * (absorber_width + wrapping_thickness) - detector_width_ / 2 + absorber_width / 2, -detector_height_ / 2 - absorber_width / 2 - 5 * mm, iy * (absorber_width + wrapping_thickness) - detector_width_ / 2 + absorber_width / 2),
                          absorber_logic, "ABSORBER", lab_logic, false, ix * detector_width_ / absorber_width + iy, true);
      }
    }

    // new G4PVPlacement(0, G4ThreeVector(0., -detector_height_ / 2 - absorber_width / 2 - 5 * mm, 0.),
    //                   absorber_logic, "ABSORBER", lab_logic, false, 999999, true);

    // Add a lattice of GenericWLSFibers uniformly distributed in the scatterer
    G4double fiber_length = detector_height_ - 5 * mm;
    GenericWLSFiber *fiber_ = new GenericWLSFiber("B2", true, 1 * mm, fiber_length, true, false, nullptr, materials::B2(), true, true);
    fiber_->SetCoreOpticalProperties(opticalprops::B2());
    fiber_->Construct();
    G4LogicalVolume *fiber_logic = fiber_->GetLogicalVolume();
    fiber_logic->SetVisAttributes(nexus::LightGreen());
    G4double pitch = 1 * cm;

    G4RotationMatrix *rot = new G4RotationMatrix();
    rot->rotateX(90 * deg);

    GenericCircularPhotosensor *photosensor = new GenericCircularPhotosensor("SIPM_FIBER", 1 * mm, 1 * mm);
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
    photosensor->SetTimeBinning(1 * us);
    photosensor->SetVisibility(true);
    photosensor->SetWindowRefractiveIndex(opticalprops::BorosilicateGlass()->GetProperty("RINDEX"));
    photosensor->SetOpticalProperties(photosensor_mpt);
    photosensor->SetSensorDepth(1);
    photosensor->Construct();
    G4LogicalVolume *photosensor_logic = photosensor->GetLogicalVolume();
    photosensor_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
    G4Tubs *fiber_sensor = new G4Tubs("FIBER_SENSOR", 0, 1 * mm / 2, fiber_length / 2 + 1 * mm / 2, 0, 2 * M_PI);

    // fiber_sensor_logic->SetVisAttributes(nexus::WhiteAlpha());

    for (uint ix = 0; ix < detector_width_ / pitch; ix++)
    {
      for (uint iy = 0; iy < detector_thickness_ / pitch; iy++)
      {
        if (ix % 2 == 0)
          rot->rotateZ(inclination);
        else
          rot->rotateZ(-inclination);

        G4LogicalVolume *fiber_sensor_logic = new G4LogicalVolume(fiber_sensor, G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic"), "FIBER_SENSOR");
        fiber_sensor_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
        if (ix % 2 == 0)
        {
          std::string label = std::to_string(ix * detector_width_ / pitch + iy + 10000);
          new G4PVPlacement(0, G4ThreeVector(0, 0, -fiber_length / 2), photosensor_logic, "SENSOR", fiber_sensor_logic, true, ix * detector_width_ / pitch + iy + 10000, true);
          new G4PVPlacement(0, G4ThreeVector(0, 0, 1 * mm / 2), fiber_logic, "FIBER", fiber_sensor_logic, true, ix * detector_width_ / pitch + iy + 10000, true);
          new G4PVPlacement(G4Transform3D(*rot, G4ThreeVector(ix * pitch - detector_width_ / 2, 0, iy * pitch - detector_thickness_ / 2)),
                            fiber_sensor_logic, "FIBERSENSOR" + label, scatterer_logic,
                            true, ix * detector_width_ / pitch + iy + 10000, true);
        }
        else
        {
          std::string label = std::to_string(ix * detector_width_ / pitch + iy);
          new G4PVPlacement(0, G4ThreeVector(0, 0, -fiber_length / 2), photosensor_logic, "SENSOR", fiber_sensor_logic, true, ix * detector_width_ / pitch + iy, true);
          new G4PVPlacement(0, G4ThreeVector(0, 0, 1 * mm / 2), fiber_logic, "FIBER", fiber_sensor_logic, true, ix * detector_width_ / pitch + iy, true);
          new G4PVPlacement(G4Transform3D(*rot, G4ThreeVector(ix * pitch - detector_width_ / 2, 0, iy * pitch - detector_thickness_ / 2 + 1 * mm)),
                            fiber_sensor_logic, "FIBER" + label, scatterer_logic,
                            true, ix * detector_width_ / pitch + iy, true);
        }
        if (ix % 2 == 0)
          rot->rotateZ(-inclination);
        else
          rot->rotateZ(inclination);
      }
    }

    // G4SDManager *sdman = G4SDManager::GetSDMpointer();
    // IonizationSD *ionization_absorber = new IonizationSD("ABSORBER");
    // sdman->AddNewDetector(ionization_absorber);
    // absorber_logic->SetSensitiveDetector(ionization_absorber);

    // IonizationSD *ionization_scatterer = new IonizationSD("SCATTERER");
    // sdman->AddNewDetector(ionization_scatterer);
    // scatterer_logic->SetSensitiveDetector(ionization_scatterer);
  }

  G4ThreeVector COCOA::GenerateVertex(const G4String &region) const
  {
    return scatterer_volume_->GenerateVertex(region);
    return gamma_plane_->GenerateVertex(region);

    G4ThreeVector vertex(0, 50 * cm, 0);
    return vertex;
  }

}
