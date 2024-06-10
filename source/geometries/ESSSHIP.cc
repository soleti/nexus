#include "ESSSHIP.h"

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

REGISTER_CLASS(ESSSHIP, GeometryBase)

namespace nexus {

  using namespace CLHEP;

  ESSSHIP::ESSSHIP():
    GeometryBase(),
    beam_(true)
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/ESSSHIP/",
                                  "Control commands of geometry ESSSHIP.");
    msg_->DeclareProperty("beam", beam_,
                          "Simulate beam");
  }

  ESSSHIP::~ESSSHIP()
  {
    delete msg_;
  }

 void ESSSHIP::Construct()
  {
    G4double size = 80. * m;
    G4Box* lab_solid =
      new G4Box("LAB", size/2., size/2., size/2.);

    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");

    G4LogicalVolume* lab_logic = new G4LogicalVolume(lab_solid, air, "LAB");

    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic);

    // roof_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

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
      moderator_logic->SetVisAttributes(nexus::LightGreyAlpha());

      // Then, put air inside it
      G4double mod_thickess = 5. * cm;
      G4Tubs* inner_air_solid =
        new G4Tubs("INNER_AIR",  0., (mod_diam - 2*mod_thickess)/2., (mod_height - 2*mod_thickess)/2., 0, twopi);
      G4LogicalVolume* inner_air_logic =
        new G4LogicalVolume(inner_air_solid, air, "INNER_AIR");
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), inner_air_logic,
                        "INNER_AIR", moderator_logic, false, 0, true);
      inner_air_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

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

    // Now we create an empty decay volume that is 15 m far from the beam target and 5 m long
    // aligned with the beam direction

    G4double decay_vol_length = 5. * m;
    G4double decay_vol_diam = 2. * m;
    G4Tubs* decay_vol_solid =
      new G4Tubs("DECAY_VOL",  0., decay_vol_diam/2., decay_vol_length/2., 0, twopi);
    G4LogicalVolume* decay_vol_logic =
      new G4LogicalVolume(decay_vol_solid, air, "DECAY_VOL");
    decay_vol_logic->SetVisAttributes(nexus::LightBlueAlpha());
    G4RotationMatrix *rot_x = new G4RotationMatrix();
      rot_x->rotateX(90 * deg);
    new G4PVPlacement(G4Transform3D(*rot_x, G4ThreeVector(0., 17.5 * m + decay_vol_length / 2, 0)), decay_vol_logic,
                      "DECAY_VOL", lab_logic, false, 0, true);

    // Now we create the TPC volume, aligned with the decay volume and 0.5 m long
    G4double tpc_length = 0.5 * m;
    G4double tpc_xz = 2. * m;
    G4Box* tpc_solid =
      new G4Box("TPC", tpc_xz/2., tpc_xz/2., tpc_length/2.);
    G4LogicalVolume* tpc_logic =
      new G4LogicalVolume(tpc_solid, air, "TPC");
    tpc_logic->SetVisAttributes(nexus::LightGreenAlpha());
    new G4PVPlacement(G4Transform3D(*rot_x, G4ThreeVector(0., 17.5 * m + decay_vol_length + tpc_length/2. + 10 * cm, 0)), tpc_logic,
                      "TPC", lab_logic, false, 0, true);

    // Now we create the calorimeter volume, same size as the TPC
    G4Box* calorimeter_solid =
      new G4Box("CALORIMETER", tpc_xz/2., tpc_xz/2., tpc_length/2.);
    G4LogicalVolume* calorimeter_logic =
      new G4LogicalVolume(calorimeter_solid, air, "CALORIMETER");
    calorimeter_logic->SetVisAttributes(nexus::RedAlpha());
    new G4PVPlacement(G4Transform3D(*rot_x, G4ThreeVector(0., 17.5 * m + decay_vol_length + tpc_length + tpc_length/2. + 20 * cm, 0)), calorimeter_logic,
                      "CALORIMETER", lab_logic, false, 0, true);

  }

  G4ThreeVector ESSSHIP::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0,0. * m,0);
    G4double y_starting_point = -1.75 * m;

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
    } else {
      G4Exception("[ESSSHIP]", "GenerateVertex()", FatalException,
                  "Unknown vertex generation region!");
    }

    return vertex;
  }
  
}
