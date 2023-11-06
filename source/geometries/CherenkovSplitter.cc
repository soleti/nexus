#include "CherenkovSplitter.h"

#include <G4Box.hh>
#include <G4Tubs.hh>

#include <G4GenericMessenger.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4SubtractionSolid.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4PVPlacement.hh>
#include <Randomize.hh>
#include <G4VisAttributes.hh>
#include <G4LogicalBorderSurface.hh>

#include "GenericWLSFiber.h"
#include "FactoryBase.h"
#include "MaterialsList.h"
#include "Visibilities.h"
#include "OpticalMaterialProperties.h"
#include "GenericPhotosensor.h"
#include "GenericSquarePhotosensor.h"
#include "SiPM11.h"

using namespace nexus;

REGISTER_CLASS(CherenkovSplitter, GeometryBase)

namespace nexus {

  using namespace CLHEP;

  CherenkovSplitter::CherenkovSplitter():
    GeometryBase()
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/CherenkovSplitter/",
                                  "Control commands of geometry CherenkovSplitter.");
  }

  CherenkovSplitter::~CherenkovSplitter()
  {
    delete msg_;
  }

 void CherenkovSplitter::Construct()
  {
    G4ThreeVector *detector_center;
    detector_center = new G4ThreeVector(0, 0, 0);

    G4double size = 1. * m;
    G4Box* world_solid =
      new G4Box("LAB", size/2., size/2., size/2.);

    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    air->SetMaterialPropertiesTable(opticalprops::Vacuum());
    G4LogicalVolume* world_logic = new G4LogicalVolume(world_solid, air, "WORLD");

    world_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(world_logic);

    G4double vial_height = 23.7 * mm;
    G4double vial_diam = 20.5 * mm;

    G4Tubs* vial_solid =
      new G4Tubs("VIAL",  0., vial_diam/2., vial_height/2., 0, twopi);

    G4Material *vial_mat = materials::LAB();
    vial_mat->SetMaterialPropertiesTable(opticalprops::GdLS());

    G4LogicalVolume* vial_logic =
      new G4LogicalVolume(vial_solid, vial_mat, "VIAL");

    vial_logic->SetVisAttributes(nexus::LightBlueAlpha());
    G4VPhysicalVolume* vial_position = new G4PVPlacement(0, *detector_center,
                      vial_logic, "VIAL", world_logic, false, 0, true);


    G4double container_diam   = vial_diam+1*mm;
    G4Tubs* container_full =
      new G4Tubs("CONTAINER_FULL", 0, container_diam/2., vial_height/2., 0, twopi);
    G4SubtractionSolid *container = new G4SubtractionSolid("CONTAINER",
							   container_full,
							   vial_solid);

    G4Material *container_mat = materials::FusedSilica();
    container_mat->SetMaterialPropertiesTable(opticalprops::FusedSilica());
    G4LogicalVolume* container_logic =
      new G4LogicalVolume(container, container_mat, "CONTAINER");
    container_logic->SetVisAttributes(nexus::WhiteAlpha());

    G4VPhysicalVolume* container_position = new G4PVPlacement(0, *detector_center, container_logic,
                       "CONTAINER", world_logic, false, 0, false);


    // new G4LogicalBorderSurface(
    //   "VIAL_PTFE", vial_position, lab_position, ptfe_surface);
    GenericSquarePhotosensor *pmt  = new GenericSquarePhotosensor("PMT", 1 *cm, 1*cm, 1* cm);

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

    G4double energy[]       = {opticalprops::optPhotMinE_, opticalprops::optPhotMaxE_};
    G4double reflectivity[] = {0.0     , 0.0     };
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 2);
    photosensor_mpt->AddProperty("EFFICIENCY", eff_energies, eff, entries);
    pmt->SetOpticalProperties(photosensor_mpt);

    pmt->SetTimeBinning(1 * us);
    pmt->SetVisibility(true);
    pmt->SetWindowRefractiveIndex(opticalprops::BorosilicateGlass()->GetProperty("RINDEX"));
    pmt->SetSensorDepth(1);
    pmt->Construct();
    G4LogicalVolume* pmt_logic = pmt->GetLogicalVolume();

    G4RotationMatrix *rot_z = new G4RotationMatrix();
      rot_z->rotateX(90 * deg);

    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, 2 *cm, 0)),
                      pmt_logic, "PMT", world_logic,
                      false, 1, false);
  }

  G4ThreeVector CherenkovSplitter::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0,0,0*mm);

    return vertex;
  }
  
}
