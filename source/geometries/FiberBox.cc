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

#include <G4GenericMessenger.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4VisAttributes.hh>

using namespace nexus;

REGISTER_CLASS(FiberBox, GeometryBase)

namespace nexus {

  FiberBox::FiberBox():
    GeometryBase(),
    radius_ (1 * cm),
    fiber_radius_(1 * mm),
    length_ (2 * cm)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/FiberBox/",
      "Control commands of geometry FiberBox.");

    G4GenericMessenger::Command&  radius_cmd =
      msg_->DeclareProperty("radius", radius_,
                            "Barrel fiber radius");
    radius_cmd.SetParameterName("radius", true);
    radius_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  length_cmd =
      msg_->DeclareProperty("length", length_,
                            "Barrel fiber length");
    length_cmd.SetParameterName("length", true);
    length_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  fiber_radius_cmd =
      msg_->DeclareProperty("fiber_radius", fiber_radius_,
                            "Fiber radius");
    fiber_radius_cmd.SetParameterName("fiber_radius", true);
    fiber_radius_cmd.SetUnitCategory("Length");
  }



  FiberBox::~FiberBox()
  {
    delete msg_;
  }



  void FiberBox::Construct()
  {

    world_z_ = length_ * 2;
    world_xy_ = radius_ * 2;

    // G4Material* coating = G4NistManager::Instance()->FindOrBuildMaterial("TPH");

    fiber_ = new GenericWLSFiber("B2", true, fiber_radius_, length_, true, true, materials::TPH(), materials::B2(), true);
    inside_cylinder_ = new CylinderPointSampler2020(0, radius_, length_/2, 0, 2 * M_PI);

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

    fiber_->SetCoreOpticalProperties(opticalprops::B2());
    fiber_->SetCoatingOpticalProperties(opticalprops::TPH());

    fiber_->Construct();
    G4LogicalVolume* fiber_logic = fiber_->GetLogicalVolume();

    G4int n_fibers = (radius_ * 2 * M_PI) / fiber_radius_;
   // BLOCK //////////////////////////////////////////////////

    G4Material* block_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");

    G4double block_z = 0.1 * mm;
    // G4Box* block_solid_vol =
    //   new G4Box("block", fiber_radius_/2, fiber_radius_/2, block_z);
    G4Tubs* block_solid_vol =
      new G4Tubs("disk", 0, fiber_radius_/2, block_z, 0, 2 * M_PI);
    G4LogicalVolume* block_logic_vol =
      new G4LogicalVolume(block_solid_vol, block_mat, "BLOCK");

    G4OpticalSurface* opsur =
      new G4OpticalSurface("PERFECT_OPSURF", unified, polished, dielectric_metal);
    opsur->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());

    new G4LogicalSkinSurface("PERFECT_OPSURF", block_logic_vol, opsur);

// G4CSGSolid(pName), fRMin(pRMin), fRMax(pRMax), fDz(pDz), fSPhi(0), fDPhi(0)


    // G4LogicalVolume* block_logic_vol =
    //   new G4LogicalVolume(block_solid_vol, block_mat, "BLOCK");

    // G4OpticalSurface* opsur =
    //   new G4OpticalSurface("PERFECT_OPSURF", unified, polished, dielectric_metal);
    // opsur->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());

    // new G4LogicalSkinSurface("PERFECT_OPSURF", block_logic_vol, opsur);


    for (G4int itheta=0; itheta <= n_fibers; itheta++) {

      G4float theta = 2 * M_PI / n_fibers * itheta;
      G4double x = radius_ * std::cos(theta) * mm;
      G4double y = radius_ * std::sin(theta) * mm;
      new G4PVPlacement(0, G4ThreeVector(x,y),
                        fiber_logic, "B2", world_logic_vol,
                        false, itheta, false);
      new G4PVPlacement(0, G4ThreeVector(x,y,length_/2 + 0.1*mm),
                        block_logic_vol, "BLOCK", world_logic_vol,
                        false, itheta, false);
      new G4PVPlacement(0, G4ThreeVector(x,y,-(length_/2 + 0.1*mm)),
                    block_logic_vol, "BLOCK", world_logic_vol,
                    false, n_fibers+itheta, false);
    }

    // pmt_.SetSensorDepth(3);
    // pmt_.Construct();
    // G4LogicalVolume* pmt_logic = pmt_.GetLogicalVolume();
    // //   pmt_length_ = pmt_.Length() // this is R7378A
    // pmt_length_ = 20*cm; // this is R11410

    // new G4PVPlacement(0, G4ThreeVector(0.,0.,pmt_length_/2.),
		//       pmt_logic, "PMT",
		//       world_logic_vol, false, 0, true);


  }



  G4ThreeVector FiberBox::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0.,0.,0.);

    // WORLD
    if (region == "CENTER") {
      return inside_cylinder_->GenerateVertex("VOLUME");;
    }
    else {
      G4Exception("[FiberBox]", "GenerateVertex()", FatalException,
		  "Unknown vertex generation region!");
    }
    return vertex;
  }


} // end namespace nexus