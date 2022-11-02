// ----------------------------------------------------------------------------
// nexus | Block.cc
//
// Block coated with an optical material
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "Block.h"

#include "FactoryBase.h"

#include <G4GenericMessenger.hh>

using namespace nexus;

REGISTER_CLASS(Block, GeometryBase)

namespace nexus {

  Block::Block():
    GeometryBase(), 
    box_x_(0),
    box_y_(0),
    box_z_(0),
    box_material_("Pb"),
    box_optical_coating_("Teflon")
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/Block/",
      "Control commands of geometry Block.");

    msg_->DeclareProperty("box_material", box_material_,
      "Material of the block.");
    msg_->DeclareProperty("box_optical_coating", box_optical_coating_,
      "Block optical coating.");
    msg_->DeclareProperty("box_x", box_x_,
      "Block x dimension.");
    msg_->DeclareProperty("box_y", box_y_,
      "Block y dimension.");
    msg_->DeclareProperty("box_z", box_z_,
      "Block z dimension.");

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

  }



  G4ThreeVector Block::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0.,0.,0.);

    // WORLD
    if (region == "CENTER") {
      return vertex;
    }
    else {
      G4Exception("[BlackBox]", "GenerateVertex()", FatalException,
		  "Unknown vertex generation region!");
    }
    return vertex;
  }


} // end namespace nexus
