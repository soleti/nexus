// ----------------------------------------------------------------------------
// nexus | NextPrecdr.cc
//
// Cylinder filled with xenon, surrounded by a copper shell.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "NextPrecdr.h"
#include "Visibilities.h"

#include "CylinderPointSampler.h"
#include "MaterialsList.h"
#include "IonizationSD.h"
#include "FactoryBase.h"

#include <G4GenericMessenger.hh>
#include <G4UserLimits.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>
#include <G4UnionSolid.hh>
#include <G4VUserDetectorConstruction.hh>

#include <CLHEP/Units/SystemOfUnits.h>

using namespace nexus;
using namespace CLHEP;

REGISTER_CLASS(NextPrecdr, GeometryBase)

namespace nexus {

  NextPrecdr::NextPrecdr():
    GeometryBase(), liquid_(true), pressure_(STP_Pressure),
    radius_(2.*m), height_(4.*m), shell_thickness_(4.*cm),
    plate_thickness_(3 * mm),
    xenon_vertex_gen_(nullptr), shell_vertex_gen_(nullptr)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/NextPrecdr/",
      "Control commands of geometry NextPrecdr.");

    msg_->DeclareProperty("LXe", liquid_,
      "Build the cylinder with liquid xenon.");

    G4GenericMessenger::Command& pressure_cmd =
      msg_->DeclareProperty("pressure", pressure_,
      "Set pressure for gaseous xenon (if selected).");
    pressure_cmd.SetUnitCategory("Pressure");
    pressure_cmd.SetParameterName("pressure", false);
    pressure_cmd.SetRange("pressure>0.");

    G4GenericMessenger::Command& radius_cmd =
      msg_->DeclareProperty("radius", radius_, "Inner radius of the xenon cylinder.");
    radius_cmd.SetUnitCategory("Length");
    radius_cmd.SetParameterName("radius", false);
    radius_cmd.SetRange("radius>0.");

    G4GenericMessenger::Command& height_cmd =
      msg_->DeclareProperty("height", height_, "Height of the xenon cylinder.");
    height_cmd.SetUnitCategory("Length");
    height_cmd.SetParameterName("height", false);
    height_cmd.SetRange("height>0.");

    G4GenericMessenger::Command& shell_cmd =
      msg_->DeclareProperty("shell_thickness", shell_thickness_, "Thickness of copper shell.");
    shell_cmd.SetUnitCategory("Length");
    shell_cmd.SetParameterName("shell_thickness", false);
    shell_cmd.SetRange("shell_thickness>0.");

    // Create vertex generators for xenon and shell
    xenon_vertex_gen_ = new CylinderPointSampler(0., radius_, height_/2., 0., 2*pi);
    shell_vertex_gen_ = new CylinderPointSampler(radius_, radius_ + shell_thickness_, height_/2., 0., 2*pi);
    // Endcaps: thin disks at top and bottom
    shell_endcap_vertex_gen_ = new CylinderPointSampler(0., radius_ + shell_thickness_, shell_thickness_/2., 0., 2*pi);
    surface_plate_vertex_gen_ = new CylinderPointSampler(0, radius_, 0, 0., 2*pi);
  }


  NextPrecdr::~NextPrecdr()
  {
    delete xenon_vertex_gen_;
    delete shell_vertex_gen_;
    delete shell_endcap_vertex_gen_;
    delete msg_;
  }



  void NextPrecdr::Construct()
  {
    G4String name = "NEXT_PRECDR";

    // Define the copper shell (outer cylinder)
    G4double outer_radius = radius_ + shell_thickness_;
    G4double half_height = height_ / 2.;
    G4double cap_thickness = shell_thickness_;
    // Define the LAB volume (air box) containing everything
    G4double lab_size = 1.5 * (outer_radius + cap_thickness); // generous margin
    G4Box* lab_solid = new G4Box(name+"_LAB", lab_size, lab_size, lab_size);
    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    G4LogicalVolume* lab_logic = new G4LogicalVolume(lab_solid, air, name+"_LAB");
    // Set a visible color (transparent)
    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set the logical volume of the LAB as the main geometry volume
    GeometryBase::SetLogicalVolume(lab_logic);


    // Barrel (side shell)
    G4Tubs* shell_barrel = new G4Tubs(name+"_SHELL_BARREL", radius_, outer_radius, half_height, 0., twopi);
    // Endcaps (top and bottom)
    G4Tubs* shell_cap = new G4Tubs(name+"_SHELL_CAP", 0., outer_radius, cap_thickness/2., 0., twopi);

    // Union: barrel + top cap
    G4UnionSolid* shell_with_top = new G4UnionSolid(name+"_SHELL_TOP", shell_barrel, shell_cap, 0, G4ThreeVector(0., 0., half_height + cap_thickness/2.));
    // Union: (barrel+top) + bottom cap
    G4UnionSolid* shell_solid = new G4UnionSolid(name+"_SHELL", shell_with_top, shell_cap, 0, G4ThreeVector(0., 0., -half_height - cap_thickness/2.));

    G4Material* copper = G4NistManager::Instance()->FindOrBuildMaterial("G4_Cu");
    G4LogicalVolume* shell_logic = new G4LogicalVolume(shell_solid, copper, name+"_SHELL");
    // make it brown
    shell_logic->SetVisAttributes(nexus::CopperBrownAlpha());

    // Place the copper shell at the center of the LAB
    new G4PVPlacement(0, G4ThreeVector(), shell_logic, name+"_SHELL", lab_logic, false, 0, true);

    // Define the xenon volume (inner cylinder)
    G4Tubs* xenon_solid = new G4Tubs(name+"_XENON", 0., radius_, half_height, 0., twopi);
    G4Material* xenon = nullptr;
    if (liquid_)
      xenon = G4NistManager::Instance()->FindOrBuildMaterial("G4_lXe");
    else
      xenon = materials::GXe(pressure_);
    G4LogicalVolume* xenon_logic = new G4LogicalVolume(xenon_solid, xenon, name+"_XENON");
    // make it blue
    xenon_logic->SetVisAttributes(nexus::LightBlueAlpha());

    // Place the copper shell at the center of the LAB
    new G4PVPlacement(0, G4ThreeVector(), xenon_logic, name+"_XENON", lab_logic, false, 0, true);


    // Add a steel plate in the middle of the xenon volume
    G4Tubs* steel_plate_solid = new G4Tubs(name+"_STEEL_PLATE", 0., radius_, plate_thickness_/2., 0., twopi);
    G4Material* steel = materials::Steel();
    G4LogicalVolume* steel_plate_logic = new G4LogicalVolume(steel_plate_solid, steel, name+"_STEEL_PLATE");
    // Set a visible color (grey)
    steel_plate_logic->SetVisAttributes(G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 1.0)));
    // Place at z=0 (center)
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), steel_plate_logic, name+"_STEEL_PLATE", xenon_logic, false, 0, true);


    // Set the xenon logical volume as an ionization sensitive detector
    IonizationSD* ionizsd = new IonizationSD("/NEXT_PRECDR_XENON");
    G4SDManager::GetSDMpointer()->AddNewDetector(ionizsd);
    xenon_logic->SetSensitiveDetector(ionizsd);

    xenon_logic->SetUserLimits(new G4UserLimits(1 * mm));
  }



  G4ThreeVector NextPrecdr::GenerateVertex(const G4String& region) const
  {
    if (region == "COPPER") {
      // Compute volumes
      double barrel_vol = CLHEP::pi * (std::pow(radius_ + shell_thickness_, 2) - std::pow(radius_, 2)) * height_;
      double cap_vol = CLHEP::pi * std::pow(radius_ + shell_thickness_, 2) * shell_thickness_;
      double total_shell_vol = barrel_vol + 2 * cap_vol;
      double r = G4UniformRand();
      if (r < barrel_vol / total_shell_vol) {
        // Barrel
        return shell_vertex_gen_->GenerateVertex(VOLUME);
      } else {
        // Endcaps: randomly choose top or bottom
        G4ThreeVector v = shell_endcap_vertex_gen_->GenerateVertex(VOLUME);
        if (G4UniformRand() < 0.5)
          v.setZ(v.z() + height_/2. + shell_thickness_/2.); // top
        else
          v.setZ(v.z() - height_/2. - shell_thickness_/2.); // bottom
        return v;
      }
    } else if (region == "XENON") {
      return xenon_vertex_gen_->GenerateVertex(VOLUME);
    } else if (region == "SURFACE") {
      // Generate uniformly between barrel, endcaps of the copper shell surface, and cathode surface
      double barrel_area = 2 * CLHEP::pi * (radius_ + shell_thickness_/2.) * height_;
      double cap_area = 4 * CLHEP::pi * std::pow(radius_ + shell_thickness_/2., 2);
      double total_area = barrel_area + cap_area;
      double r = G4UniformRand();
      if (r < barrel_area / total_area) {
        // Barrel surface
        return shell_vertex_gen_->GenerateVertex(INNER_SURF);
      } else {
        G4ThreeVector v = surface_plate_vertex_gen_->GenerateVertex(VOLUME);
        G4double random_number = G4UniformRand();
        if (random_number < 0.25) {
          v.setZ(v.z() - height_/2 - shell_thickness_/2);
        } else if (random_number < 0.5) {
          v.setZ(v.z() + height_/2 + shell_thickness_/2);
        } else if (random_number < 0.75) {
          v.setZ(v.z() + plate_thickness_ /2);
        } else {
          v.setZ(v.z() - plate_thickness_ /2);
        }

        return v;

      }

    } else {
      return G4ThreeVector(0, 0, 0);
    }
  }


} // end namespace nexus
