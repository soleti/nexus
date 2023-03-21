#include "ESSBeam.h"

#include <G4Box.hh>
#include <G4Tubs.hh>

#include <G4GenericMessenger.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <Randomize.hh>
#include <G4VisAttributes.hh>

#include "FactoryBase.h"
#include "MaterialsList.h"
#include "Visibilities.h"
#include "OpticalMaterialProperties.h"
#include "GenericPhotosensor.h"

using namespace nexus;

REGISTER_CLASS(ESSBeam, GeometryBase)

namespace nexus {

  using namespace CLHEP;

  ESSBeam::ESSBeam():
    GeometryBase(),
    detector_height_(4 * m),
    detector_diam_ (4 * m)
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/ESSBeam/",
                                  "Control commands of geometry ESSBeam.");

    G4GenericMessenger::Command&  height_cmd =
      msg_->DeclareProperty("detector_height", detector_height_,
                            "Detector height");
    height_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  diam_cmd =
      msg_->DeclareProperty("detector_diam", detector_diam_,
                            "Detector diameter");
    diam_cmd.SetUnitCategory("Length");
  }

  ESSBeam::~ESSBeam()
  {
    delete msg_;
  }

 void ESSBeam::Construct()
  {
    // G4ThreeVector *detector_center = new G4ThreeVector(13.* m, 26. * m, 0);
    G4ThreeVector *detector_center = new G4ThreeVector(0, 0, 0);

    inside_cylinder_ = new CylinderPointSampler2020(0, detector_diam_ / 2., detector_height_ / 2., 0, twopi, nullptr, *detector_center);

    G4double size = 80. * m;
    G4Box* lab_solid =
      new G4Box("LAB", size/2., size/2., size/2.);

    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

    G4LogicalVolume* lab_logic = new G4LogicalVolume(lab_solid, air, "LAB");

    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic);

    // // First, build the moderator: a big steel cylinder.
    // G4double mod_diam   = 6. * m;
    // G4double mod_height = 8. * m;
    // G4Tubs* moderator_solid =
    //   new G4Tubs("MODERATOR",  0., mod_diam/2., mod_height/2., 0, twopi);

    // G4Material* steel = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    // G4LogicalVolume* moderator_logic =
    //   new G4LogicalVolume(moderator_solid, steel, "MODERATOR");
    // new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), moderator_logic,
    //                   "MODERATOR", lab_logic, false, 0, true);
    // moderator_logic->SetVisAttributes(nexus::DarkGreyAlpha());

    // // Then, put air inside it
    // G4double mod_thickess = 5. * cm;
    // G4Tubs* inner_air_solid =
    //   new G4Tubs("INNER_AIR",  0., (mod_diam - 2*mod_thickess)/2., (mod_height - 2*mod_thickess)/2., 0, twopi);
    // G4LogicalVolume* inner_air_logic =
    //   new G4LogicalVolume(inner_air_solid, air, "INNER_AIR");
    // new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), inner_air_logic,
    //                   "INNER_AIR", moderator_logic, false, 0, true);

    // // Finally, the wheel is placed inside the air.
    // G4double wheel_diam = 2.5 * m;
    // G4double wheel_height = 10 * cm; // to check

    // G4Tubs* wheel_solid =
    //   new G4Tubs("WHEEL",  0., wheel_diam/2., wheel_height/2., 0, twopi);

    // G4Material* wheel_mat;
    // wheel_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_W");
    // G4cout << "Wheel made of tungsten" << G4endl;

    // G4LogicalVolume* wheel_logic =
    //   new G4LogicalVolume(wheel_solid, wheel_mat, "WHEEL");
    // new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), wheel_logic,
    //                   "WHEEL", inner_air_logic, false, 0, true);
    // wheel_logic->SetVisAttributes(nexus::DarkGreyAlpha());

    // Detector
    G4Tubs* detector_solid =
      new G4Tubs("DETECTOR",  0., detector_diam_/2., detector_height_/2., 0, twopi);

    G4Material *detector_mat = materials::GdLS();
    detector_mat->SetMaterialPropertiesTable(opticalprops::GdLS());

    G4LogicalVolume* detector_logic =
      new G4LogicalVolume(detector_solid, detector_mat, "DETECTOR");

    detector_logic->SetVisAttributes(nexus::LightBlueAlpha());
    new G4PVPlacement(0, *detector_center,
                      detector_logic, "DETECTOR", lab_logic, false, 0, true);

    GenericPhotosensor *pmt_  = new GenericPhotosensor("PMT", 46 * cm, 46 * cm, 5 * cm);
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();

    const G4int entries = 30;

    G4double eff_energies[entries] =
      {1.72194*eV, 1.77114*eV, 1.82324*eV, 1.87848*eV, 1.93719*eV,
       1.99968*eV,  2.06633*eV, 2.13759*eV, 2.21393*eV, 2.29593*eV,
       2.38423*eV, 2.47960*eV, 2.58292*eV, 2.69522*eV, 2.81773*eV,
       2.95190*eV, 3.0995*eV, 3.26263*eV, 3.44389*eV, 3.64647*eV,
       3.87438*eV, 4.13267*eV, 4.42786*eV, 4.76846*eV, 5.16583*eV,
       5.63545*eV, 6.19900*eV, 6.88778*eV, 7.74875*eV, 8.85571*eV};
    G4double eff[entries] =
      { 0.00000, 0.00028, 0.00100, 0.00500, 0.00100,
    	0.02200, 0.04500, 0.07000, 0.11500, 0.16000,
    	0.20500, 0.23500, 0.27000, 0.29000, 0.31300,
    	0.35200, 0.38000, 0.38000, 0.37300, 0.37300,
    	0.37000, 0.36000, 0.35500, 0.33500, 0.31000,
    	0.29500, 0.27500, 0.23000, 0.52000, 0.00000};

    G4double energy[]       = {opticalprops::optPhotMinE_, opticalprops::optPhotMaxE_};
    G4double reflectivity[] = {0.0     , 0.0     };

    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 2);
    photosensor_mpt->AddProperty("EFFICIENCY", eff_energies, eff, entries);
    pmt_->SetVisibility(true);
    pmt_->SetWindowRefractiveIndex(opticalprops::BorosilicateGlass()->GetProperty("RINDEX"));
    pmt_->SetSensorDepth(1);
    pmt_->SetOpticalProperties(photosensor_mpt);
    pmt_->Construct();

    G4LogicalVolume* pmt_logic = pmt_->GetLogicalVolume();

    G4int angles = 10;
    G4double step = 2. * pi / angles;


    G4RotationMatrix *rot_z = new G4RotationMatrix();
      rot_z->rotateY(180 * deg);

    for (G4int itheta=0; itheta < angles; itheta++) {
      G4float theta = twopi / angles * itheta;
      std::string label = std::to_string(theta);
      G4double x = (detector_diam_ / 2. - 50 * cm) * std::sin(theta) * mm;
      G4double y = (detector_diam_ / 2. - 50 * cm) * std::cos(theta) * mm;
      new G4PVPlacement(0, G4ThreeVector(x, y, -detector_height_ / 2 + 5 * cm),
                        pmt_logic, "PMTDownout" + label, detector_logic,
                        false, 100+itheta, false);
      new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(x, y, detector_height_ / 2 - 5 * cm)),
                        pmt_logic, "PMTUpout" + label, detector_logic,
                        false, 400+itheta, false);
    }

    angles = 5;
    for (G4int itheta=0; itheta < angles; itheta++) {
      G4float theta = twopi / angles * itheta;
      std::string label = std::to_string(theta);
      G4double x = (detector_diam_ / 2. - 120 * cm) * std::sin(theta) * mm;
      G4double y = (detector_diam_ / 2. - 120 * cm) * std::cos(theta) * mm;
      new G4PVPlacement(0, G4ThreeVector(x, y, -detector_height_ / 2 + 5 * cm),
                        pmt_logic, "PMTDownmid" + label, detector_logic,
                        false, 200+itheta, false);
      new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(x, y, detector_height_ / 2 - 5 * cm)),
                        pmt_logic, "PMTUpmid" + label, detector_logic,
                        false, 500+itheta, false);
    }

    new G4PVPlacement(0, G4ThreeVector(0, 0, -detector_height_ / 2 + 5 * cm),
                      pmt_logic, "PMTDown0", detector_logic,
                      false, 300, false);
    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, 0, detector_height_ / 2 - 5 * cm)),
                      pmt_logic, "PMTUp0", detector_logic,
                      false, 600, false);

    angles = 10;
    G4int rows = 5;
    for (G4int irow = 0; irow < rows; irow++) {
      G4RotationMatrix *rot = new G4RotationMatrix();
      rot->rotateX(90 * deg);
      for (G4int itheta=0; itheta < angles; itheta++) {
        G4float theta = 2 * M_PI / angles * itheta;
        std::string label = std::to_string(theta);

        if (irow % 2 == 0) {
          theta += step / 2;
          if (itheta == 0)
            rot->rotateZ(-step/2);
        }

        G4double y = (detector_diam_ / 2. - 10 * cm / 2 - 1 * cm) * std::cos(theta) * mm;
        G4double x = (detector_diam_ / 2. - 10 * cm / 2 - 1 * cm) * std::sin(theta) * mm;

        new G4PVPlacement(G4Transform3D(*rot, G4ThreeVector(x, y, -detector_height_ / 2 + detector_height_/rows * irow + detector_height_/rows / 2)),
                          pmt_logic, "PMT" + label, detector_logic,
                          false, irow*angles + itheta, true);
        rot->rotateZ(-step);
      }
    }
  }

  G4ThreeVector ESSBeam::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0., 0., 0.);
    G4double y_starting_point = 30 * m;

    if (region == "SINGLE_POINT") {

      vertex = G4ThreeVector(0., y_starting_point, 0.);

    } else if (region == "BEAM") {

      G4double xz_rms = 2.2 * mm;
      G4double t_rms = 3 * picosecond;
      G4double proton_speed = 2.9*1.e8 * m / s;
      G4double y_rms = t_rms * proton_speed;

      for (G4int i=0; i<3; i++) {
        if (i != 1)  { // Coordinates transverse to beam direction
          vertex[i] = G4RandGauss::shoot(0, xz_rms);
        } else {
          vertex[i] = G4RandGauss::shoot(y_starting_point, y_rms);
        }
      }
    } else if (region == "DETECTOR") {
      return inside_cylinder_->GenerateVertex("VOLUME");
    } else {
      G4Exception("[ESSBeam]", "GenerateVertex()", FatalException,
                  "Unknown vertex generation region!");
    }

    return vertex;
  }
  
}
