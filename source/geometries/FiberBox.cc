// ----------------------------------------------------------------------------
// nexus | FiberBox.cc
//
// Box containing optical fibers
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "FiberBox.h"

#include "FactoryBase.h"
#include "OpticalMaterialProperties.h"
#include "MaterialsList.h"

#include <G4GenericMessenger.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4VisAttributes.hh>

using namespace nexus;

REGISTER_CLASS(FiberBox, GeometryBase)

namespace nexus {

  FiberBox::FiberBox():
    GeometryBase(),
    world_z_ (3. * m),
    world_xy_ (2. *m)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/FiberBox/",
      "Control commands of geometry FiberBox.");

    G4Material* ps = materials::PS();
    G4Material* tpb = materials::TPB();
    fiber_ = new GenericWLSFiber("Y11", true, 1 * mm, 1 * cm, true, true, tpb, ps, true);
  }



  FiberBox::~FiberBox()
  {
    delete msg_;
  }



  void FiberBox::Construct()
  {

    // WORLD /////////////////////////////////////////////////

    G4String world_name = "WORLD";

    G4Material* world_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

    world_mat->SetMaterialPropertiesTable(opticalprops::Vacuum());

    G4Box* world_solid_vol =
     new G4Box(world_name, world_xy_/2., world_xy_/2., world_z_/2.);

    G4LogicalVolume* world_logic_vol =
      new G4LogicalVolume(world_solid_vol, world_mat, world_name);
    world_logic_vol->SetVisAttributes(G4VisAttributes::GetInvisible());
    GeometryBase::SetLogicalVolume(world_logic_vol);
    fiber_->SetCoreOpticalProperties(opticalprops::Y11());
    fiber_->Construct();
    G4LogicalVolume* fiber_logic = fiber_->GetLogicalVolume();

    new G4PVPlacement(0, G4ThreeVector(0., 0., 0),
                      fiber_logic, "Y11", world_logic_vol,
                      false, 0, false);

   // BLOCK //////////////////////////////////////////////////

    G4Material* block_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");

    G4Box* block_solid_vol_right =
      new G4Box("block", 1 * mm, 1 * mm, 1 * mm);

    G4LogicalVolume* block_logic_vol_right =
      new G4LogicalVolume(block_solid_vol_right, block_mat, "BLOCK_RIGHT");

    new G4PVPlacement(0, G4ThreeVector(0,0,-0.6*cm),
                      block_logic_vol_right, "BLOCK_RIGHT", world_logic_vol,
                      false, 0, false);

    G4OpticalSurface* opsur =
      new G4OpticalSurface("PERFECT_OPSURF", unified, polished, dielectric_metal);
    opsur->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());

    new G4LogicalSkinSurface("PERFECT_OPSURF", block_logic_vol_right, opsur);

    G4Box* block_solid_vol_left =
      new G4Box("block", 1 * mm, 1 * mm, 1 * mm);

    G4LogicalVolume* block_logic_vol_left =
      new G4LogicalVolume(block_solid_vol_left, block_mat, "BLOCK_LEFT");

    new G4PVPlacement(0, G4ThreeVector(0,0,0.6*cm),
                      block_logic_vol_left, "BLOCK_LEFT", world_logic_vol,
                      false, 0, false);

    new G4LogicalSkinSurface("PERFECT_OPSURF", block_logic_vol_left, opsur);

  }



  G4ThreeVector FiberBox::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0.,10.,0. * cm);

    // WORLD
    if (region == "CENTER") {
      return vertex;
    }
    else {
      G4Exception("[FiberBox]", "GenerateVertex()", FatalException,
		  "Unknown vertex generation region!");
    }
    return vertex;
  }


} // end namespace nexus