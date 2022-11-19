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
    length_ (2 * cm),
    coating_ ("TPB"),
    fiber_type_ ("Y11"),
    coated_(true)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/FiberBox/",
      "Control commands of geometry FiberBox.");

    G4GenericMessenger::Command&  radius_cmd =
      msg_->DeclareProperty("radius", radius_,
                            "Barrel fiber radius");
    radius_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  length_cmd =
      msg_->DeclareProperty("length", length_,
                            "Barrel fiber length");
    length_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  fiber_radius_cmd =
      msg_->DeclareProperty("fiber_radius", fiber_radius_,
                            "Fiber radius");
    fiber_radius_cmd.SetUnitCategory("Length");

    msg_->DeclareProperty("coating", coating_, "Fiber coating (TPB or PTH)");
    msg_->DeclareProperty("fiber_type", fiber_type_, "Fiber type (Y11 or B2)");
    msg_->DeclareProperty("coated", coated_, "Coat fibers with WLS coating");

  }



  FiberBox::~FiberBox()
  {
    delete msg_;
  }



  void FiberBox::Construct()
  {
    inside_cylinder_ = new CylinderPointSampler2020(0, radius_, length_/2, 0, 2 * M_PI);

    world_z_ = length_ * 2;
    world_xy_ = radius_ * 2;

    G4Material *this_fiber = materials::Y11();
    G4MaterialPropertiesTable *this_fiber_optical = opticalprops::Y11();
    if (fiber_type_ == "B2") {
      this_fiber = materials::B2();
      this_fiber_optical = opticalprops::B2();
    }

    G4Material *this_coating = nullptr;
    G4MaterialPropertiesTable *this_coating_optical = nullptr;
    if (coated_) {
      if (coating_ == "TPB") {
        this_coating = materials::TPB();
        this_coating_optical = opticalprops::TPB();
      } else if (coating_ == "TPH") {
        this_coating = materials::TPH();
        this_coating_optical = opticalprops::TPH();
      }
    }

    fiber_ = new GenericWLSFiber(fiber_type_, true, fiber_radius_, length_, true, coated_, this_coating, this_fiber, true);

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

    // FIBER ////////////////////////////////////////////////////

    fiber_->SetCoreOpticalProperties(this_fiber_optical);
    fiber_->SetCoatingOpticalProperties(this_coating_optical);

    fiber_->Construct();
    G4LogicalVolume* fiber_logic = fiber_->GetLogicalVolume();

    G4int n_fibers = (radius_ * 2 * M_PI) / fiber_radius_;

    // FAKE DETECTOR /////////////////////////////////////////////

    G4Material* disk_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");

    G4double disk_z = 0.1 * mm;

    G4Tubs* disk_solid_vol =
      new G4Tubs("disk", 0, fiber_radius_/2, disk_z, 0, 2 * M_PI);

    G4LogicalVolume* disk_logic_vol =
      new G4LogicalVolume(disk_solid_vol, disk_mat, "DISK");

    G4OpticalSurface* opsur =
      new G4OpticalSurface("PERFECT_OPSURF", unified, polished, dielectric_metal);
    opsur->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());

    new G4LogicalSkinSurface("PERFECT_OPSURF", disk_logic_vol, opsur);

    // PLACEMENT /////////////////////////////////////////////

    for (G4int itheta=0; itheta <= n_fibers; itheta++) {

      G4float theta = 2 * M_PI / n_fibers * itheta;
      G4double x = radius_ * std::cos(theta) * mm;
      G4double y = radius_ * std::sin(theta) * mm;
      new G4PVPlacement(0, G4ThreeVector(x,y),
                        fiber_logic, "B2", world_logic_vol,
                        false, itheta, false);
      new G4PVPlacement(0, G4ThreeVector(x,y,length_/2 + disk_z),
                        disk_logic_vol, "DISK", world_logic_vol,
                        false, itheta, false);
      new G4PVPlacement(0, G4ThreeVector(x,y,-(length_/2 + disk_z)),
                        disk_logic_vol, "DISK", world_logic_vol,
                        false, n_fibers+itheta, false);
    }

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