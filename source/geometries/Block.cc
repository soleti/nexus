// ----------------------------------------------------------------------------
// nexus | Block.cc
//
// Block coated with an optical material
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "Block.h"

#include "FactoryBase.h"
#include "OpticalMaterialProperties.h"

#include <G4GenericMessenger.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4VisAttributes.hh>

using namespace nexus;

REGISTER_CLASS(Block, GeometryBase)

namespace nexus {

  Block::Block():
    GeometryBase(),
    world_z_ (3. * m),
    world_xy_ (2. *m),
    box_x_(0),
    box_y_(0),
    box_z_(0),
    box_material_("G4_Pb"),
    box_optical_coating_("Teflon")
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/Block/",
      "Control commands of geometry Block.");

    msg_->DeclareProperty("material", box_material_,
      "Material of the block.");
    msg_->DeclareProperty("optical_coating", box_optical_coating_,
      "Block optical coating.");

    G4GenericMessenger::Command& box_x_cmd =
      msg_->DeclareProperty("box_x", box_x_, "Block x dimension.");
    box_x_cmd.SetUnitCategory("Length");
    box_x_cmd.SetParameterName("box_x", false);
    box_x_cmd.SetRange("box_x>0.");

    G4GenericMessenger::Command& box_y_cmd =
      msg_->DeclareProperty("box_y", box_y_, "Block y dimension.");
    box_y_cmd.SetUnitCategory("Length");
    box_y_cmd.SetParameterName("box_y", false);
    box_y_cmd.SetRange("box_y>0.");

    G4GenericMessenger::Command& box_z_cmd =
      msg_->DeclareProperty("box_z", box_z_, "Block z dimension.");
    box_z_cmd.SetUnitCategory("Length");
    box_z_cmd.SetParameterName("box_z", false);
    box_z_cmd.SetRange("box_z>0.");
  }



  Block::~Block()
  {
    delete msg_;
  }



  void Block::Construct()
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


    // BLOCK //////////////////////////////////////////////////

    G4String block_name = "BLOCK";
    G4Material* block_mat = G4NistManager::Instance()->FindOrBuildMaterial(box_material_);

    G4Box* block_solid_vol =
      new G4Box("block", box_x_, box_y_, box_z_);

    G4LogicalVolume* block_logic_vol =
      new G4LogicalVolume(block_solid_vol, block_mat, block_name);

    new G4PVPlacement(0, G4ThreeVector(0.,0.,0.),
                      block_logic_vol, block_name, world_logic_vol,
                      false, 0, false);

    G4OpticalSurface* opsur = 
      new G4OpticalSurface("PERFECT_OPSURF", unified, polished, dielectric_metal);
    opsur->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());

    new G4LogicalSkinSurface("PERFECT_OPSURF", block_logic_vol, opsur);
  }



  G4ThreeVector Block::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0.,0.,20. * cm);

    // WORLD
    if (region == "CENTER") {
      return vertex;
    }
    else {
      G4Exception("[Block]", "GenerateVertex()", FatalException,
		  "Unknown vertex generation region!");
    }
    return vertex;
  }


} // end namespace nexus
