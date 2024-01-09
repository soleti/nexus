#include "ESSBeam.h"

#include <G4Box.hh>
#include <G4Tubs.hh>
#include "PmtR12860.h"
#include <G4GenericMessenger.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4SubtractionSolid.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4PVPlacement.hh>
#include <Randomize.hh>
#include <G4VisAttributes.hh>
#include "GenericWLSFiber.h"
#include "FactoryBase.h"
#include "MaterialsList.h"
#include "Visibilities.h"
#include "OpticalMaterialProperties.h"
#include "GenericPhotosensor.h"
#include "GenericSquarePhotosensor.h"
#include "SiPM11.h"

using namespace nexus;

REGISTER_CLASS(ESSBeam, GeometryBase)

namespace nexus {

  using namespace CLHEP;

  ESSBeam::ESSBeam():
    GeometryBase(),
    detector_height_(10 * cm),
    detector_diam_ (10 * cm),
    beam_(true),
    liquido_(false)
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

    msg_->DeclareProperty("beam", beam_,
                          "Simulate beam");
    msg_->DeclareProperty("liquido", liquido_,
                          "Use a LiquidO-based detector");
  }

  ESSBeam::~ESSBeam()
  {
    delete msg_;
  }

 void ESSBeam::Construct()
  {
    G4ThreeVector *detector_center;
    if (beam_) {
      detector_center = new G4ThreeVector(13.* m, 26. * m, 0);
    } else {
      detector_center = new G4ThreeVector(0, 0, 0);
    }

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

    G4Tubs* roof =
      new G4Tubs("ROOF", 10*m, 10*m + 20*cm, 10 * m, 0, twopi);
    G4Material* concrete = materials::Concrete();
    G4LogicalVolume* roof_logic =
      new G4LogicalVolume(roof, concrete, "CONCRETE");
    new G4PVPlacement(0, *detector_center+G4ThreeVector(0,0,10*m - detector_height_/2 - 0.2 *m ), roof_logic,
                      "ROOF", lab_logic, false, 0, true);
    roof_logic->SetVisAttributes(nexus::LightGreyAlpha());
    roof_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    G4Material* steel = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    if (beam_) {
      // First, build the moderator: a big steel cylinder.
      G4double mod_diam   = 6. * m;
      G4double mod_height = 8. * m;
      G4Tubs* moderator_solid =
        new G4Tubs("MODERATOR",  0., mod_diam/2., mod_height/2., 0, twopi);

      G4LogicalVolume* moderator_logic =
        new G4LogicalVolume(moderator_solid, steel, "MODERATOR");
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), moderator_logic,
                        "MODERATOR", lab_logic, false, 0, true);
      moderator_logic->SetVisAttributes(nexus::DarkGreyAlpha());

      // Then, put air inside it
      G4double mod_thickess = 5. * cm;
      G4Tubs* inner_air_solid =
        new G4Tubs("INNER_AIR",  0., (mod_diam - 2*mod_thickess)/2., (mod_height - 2*mod_thickess)/2., 0, twopi);
      G4LogicalVolume* inner_air_logic =
        new G4LogicalVolume(inner_air_solid, air, "INNER_AIR");
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), inner_air_logic,
                        "INNER_AIR", moderator_logic, false, 0, true);

      // Finally, the wheel is placed inside the air.
      G4double wheel_diam = 2.5 * m;
      G4double wheel_height = 10 * cm; // to check

      G4Tubs* wheel_solid =
        new G4Tubs("WHEEL",  0., wheel_diam/2., wheel_height/2., 0, twopi);

      G4Material* wheel_mat;
      wheel_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_W");
      G4cout << "Wheel made of tungsten" << G4endl;

      G4LogicalVolume* wheel_logic =
        new G4LogicalVolume(wheel_solid, wheel_mat, "WHEEL");
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), wheel_logic,
                        "WHEEL", inner_air_logic, false, 0, true);
      wheel_logic->SetVisAttributes(nexus::DarkGreyAlpha());
    }
    // Detector

    G4Tubs* detector_solid =
      new G4Tubs("DETECTOR",  0., detector_diam_/2., detector_height_/2., 0, twopi);

    G4Material *detector_mat = materials::GdLS();
    G4bool liquido = false;
    if (liquido) {
      detector_mat->SetMaterialPropertiesTable(opticalprops::LiquidO());
    } else {
      detector_mat->SetMaterialPropertiesTable(opticalprops::GdLSAcenaphthene());
    }

    G4LogicalVolume* detector_logic =
      new G4LogicalVolume(detector_solid, detector_mat, "DETECTOR");

    detector_logic->SetVisAttributes(nexus::LightBlueAlpha());
    new G4PVPlacement(0, *detector_center,
                      detector_logic, "DETECTOR", lab_logic, false, 0, true);

    // Steel container
    G4double container_diam   = detector_diam_+10*cm;
    G4double container_height = detector_height_+10*cm;
    G4Tubs* container_full =
      new G4Tubs("CONTAINER_FULL", 0, container_diam/2.+5*cm, container_height/2., 0, twopi);
    G4SubtractionSolid *container = new G4SubtractionSolid("CONTAINER",
							   container_full,
							   detector_solid);

    // G4Material* steel = G4NistManager::Instance()->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    G4LogicalVolume* container_logic =
      new G4LogicalVolume(container, steel, "CONTAINER");
    // container_logic->SetVisAttributes(nexus::DarkGreyAlpha());
    container_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
    new G4PVPlacement(0, *detector_center, container_logic,
                      "CONTAINER", lab_logic, false, 0, true);

    GenericPhotosensor *pmt_  = new GenericPhotosensor("PMT", 46 * cm, 46 * cm, 5 * cm);
    // PmtR12860 *pmt_ = new PmtR12860();
    GenericSquarePhotosensor *lappd_  = new GenericSquarePhotosensor("LAPPD", 20 * cm, 20 * cm, 5 * cm);
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    G4MaterialPropertiesTable* lappd_mpt = new G4MaterialPropertiesTable();

    // Paraffin aborber
    G4double absorber_diam   = container_diam+20*cm;
    G4double absorber_height = container_height+20*cm;
    G4Tubs* absorber_full =
      new G4Tubs("ABSORBER_FULL", 0, absorber_diam/2.+20*cm, absorber_height/2., 0, twopi);
    G4SubtractionSolid *absorber_container = new G4SubtractionSolid("ABSORBER_CONTAINER",
							   absorber_full,
							   container);
    G4SubtractionSolid *absorber = new G4SubtractionSolid("ABSORBER",
							   absorber_container,
							   detector_solid);

    G4Material* paraffin = G4NistManager::Instance()->FindOrBuildMaterial("G4_PARAFFIN");
    // G4LogicalVolume* absorber_logic =
    //   new G4LogicalVolume(absorber, paraffin, "ABSORBER");
    // absorber_logic->SetVisAttributes(nexus::RedAlpha());
    // absorber_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    G4OpticalSurface* opsur_ptfe =
      new G4OpticalSurface("PTFE_OPSURF", unified, polished, dielectric_metal);
    opsur_ptfe->SetMaterialPropertiesTable(opticalprops::PTFE());

    // new G4LogicalSkinSurface("PTFE_OPSURF", absorber_logic, opsur_ptfe);
    // new G4LogicalSkinSurface("PTFE_OPSURF", container_logic, opsur_ptfe);

    // new G4PVPlacement(0, *detector_center, absorber_logic,
    //                   "ABSORBER", lab_logic, false, 0, true);

    if (!liquido) {
      const G4int entries = 11;

      G4double eff_energies[entries] =
        {
          h_Planck * c_light / (638.96 * nm), h_Planck * c_light / (618.18 * nm),
          h_Planck * c_light / (584.42 * nm), h_Planck * c_light / (545.45 * nm),
          h_Planck * c_light / (514.29 * nm), h_Planck * c_light / (464.94 * nm),
          h_Planck * c_light / (405.19 * nm), h_Planck * c_light / (342.86 * nm),
          h_Planck * c_light / (307.79 * nm), h_Planck * c_light / (294.81 * nm),
          h_Planck * c_light / (284.42 * nm)
        };
      G4double eff[entries] =
        {
          0.0007, 0.0030,
          0.0185, 0.0562,
          0.1259, 0.2154,
          0.2929, 0.2765,
          0.0656, 0.0075,
          0.0006
        };
        G4double eff_lappd[entries] =
        {
          0.05, 0.05,
          0.15, 0.15,
          0.20, 0.20,
          0.40, 0.40,
          0.42, 0.42,
          0.42
        };

      G4double energy[]       = {opticalprops::optPhotMinE_, opticalprops::optPhotMaxE_};
      G4double reflectivity[] = {0.0     , 0.0     };

      photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 2);
      photosensor_mpt->AddProperty("EFFICIENCY", eff_energies, eff, entries);

      lappd_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 2);
      lappd_mpt->AddProperty("EFFICIENCY", eff_energies, eff_lappd, entries);
      pmt_->SetTimeBinning(1 * us);
      pmt_->SetVisibility(true);
      pmt_->SetWindowRefractiveIndex(opticalprops::BorosilicateGlass()->GetProperty("RINDEX"));
      pmt_->SetSensorDepth(1);
      pmt_->SetOpticalProperties(photosensor_mpt);
      pmt_->Construct();

      lappd_->SetTimeBinning(0.01 * ns);
      lappd_->SetVisibility(true);
      lappd_->SetWindowRefractiveIndex(opticalprops::BorosilicateGlass()->GetProperty("RINDEX"));
      lappd_->SetSensorDepth(1);
      lappd_->SetOpticalProperties(lappd_mpt);
      lappd_->Construct();

      G4LogicalVolume* pmt_logic = pmt_->GetLogicalVolume();
      G4LogicalVolume* lappd_logic = lappd_->GetLogicalVolume();

      G4int angles = 6;
      G4double step = 2. * pi / angles;

      G4RotationMatrix *rot_z = new G4RotationMatrix();
        rot_z->rotateY(180 * deg);

      for (G4int itheta=0; itheta < angles; itheta++) {
        G4float theta = twopi / angles * itheta;
        std::string label = std::to_string(theta);
        G4double x = (detector_diam_ / 2. - 75 * cm) * std::sin(theta) * mm;
        G4double y = (detector_diam_ / 2. - 75 * cm) * std::cos(theta) * mm;
        new G4PVPlacement(0, G4ThreeVector(x, y, -detector_height_ / 2 + 5 * cm),
                          pmt_logic, "PMTDownout" + label, detector_logic,
                          false, 100+itheta, false);
        new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(x, y, detector_height_ / 2 - 5 * cm)),
                          pmt_logic, "PMTUpout" + label, detector_logic,
                          false, 400+itheta, false);
      }

      angles = 4;
      step = 2. * pi / angles;
      for (G4int itheta=0; itheta < angles; itheta++) {
        G4float theta = twopi / angles * itheta;
        std::string label = std::to_string(theta);
        G4double x = (detector_diam_ / 2. - 125 * cm) * std::sin(theta) * mm;
        G4double y = (detector_diam_ / 2. - 125 * cm) * std::cos(theta) * mm;
        new G4PVPlacement(0, G4ThreeVector(x, y, -detector_height_ / 2 + 5 * cm),
                          lappd_logic, "LAPPDDownout" + label, detector_logic,
                          false, 1100+itheta, false);
        new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(x, y, detector_height_ / 2 - 5 * cm)),
                          lappd_logic, "LAPPDUpout" + label, detector_logic,
                          false, 1400+itheta, false);
      }



      // angles = 3;
      // for (G4int itheta=0; itheta < angles; itheta++) {
      //   G4float theta = twopi / angles * itheta;
      //   std::string label = std::to_string(itheta);
      //   G4double x = (detector_diam_ / 2. - 120 * cm) * std::sin(theta) * mm;
      //   G4double y = (detector_diam_ / 2. - 120 * cm) * std::cos(theta) * mm;
      //   new G4PVPlacement(0, G4ThreeVector(x, y, -detector_height_ / 2 + 5 * cm),
      //                     pmt_logic, "PMTDownmid" + label, detector_logic,
      //                     false, 200+itheta, false);
      //   new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(x, y, detector_height_ / 2 - 5 * cm)),
      //                     pmt_logic, "PMTUpmid" + label, detector_logic,
      //                     false, 500+itheta, false);
      // }

      new G4PVPlacement(0, G4ThreeVector(0, 0, -detector_height_ / 2 + 5 * cm),
                        pmt_logic, "PMTDown0", detector_logic,
                        false, 300, false);
      new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, 0, detector_height_ / 2 - 5 * cm)),
                        pmt_logic, "PMTUp0", detector_logic,
                        false, 600, false);

      angles = 8;
      step = 2. * pi / angles;
      G4int rows = 6;
      for (G4int irow = 0; irow < rows; irow++) {
        G4RotationMatrix *rot = new G4RotationMatrix();
        rot->rotateX(90 * deg);
        for (G4int itheta=0; itheta < angles; itheta++) {
          G4float theta = 2 * M_PI / angles * itheta;
          std::string label = std::to_string(irow*angles + itheta);

          if (irow % 2 == 0) {
            theta += step / 2;
            if (itheta == 0)
              rot->rotateZ(-step/2);
          }

          G4double y = (detector_diam_ / 2. - 10 * cm / 2 - 1 * cm) * std::cos(theta) * mm;
          G4double x = (detector_diam_ / 2. - 10 * cm / 2 - 1 * cm) * std::sin(theta) * mm;

          new G4PVPlacement(G4Transform3D(*rot, G4ThreeVector(x, y, -detector_height_ / 2 + detector_height_/rows * irow + detector_height_/ rows / 2)),
                            pmt_logic, "PMT" + label, detector_logic,
                            false, irow*angles + itheta, true);
          rot->rotateZ(-step);
        }
      }

      G4int lappd_rows = 6;
      angles = 8;
      step = 2. * pi / angles;
      for (G4int irow = 0; irow < lappd_rows; irow++) {
        G4RotationMatrix *rot = new G4RotationMatrix();
        rot->rotateX(90 * deg);
        for (G4int itheta=0; itheta < angles; itheta++) {
          G4float theta = 2 * M_PI / angles * itheta;
          std::string label = std::to_string(irow*angles + itheta);

          G4int sign = -1;
          if (irow % 2 != 0) {
            theta += step / 2;
            if (itheta == 0)
              rot->rotateZ(-step/2);
            sign = 1;
          }

          G4double y = (detector_diam_ / 2. - 10 * cm / 2 - 1 * cm) * std::cos(theta) * mm;
          G4double x = (detector_diam_ / 2. - 10 * cm / 2 - 1 * cm) * std::sin(theta) * mm;

          if (irow == 1) sign = -1;

          new G4PVPlacement(G4Transform3D(*rot, G4ThreeVector(x, y, -detector_height_ /2  + detector_height_/lappd_rows * irow + detector_height_/lappd_rows / 2 )),
                            lappd_logic, "LAPPD" + label, detector_logic,
                            false, 1000+irow*angles + itheta, true);
          rot->rotateZ(-step);
        }
      }
    // } else { 
    //       G4double fiber_length =detector_height_ - 425*um*4;
    //       GenericWLSFiber* fiber_ = new GenericWLSFiber("Y11", true, 1 * mm, fiber_length, true, false, nullptr, materials::Y11(), true);
    //       fiber_->SetCoreOpticalProperties(opticalprops::Y11());
    //       fiber_->Construct();
    //       G4LogicalVolume* fiber_logic = fiber_->GetLogicalVolume();
    //       fiber_logic->SetVisAttributes(nexus::LightGreenAlpha());

    //       G4Material* fiber_end_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");

    //       G4double fiber_end_z = 0.1 * mm;

    //       G4Tubs* fiber_end_solid_vol =
    //         new G4Tubs("fiber_end", 0, 1 * mm/2, fiber_end_z, 0, 2 * M_PI);

    //       G4LogicalVolume* fiber_end_logic_vol =
    //         new G4LogicalVolume(fiber_end_solid_vol, fiber_end_mat, "FIBER_END");
    //       G4OpticalSurface* opsur_al =
    //         new G4OpticalSurface("POLISHED_AL_OPSURF", unified, polished, dielectric_metal);
    //       opsur_al->SetMaterialPropertiesTable(opticalprops::PolishedAl());

    //       new G4LogicalSkinSurface("POLISHED_AL_OPSURF", fiber_end_logic_vol, opsur_al);


    //       SiPM11 *sipm_  = new SiPM11();
    //       sipm_->Construct();
    //       G4LogicalVolume* sipm_logic = sipm_->GetLogicalVolume();

    //       G4double pitch = 1 * cm;

    //       for (uint ix=0; ix<detector_diam_/pitch; ix++) {
    //         for (uint iy=0; iy<detector_diam_/pitch; iy++) {
    //           if (sqrt(pow(ix*pitch-detector_diam_/2,2) + pow(iy*pitch-detector_diam_/2,2)) < detector_diam_/2-0.5*cm) {
    //             std::string label = std::to_string(ix * detector_diam_/pitch + iy);

    //             new G4PVPlacement(0, G4ThreeVector(ix*pitch-detector_diam_/2, iy*pitch-detector_diam_/2, 0),
    //                     fiber_logic, "FIBER" + label, detector_logic,
    //                     true, ix * 10000 + iy, true);
    //             new G4PVPlacement(0, G4ThreeVector(ix*pitch-detector_diam_/2, iy*pitch-detector_diam_/2, -fiber_length/2-0.1*mm),
    //                     fiber_end_logic_vol, "AL_ENDCAP" + label, detector_logic,
    //                     true, ix * 10000 + iy, true);
    //             new G4PVPlacement(0, G4ThreeVector(ix*pitch-detector_diam_/2, iy*pitch-detector_diam_/2, detector_height_/2-425*um),
    //                     sipm_logic, "SiPM"+label, detector_logic,
    //                     true, ix * 10000 + iy, true);
    //           }
    //         }
    //       }

    }
  }

  G4ThreeVector ESSBeam::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0,0. * m,0);
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
      // return vertex;
    } else {
      G4Exception("[ESSBeam]", "GenerateVertex()", FatalException,
                  "Unknown vertex generation region!");
    }

    return vertex;
  }
  
}
