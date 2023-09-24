// ----------------------------------------------------------------------------
// nexus | FiberBarrel.cc
//
// Box containing optical fibers
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "FiberBarrel.h"

#include "FactoryBase.h"
#include "OpticalMaterialProperties.h"
#include "Visibilities.h"

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

REGISTER_CLASS(FiberBarrel, GeometryBase)

namespace nexus {

  FiberBarrel::FiberBarrel():
    GeometryBase(),
    radius_ (1 * cm),
    fiber_diameter_(1 * mm),
    length_ (2 * cm),
    coating_ ("TPB"),
    fiber_type_ ("Y11"),
    coated_(true)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/FiberBarrel/",
      "Control commands of geometry FiberBarrel.");

    G4GenericMessenger::Command&  radius_cmd =
      msg_->DeclareProperty("radius", radius_,
                            "Barrel fiber radius");
    radius_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  length_cmd =
      msg_->DeclareProperty("length", length_,
                            "Barrel fiber length");
    length_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  fiber_diameter_cmd =
      msg_->DeclareProperty("fiber_diameter", fiber_diameter_,
                            "Fiber diameter");
    fiber_diameter_cmd.SetUnitCategory("Length");

    msg_->DeclareProperty("coating", coating_, "Fiber coating (TPB or PTH)");
    msg_->DeclareProperty("fiber_type", fiber_type_, "Fiber type (Y11 or B2)");
    msg_->DeclareProperty("coated", coated_, "Coat fibers with WLS coating");

  }



  FiberBarrel::~FiberBarrel()
  {
    delete msg_;
  }



  void FiberBarrel::Construct()
  {

    G4cout << "[FiberBarrel] *** Barrel Fiber prototype ***" << G4endl;
    G4cout << "[FiberBarrel] Using " << fiber_type_ << " fibers";
    if (coated_)
      G4cout << " with " << coating_ << " coating";
    G4cout << G4endl;

    inside_cylinder_ = new CylinderPointSampler2020(0, radius_, length_/2, 0, 2 * M_PI);

    world_z_ = length_ * 4;
    world_xy_ = radius_ * 4;


    G4Material *this_fiber = nullptr;
    G4MaterialPropertiesTable *this_fiber_optical = nullptr;
    if (fiber_type_ == "Y11") {
      this_fiber = materials::Y11();
      this_fiber_optical = opticalprops::Y11();
    } else if (fiber_type_ == "B2") {
      this_fiber = materials::B2();
      this_fiber_optical = opticalprops::B2();
    } else {
      G4Exception("[FiberBarrel]", "Construct()",
                  FatalException, "Invalid fiber type, must be Y11 or B2");
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
      } else {
        G4Exception("[FiberBarrel]", "Construct()",
                    FatalException, "Invalid coating, must be TPB or TPH");
      }
    }

    fiber_ = new GenericWLSFiber(fiber_type_, true, fiber_diameter_, length_, true, coated_, this_coating, this_fiber, true);

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

    // TEFLON PANELS ////////////////////////////////////////////
    G4Tubs* teflon_panel =
      new G4Tubs("TEFLON_PANEL", 0, radius_ - fiber_diameter_ / 2, 5 * mm, 0, twopi);
    G4Material* teflon = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
    teflon->SetMaterialPropertiesTable(opticalprops::PTFE());
    G4LogicalVolume* teflon_logic =
      new G4LogicalVolume(teflon_panel, teflon, "TEFLON");

    G4OpticalSurface* opsur_teflon =
      new G4OpticalSurface("TEFLON_OPSURF", unified, groundteflonair, dielectric_metal);
    opsur_teflon->SetMaterialPropertiesTable(opticalprops::PTFE());

    new G4LogicalSkinSurface("TEFLON_OPSURF", teflon_logic, opsur_teflon);

    new G4PVPlacement(0, G4ThreeVector(0, 0, -length_/2 - 5 * mm),
                      teflon_logic, "TEFLON1", world_logic_vol,
                      true, 0, false);

    new G4PVPlacement(0, G4ThreeVector(0, 0, length_/2 + 5 * mm),
                      teflon_logic, "TEFLON2", world_logic_vol,
                      true, 1, false);

    // TEFLON CYLINDER

    G4Tubs* teflon_cylinder =
      new G4Tubs("TEFLON_CYLINDER", radius_ + fiber_diameter_ / 2, radius_ + fiber_diameter_ / 2 + 1 * cm, length_/2, 0, twopi);
    G4LogicalVolume* teflon_cylinder_logic =
      new G4LogicalVolume(teflon_cylinder, teflon, "TEFLON");

    new G4LogicalSkinSurface("TEFLON_CYLINDER_OPSURF", teflon_cylinder_logic, opsur_teflon);

    new G4PVPlacement(0, G4ThreeVector(0, 0),
                      teflon_cylinder_logic, "TEFLON_CYLINDER", world_logic_vol,
                      false, 0, false);

    // FIBER ////////////////////////////////////////////////////

    fiber_->SetCoreOpticalProperties(this_fiber_optical);
    fiber_->SetCoatingOpticalProperties(this_coating_optical);

    fiber_->Construct();
    G4LogicalVolume* fiber_logic = fiber_->GetLogicalVolume();
    if (fiber_type_ == "Y11")
      fiber_logic->SetVisAttributes(nexus::LightGreenAlpha());
    else if (fiber_type_ == "B2")
      fiber_logic->SetVisAttributes(nexus::LightBlueAlpha());

    G4int n_fibers = floor((radius_ * 2 * M_PI) / fiber_diameter_);
    G4cout << "[FiberBarrel] Barrel with " << n_fibers << " fibers" << G4endl;

    // FAKE DETECTOR /////////////////////////////////////////////

    G4Material* disk_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");

    G4double disk_z = 0.1 * mm;

    G4Tubs* disk_solid_vol =
      new G4Tubs("disk", 0, fiber_diameter_/2, disk_z, 0, 2 * M_PI);

    G4LogicalVolume* disk_logic_vol =
      new G4LogicalVolume(disk_solid_vol, disk_mat, "DISK");

    G4OpticalSurface* opsur =
      new G4OpticalSurface("PERFECT_OPSURF", unified, polished, dielectric_metal);
    opsur->SetMaterialPropertiesTable(opticalprops::PerfectAbsorber());

    new G4LogicalSkinSurface("PERFECT_OPSURF", disk_logic_vol, opsur);

    // ALUMINIZED ENDCAP

    G4Material* fiber_end_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");

    G4double fiber_end_z = 350 * nm;

    G4Tubs* fiber_end_solid_vol =
      new G4Tubs("fiber_end", 0, fiber_diameter_ / 2, fiber_end_z, 0, 2 * M_PI);

    G4LogicalVolume* fiber_end_logic_vol =
      new G4LogicalVolume(fiber_end_solid_vol, fiber_end_mat, "FIBER_END");
    // G4OpticalSurface* opsur_al =
    //   new G4OpticalSurface("POLISHED_AL_OPSURF", unified, polished, dielectric_metal);
    G4OpticalSurface* opsur_al =
      new G4OpticalSurface("POLISHED_AL_OPSURF", glisur, ground, dielectric_metal);
    opsur_al->SetPolish(0.75);
    opsur_al->SetMaterialPropertiesTable(opticalprops::PolishedAl());

    new G4LogicalSkinSurface("POLISHED_AL_OPSURF", fiber_end_logic_vol, opsur_al);

    // PLACEMENT /////////////////////////////////////////////

    for (G4int itheta=0; itheta < n_fibers; itheta++) {

      G4double theta = 2 * M_PI / n_fibers * itheta;
      G4double x = radius_ * std::cos(theta) * mm;
      G4double y = radius_ * std::sin(theta) * mm;
      std::string label = std::to_string(itheta);

      new G4PVPlacement(0, G4ThreeVector(x,y),
                        fiber_logic, "B2-"+label, world_logic_vol,
                        true, itheta, false);
      new G4PVPlacement(0, G4ThreeVector(x,y,length_/2 + disk_z),
                        disk_logic_vol, "DISKL-" + label, world_logic_vol,
                        true, itheta+1000, false);
      new G4PVPlacement(0, G4ThreeVector(x,y,-(length_/2 + disk_z)),
                        fiber_end_logic_vol, "ALUMINIUMR-" + label, world_logic_vol,
                        true, n_fibers+itheta, false);
    }

  }



  G4ThreeVector FiberBarrel::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0.,0.,0.);

    // WORLD
    if (region == "INSIDE_BARREL") {
      return inside_cylinder_->GenerateVertex("VOLUME");
    }
    else {
      G4Exception("[FiberBarrel]", "GenerateVertex()", FatalException,
		  "Unknown vertex generation region!");
    }
    return vertex;
  }


} // end namespace nexus