// ----------------------------------------------------------------------------
// petalosim | HumanPhantom.cc
//
// This class implements the geometry of a Jaszczak phantom, filled with water.source/geometries/HumanPhantom.cc
//
// The PETALO Collaboration
// ----------------------------------------------------------------------------

#include "HumanPhantom.h"

#include "FactoryBase.h"
#include "Visibilities.h"
#include "CylinderPointSampler2020.h"

#include <G4Tubs.hh>
#include <G4Sphere.hh>
#include <G4Cons.hh>
#include <G4Torus.hh>
#include <G4Box.hh>
#include <G4Ellipsoid.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4IntersectionSolid.hh>
#include <G4EllipticalTube.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4PVPlacement.hh>
#include <G4RotationMatrix.hh>
#include <G4VisAttributes.hh>
#include <G4GenericMessenger.hh>
#include <G4TransportationManager.hh>

using namespace nexus;

REGISTER_CLASS(HumanPhantom, GeometryBase)

HumanPhantom::HumanPhantom() : GeometryBase()
{
    geom_navigator_ =
    G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
    bone_activity_ = 0.062;
    brain_activity_ = 0.375;
    heart_activity_ = 0.146;
    bladder_activity_ = 0.146;
    pancreas_activity_ = 0.055;
    leg_activity_ = 0.031;
    kidney_activity_ = 0.066;
    intestine_activity_ = 0.041;
    lung_activity_ = 0.013;
    liver_activity_ = 0.074;
    spleen_activity_ = 0.064;
}

HumanPhantom::~HumanPhantom()
{
}

void HumanPhantom::Construct()
{

  G4Material *air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
  G4Material *water = G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER");

  G4Tubs *body = new G4Tubs("PHANTOM", 0, 0.21 * m, 0.95 * m, 0, twopi);
  G4LogicalVolume *body_logic = new G4LogicalVolume(body, air, "PHANTOM");
  body_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
  this->SetLogicalVolume(body_logic);
  G4VPhysicalVolume *body_phys = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), body_logic, "PHANTOM", 0, false, 0);
  cyl_gen_ = new CylinderPointSampler2020(body_phys);

  // MIRD male model
  // Ellipsoid
  G4double ax = 7.0 * cm;
  G4double by = 10.0 * cm;
  G4double cz = 8.50 * cm;
  G4double zcut1 = 0.0 * cm;
  G4double zcut2 = 8.5 * cm;

  auto *head1 = new G4Ellipsoid("Head1", ax, by, cz, zcut1, zcut2);

  G4double dx = 7.0 * cm;
  G4double dy = 10.0 * cm;
  G4double dz = 7.75 * cm;

  auto *head2 = new G4EllipticalTube("Head2", dx, dy, dz);

  auto *head = new G4UnionSolid("Head", head2, head1,
                                nullptr, // Rotation
                                G4ThreeVector(0. * cm, 0. * cm, 7.7500 * cm));

  G4Material *soft = G4NistManager::Instance()->FindOrBuildMaterial("G4_TISSUE_SOFT_ICRP");
  G4Material *brain_material = G4NistManager::Instance()->FindOrBuildMaterial("G4_BRAIN_ICRP");
  G4Material *skeleton = G4NistManager::Instance()->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU");
  G4Material *lung_material = G4NistManager::Instance()->FindOrBuildMaterial("G4_LUNG_ICRP");
  auto *logicHead = new G4LogicalVolume(head, soft, "logicalHead",
                                        nullptr, nullptr, nullptr);
  logicHead->SetVisAttributes(G4VisAttributes::GetInvisible());
  // Define rotation and position here!
  auto *rm = new G4RotationMatrix();
  rm->rotateX(180. * degree);
  rm->rotateY(180. * degree);

  new G4PVPlacement(rm,
                    G4ThreeVector(0. * cm, 0. * cm, 77.75 * cm),
                    logicHead,
                    "physicalHead",
                    body_logic,
                    false, 0, true);

  ax = 6. * cm;
  by = 9. * cm;
  cz = 6.5 * cm;

  auto *brain = new G4Ellipsoid("Brain", ax, by, cz);

  auto *logicBrain = new G4LogicalVolume(brain, brain_material,
                                         "logicalBrain",
                                         nullptr, nullptr, nullptr);

  // Define rotation and position here!
  new G4PVPlacement(nullptr,
                    G4ThreeVector(0. * cm, 0. * cm, 8.75 * cm),
                    logicBrain,
                    "brain",
                    logicHead,
                    false,
                    0, true);

  logicBrain->SetVisAttributes(nexus::YellowAlpha());

  // Outer cranium
  ax = 6.8 * cm; // a out skull
  by = 9.8 * cm; // bout
  cz = 8.3 * cm; // cout

  auto *craniumOut = new G4Ellipsoid("CraniumOut", ax, by, cz);

  ax = 6. * cm;  // a in
  by = 9. * cm;  // b in
  cz = 6.5 * cm; // cin

  auto *craniumIn = new G4Ellipsoid("CraniumIn", ax, by, cz);

  auto *cranium = new G4SubtractionSolid("Cranium",
                                         craniumOut,
                                         craniumIn, nullptr,
                                         G4ThreeVector(0.0, 0.0, 1. * cm));

  auto *logicSkull = new G4LogicalVolume(cranium, skeleton,
                                         "logicalSkull",
                                         nullptr, nullptr, nullptr);

  logicSkull->SetVisAttributes(nexus::WhiteAlpha());
  // Define rotation and position here!
  new G4PVPlacement(nullptr,
                    G4ThreeVector(0., 0., 7.75 * cm),
                    logicSkull,
                    "boneSkull",
                    logicHead,
                    false,
                    0, true);

  logicSkull->SetVisAttributes(nexus::WhiteAlpha());

  // TRUNK
  dx = 20. * cm;
  dy = 10. * cm;
  dz = 35. * cm;

  auto *trunk = new G4EllipticalTube("Trunk", dx, dy, dz);

  auto *logicTrunk = new G4LogicalVolume(trunk, soft,
                                         "logicalTrunk",
                                         nullptr, nullptr, nullptr);
  // Define rotation and position here!
  auto *rm2 = new G4RotationMatrix();
  rm2->rotateX(180. * degree);
  rm2->rotateY(180. * degree);
  new G4PVPlacement(rm2,
                    G4ThreeVector(0. * cm, 0. * cm, 35. * cm),
                    logicTrunk,
                    "trunk",
                    body_logic,
                    false,
                    0, true);
  // Visualization Attributes
  logicTrunk->SetVisAttributes(nexus::WhiteAlpha());

  dx = 2. * cm;
  dy = 2.5 * cm;
  dz = 4.25 * cm;

  auto *upperSpine = new G4EllipticalTube("UpperSpine", dx, dy, dz);

  G4double xx = 20. * cm;
  G4double yy = 10. * cm;
  G4double zz = 5. * cm;

  auto *subtraction = new G4Box("box", xx / 2., yy / 2., zz / 2.);

  auto *matrix = new G4RotationMatrix();
  matrix->rotateX(-25. * deg);

  auto *upper_spine = new G4SubtractionSolid("upperspine", upperSpine, subtraction,
                                             matrix, G4ThreeVector(0., -2.5 * cm, 5.5 * cm));

  auto *logicUpperSpine = new G4LogicalVolume(upper_spine, skeleton,
                                              "logicalUpperSpine",
                                              nullptr, nullptr, nullptr);

  logicUpperSpine->SetVisAttributes(nexus::WhiteAlpha());
  // Define rotation and position here!
  new G4PVPlacement(nullptr,
                    G4ThreeVector(0.0, 5.5 * cm, -3.5 * cm),
                    logicUpperSpine,
                    "boneUpperSpine",
                    logicHead,
                    false,
                    0, true);

  dx = 2. * cm;
  dy = 2.5 * cm;
  dz = 24. * cm;

  auto *middleLowerSpine = new G4EllipticalTube("MiddleLowerSpine", dx, dy, dz);

  auto *logicMiddleLowerSpine = new G4LogicalVolume(middleLowerSpine, skeleton,
                                                    "logicalMiddleLowerSpine",
                                                    nullptr, nullptr, nullptr);

  logicMiddleLowerSpine->SetVisAttributes(nexus::WhiteAlpha());
  // Define rotation and position here!
  new G4PVPlacement(nullptr, G4ThreeVector(0.0 * cm, 5.5 * cm, 11. * cm),
                    logicMiddleLowerSpine,
                    "boneLowerSpine",
                    logicTrunk,
                    false,
                    0, true);

  // PELVIS

  dx = 12. * cm; // a2
  dy = 12. * cm; // b2
  dz = 11. * cm; // z2/2

  G4VSolid *outPelvis = new G4EllipticalTube("OutPelvis", dx, dy, dz);

  dx = 11.3 * cm; // a1
  dy = 11.3 * cm; // b1
  dz = 12.0 * cm; // z2/2

  G4VSolid *inPelvis = new G4EllipticalTube("InPelvis", dx, dy, dz);

  G4double x = 28. * cm; // a2 * 2
  G4double y = 28. * cm; // b2*2
  G4double z = 24. * cm; // z2

  auto *subPelvis = new G4Box("SubtrPelvis", x / 2., y / 2., z / 2.);

  auto *firstPelvis = new G4SubtractionSolid("FirstPelvis",
                                             outPelvis,
                                             inPelvis, nullptr, G4ThreeVector(0. * cm, -0.8 * cm, 0. * cm));

  auto *secondPelvis = new G4SubtractionSolid("SecondPelvis",
                                              firstPelvis,
                                              subPelvis, nullptr,
                                              G4ThreeVector(0.0,
                                                            -14. * cm, 0. * cm));

  auto *pelvis = new G4SubtractionSolid("Pelvis", secondPelvis, subPelvis,
                                        nullptr,
                                        G4ThreeVector(0.0,
                                                      22. * cm, -9. * cm));

  auto *logicPelvis = new G4LogicalVolume(pelvis, skeleton,
                                          "logicalPelvis", nullptr, nullptr, nullptr);

  logicPelvis->SetVisAttributes(nexus::WhiteAlpha());

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, -3. * cm, -24. * cm), // 0, y02, z position
                                                                      // with respect to the trunk
                    logicPelvis,
                    "bonePelvis",
                    logicTrunk,
                    false,
                    0, true);

  // HEART

  ax = 4.00 * cm;
  by = 4.00 * cm;
  cz = 7.00 * cm;
  zcut1 = -7.00 * cm;
  zcut2 = 0.0 * cm;

  auto *heart1 = new G4Ellipsoid("Heart1", ax, by, cz, zcut1, zcut2);

  G4double rmin = 0. * cm;
  G4double rmax = 3.99 * cm;
  G4double startphi = 0. * degree;
  G4double deltaphi = 360. * degree;
  G4double starttheta = 0. * degree;
  G4double deltatheta = 90. * degree;

  auto *heart2 = new G4Sphere("Heart2", rmin, rmax,
                              startphi, deltaphi,
                              starttheta, deltatheta);

  auto *heart = new G4UnionSolid("Heart", heart1, heart2);

  auto *logicHeart = new G4LogicalVolume(heart, soft,
                                         "HeartVolume",
                                         nullptr, nullptr, nullptr);
  logicHeart->SetVisAttributes(nexus::BloodRedAlpha());
  // Define rotation and position here!
  rm = new G4RotationMatrix();
  rm->rotateY(25. * degree);
  new G4PVPlacement(rm, G4ThreeVector(0.0, -3.0 * cm, 15.32 * cm),
                    logicHeart,
                    "heart",
                    logicTrunk,
                    false,
                    0, true);

  // LUNGS

  ax = 5. * cm;  // a
  by = 7.5 * cm; // b
  cz = 24. * cm; // c
  zcut1 = 0.0 * cm;
  zcut2 = 24. * cm;

  auto *oneLung = new G4Ellipsoid("OneLung", ax, by, cz, zcut1, zcut2);

  ax = 5. * cm;
  by = 7.5 * cm;
  cz = 24. * cm;

  auto *subtrLung = new G4Ellipsoid("subtrLung", ax, by, cz);

  dx = 5.5 * cm;
  dy = 8.5 * cm;
  dz = 24. * cm;

  auto *box = new G4Box("Box", dx, dy, dz);

  auto *section2 = new G4SubtractionSolid("BoxSub2", subtrLung, box, nullptr, G4ThreeVector(0. * cm, 8.5 * cm, 0. * cm));

  auto *lung2 = new G4SubtractionSolid("Lung2", oneLung,
                                       section2,
                                       nullptr, G4ThreeVector(-6. * cm, 0 * cm, 0.0 * cm));

  auto *logicLeftLung = new G4LogicalVolume(lung2, lung_material,
                                            "logicalLeftLung", nullptr, nullptr, nullptr);
  logicLeftLung->SetVisAttributes(nexus::DirtyWhiteAlpha());

  new G4PVPlacement(nullptr, G4ThreeVector(8.50 * cm, 0.0 * cm, 8.5 * cm),
                    logicLeftLung,
                    "lungLeft",
                    logicTrunk,
                    false,
                    0, true);

  auto *section = new G4SubtractionSolid("BoxSub", subtrLung, box, nullptr, G4ThreeVector(0. * cm, 8.5 * cm, 0. * cm));

  auto *lung1 = new G4SubtractionSolid("Lung1", oneLung,
                                       section,
                                       nullptr, G4ThreeVector(6. * cm, 0 * cm, 0.0 * cm));

  auto *logicRightLung = new G4LogicalVolume(lung1, lung_material,
                                             "logicalRightLung", nullptr, nullptr, nullptr);
  logicRightLung->SetVisAttributes(nexus::DirtyWhiteAlpha());

  rm = new G4RotationMatrix();
  rm->rotateZ(-360. * degree);
  new G4PVPlacement(rm, G4ThreeVector(-8.50 * cm, 0.0 * cm, 8.5 * cm),
                    logicRightLung,
                    "lungRight",
                    logicTrunk,
                    false,
                    0, true);

  // STOMACH
  ax = 4. * cm;
  by = 3. * cm;
  cz = 8. * cm;

  auto *stomach_out = new G4Ellipsoid("stomach_out",
                                      ax, by, cz);

  auto *logicStomach = new G4LogicalVolume(stomach_out, soft,
                                           "logicalStomach", nullptr, nullptr, nullptr);
  logicStomach->SetVisAttributes(nexus::CopperBrownAlpha());
  new G4PVPlacement(nullptr, G4ThreeVector(8. * cm, -4. * cm, 0),
                    logicStomach,
                    "stomach",
                    logicTrunk,
                    false,
                    0, true);

  // RIBCAGE

  dx = 17. * cm;                  // a2
  dy = 9.8 * cm;                  // b2
  G4double thickness = 32.4 * cm; // z2/2 of cage

  auto *outCage = new G4EllipticalTube("outCage", dx, dy, thickness / 2.);

  dx = 16.4 * cm; // a1
  dy = 9.2 * cm;  // b1
  dz = 34. * cm;  // z2/2

  auto *inCage = new G4EllipticalTube("inCage", dx, dy, dz / 2.);

  auto *cage = new G4SubtractionSolid("Cage",
                                      outCage,
                                      inCage, nullptr, G4ThreeVector(0. * cm, 0. * cm, 0. * cm));

  auto *logicRibCage = new G4LogicalVolume(cage, soft, "logicalCage", nullptr, nullptr, nullptr);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, thickness / 2. + 0.1 * cm),
                    // with respect to the trunk
                    logicRibCage,
                    "boneRibCage",
                    logicTrunk,
                    false,
                    0, true);

  xx = 17. * cm;
  yy = 9.8 * cm;
  G4double ribThickness = 1.4 * cm;
  auto *rib_out = new G4EllipticalTube("rib_out", xx, yy, ribThickness / 2.);

  xx = 16.5 * cm;
  yy = 9.3 * cm;
  zz = 1.5 * cm;
  auto *rib_in = new G4EllipticalTube("rib_in", xx, yy, zz / 2.);
  auto *rib = new G4SubtractionSolid("rib", rib_out, rib_in);

  auto *logicRib = new G4LogicalVolume(rib, skeleton, "logicalRib", nullptr, nullptr, nullptr);

  // new G4PVPlacement(nullptr,G4ThreeVector(0.0, 0.0, (- 32.2*cm/2. + 0.8 *cm)),
  // 		       // with respect to the trunk
  // 		       "physicalRib",
  // 		       logicRib,
  // 		       physRibCage,
  // 		       false,
  // 		       0, true);

  // new G4PVPlacement(nullptr,G4ThreeVector(0.0, 0.0, ( - 32.2*cm/2. + 0.8 *cm + 2.8 *cm)),
  // 		       // with respect to the trunk
  // 		       "physicalRib",
  // 		       logicRib,
  // 		       physRibCage,
  // 		       false,
  // 		       0, true);

  // new G4PVPlacement(nullptr,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 * cm + 5.6 *cm)),
  // 		       // with respect to the trunk
  // 		       "physicalRib",
  // 		       logicRib,
  // 		       physRibCage,
  // 		       false,
  // 		       0, true);

  // new G4PVPlacement(nullptr,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 * cm + 8.4 *cm)),
  // 		       // with respect to the trunk
  // 		       "physicalRib",
  // 		       logicRib,
  // 		       physRibCage,
  // 		       false,
  // 		       0, true);

  // new G4PVPlacement(nullptr,G4ThreeVector(0.0, 0.0, (-thickness/2. + 0.8 * cm + 11.2 *cm)),
  // 		       // with respect to the trunk
  // 		       "physicalRib",
  // 		       logicRib,
  // 		       physRibCage,
  // 		       false,
  // 		       0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 14. * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 16.8 * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 19.6 * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 22.4 * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 25.2 * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 28. * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, (-thickness / 2. + 0.8 * cm + 30.8 * cm)),
                    // with respect to the trunk
                    logicRib,
                    "boneRib",
                    logicRibCage,
                    false,
                    0, true);

  // Visualization Attributes
  logicRibCage->SetVisAttributes(G4VisAttributes::GetInvisible());
  logicRib->SetVisAttributes(nexus::WhiteAlpha());

  // UPPER LARGE INTESTINE

  dx = 2.5 * cm;   // aU
  dy = 2.5 * cm;   // bU
  dz = 4.775 * cm; // dzU

  auto *AscendingColonUpperLargeIntestine = new G4EllipticalTube("AscendingColon", dx, dy, dz);

  dx = 2.5 * cm;  // bt
  dy = 1.5 * cm;  // ct
  dz = 10.5 * cm; // x1t

  auto *TraverseColonUpperLargeIntestine = new G4EllipticalTube("TraverseColon", dx, dy, dz);

  auto *relative_rm = new G4RotationMatrix();
  relative_rm->rotateX(90. * degree);
  relative_rm->rotateZ(0. * degree);
  relative_rm->rotateY(90. * degree);
  auto *upperLargeIntestine = new G4UnionSolid("UpperLargeIntestine",
                                               AscendingColonUpperLargeIntestine,
                                               TraverseColonUpperLargeIntestine,
                                               relative_rm,
                                               G4ThreeVector(8.0 * cm, 0.0, 6.275 * cm)); //,0,dzU + ct transverse

  auto *logicUpperLargeIntestine = new G4LogicalVolume(upperLargeIntestine, soft,
                                                       "logicalUpperLargeIntestine",
                                                       nullptr, nullptr, nullptr);

  logicUpperLargeIntestine->SetVisAttributes(nexus::CopperBrownAlpha());

  new G4PVPlacement(nullptr,
                    G4ThreeVector(-8.0 * cm, -2.36 * cm, -15.775 * cm),
                    logicUpperLargeIntestine,
                    "intestineUpperLarge", // xo, yo, zo ascending colon
                    logicTrunk,
                    false,
                    0, true);

  // LOWER LARGE INTESTINE

  dx = 1.88 * cm; // a
  dy = 2.13 * cm; // b
  dz = 7.64 * cm; //(z1-z2)/2

  auto *DescendingColonLowerLargeIntestine = new G4EllipticalTube("DiscendingColon", dx, dy, dz);

  rmin = 0.0 * cm;
  rmax = 1.88 * cm;          // a
  G4double rtor = 5.72 * cm; // R1
  startphi = 0. * degree;
  deltaphi = 90. * degree;

  auto *SigmoidColonUpLowerLargeIntestine = new G4Torus("SigmoidColonUpLowerLargeIntestine",
                                                        rmin, rmax, rtor,
                                                        startphi, deltaphi);

  rtor = 3. * cm; // R2
  G4VSolid *SigmoidColonDownLowerLargeIntestine = new G4Torus("SigmoidColonDownLowerLargeIntestine",
                                                              rmin, rmax,
                                                              rtor, startphi, deltaphi);

  relative_rm = new G4RotationMatrix();
  relative_rm->rotateY(180. * degree);
  relative_rm->rotateZ(90. * degree);

  auto *SigmoidColonLowerLargeIntestine = new G4UnionSolid("SigmoidColonLowerLargeIntestine",
                                                           SigmoidColonUpLowerLargeIntestine,
                                                           SigmoidColonDownLowerLargeIntestine,
                                                           relative_rm,
                                                           G4ThreeVector(0.0, 8.72 * cm, 0.0));
  // R1 + R2

  auto *relative_rm_2 = new G4RotationMatrix();
  relative_rm_2->rotateX(90. * degree);

  auto *LowerLargeIntestine = new G4UnionSolid("LowerLargeIntestine",
                                               DescendingColonLowerLargeIntestine,
                                               SigmoidColonLowerLargeIntestine,
                                               relative_rm_2,
                                               G4ThreeVector(-5.72 * cm, 0.0 * cm, -7.64 * cm)); // -rtor,0, -dz

  auto *logicLowerLargeIntestine = new G4LogicalVolume(LowerLargeIntestine, soft,
                                                       "logicalLowerLargeIntestine",
                                                       nullptr, nullptr, nullptr);

  logicLowerLargeIntestine->SetVisAttributes(nexus::CopperBrownAlpha());

  new G4PVPlacement(nullptr, // R1+ R2, -2.36 (y0), z0
                    G4ThreeVector(8.72 * cm, -2.36 * cm, -18.64 * cm),
                    logicLowerLargeIntestine,
                    "intestineLowerLarge",
                    logicTrunk,
                    false,
                    0, true);

  // SMALL INTESTINE

  G4double boxX = 11. * cm;
  G4double boxY = 3.53 * cm;
  G4double boxZ = 5 * cm;

  auto *smallIntestineBox = new G4Box("smallIntestineBox", boxX, boxY, boxZ);

  G4double tubsRmin = 0 * cm;
  G4double tubsRmax = 11. * cm;
  G4double tubsZ = 5 * cm;
  G4double tubsSphi = 0 * degree;
  G4double tubsDphi = 360 * degree;

  auto *smallIntestineTubs = new G4Tubs("smallIntestineTubs", tubsRmin, tubsRmax, tubsZ, tubsSphi, tubsDphi);

  // G4IntersectionSolid* SmallIntestine = new G4IntersectionSolid("SmallIntestine",smallIntestineTubs,smallIntestineBox,
  auto *filledSmallIntestine1 = new G4IntersectionSolid("filledSmallIntestine1", smallIntestineTubs, smallIntestineBox,
                                                        nullptr, G4ThreeVector(0 * cm, -1.33 * cm, 0 * cm));

  auto *filledSmallIntestine = new G4IntersectionSolid("filledSmallIntestine", filledSmallIntestine1, smallIntestineTubs,
                                                       nullptr, G4ThreeVector(0 * cm, 0.8 * cm, 0 * cm));

  relative_rm = new G4RotationMatrix();
  relative_rm->rotateX(90. * degree);
  // relative_rm -> rotateZ(180. * degree);
  relative_rm->rotateY(90. * degree);
  upperLargeIntestine = new G4UnionSolid("UpperLargeIntestine",
                                         AscendingColonUpperLargeIntestine,
                                         TraverseColonUpperLargeIntestine,
                                         relative_rm,
                                         G4ThreeVector(-8.0 * cm, 0.0 * cm, 6.275 * cm)); //,0,dzU + ct transverse

  auto *upperlowerLargeIntestine = new G4UnionSolid("UpperLowerLargeIntestine",
                                                    upperLargeIntestine,
                                                    DescendingColonLowerLargeIntestine,
                                                    nullptr,
                                                    G4ThreeVector(-16.72 * cm, 0.0 * cm, -2.865 * cm)); //,0,dzU + ct t

  auto *SmallIntestine = new G4SubtractionSolid("SmallIntestine",
                                                filledSmallIntestine,
                                                upperlowerLargeIntestine,
                                                nullptr,
                                                G4ThreeVector(8.0 * cm, -0.3 * cm, -2.775 * cm));

  auto *logicSmallIntestine = new G4LogicalVolume(SmallIntestine,
                                                  soft,
                                                  "logicalSmallIntestine",
                                                  nullptr, nullptr, nullptr);
  rm = new G4RotationMatrix();
  rm->rotateX(180. * degree);
  rm->rotateY(180. * degree);
  new G4PVPlacement(rm,
                    G4ThreeVector(0 * cm, -2.66 * cm, -13 * cm), // Xcheck the spina position the correct placement shuod be this one
                                                                 // G4ThreeVector(0*cm, -5.13*cm, -13*cm), // Xcheck the spina position the correct placement shuod be this one
                    // G4ThreeVector(0*cm, -6*cm, -13*cm),
                    logicSmallIntestine,
                    "intestineSmall",
                    logicTrunk,
                    false,
                    0, true);

  logicSmallIntestine->SetVisAttributes(nexus::CopperBrownAlpha());

  // LIVER

  dx = 14.19 * cm; // a
  dy = 7.84 * cm;  // b
  dz = 7.21 * cm;  //(z2-z1)/2

  auto *firstLiver = new G4EllipticalTube("FirstLiver", dx, dy, dz);

  xx = 20.00 * cm;
  yy = 50.00 * cm;
  zz = 50.00 * cm;

  auto *subtrLiver = new G4Box("SubtrLiver", xx / 2., yy / 2., zz / 2.);

  auto *rm_relative = new G4RotationMatrix();
  rm_relative->rotateY(32. * degree);
  rm_relative->rotateZ(40.9 * degree);

  auto *liver = new G4SubtractionSolid("Liver",
                                       firstLiver, subtrLiver,
                                       rm_relative,
                                       G4ThreeVector(10.0 * cm, 0.0 * cm, 0.0 * cm));

  auto *logicLiver = new G4LogicalVolume(liver,
                                         soft,
                                         "LiverVolume",
                                         nullptr, nullptr, nullptr);
  logicLiver->SetVisAttributes(nexus::BrownAlpha());
  // Define rotation and position here!
  rm = new G4RotationMatrix();
  rm->rotateX(180. * degree);
  new G4PVPlacement(rm, G4ThreeVector(0. * cm, 0. * cm, 0. * cm),
                    logicLiver,
                    "liverPhysical",
                    logicTrunk,
                    false,
                    0, true);

  // PANCREAS

  ax = 3. * cm;      // c
  by = 1. * cm;      // b
  cz = 15. * cm;     // a
  zcut1 = -15. * cm; // -a
  zcut2 = 0.0 * cm;

  auto *pancreasFirst = new G4Ellipsoid("PancreasFirst", ax, by, cz,
                                        zcut1, zcut2);

  xx = 6. * cm;  // 2*c
  yy = 2. * cm;  // 2*b
  zz = 12. * cm; // cz - x1 = 3 cm
  auto *subtrPancreas = new G4Box("SubtrPancreas", xx / 2., yy / 2., zz / 2.);

  auto *pancreas = new G4SubtractionSolid("pancreas",
                                          pancreasFirst,
                                          subtrPancreas,
                                          nullptr,
                                          G4ThreeVector(-3 * cm, 0.0, -9. * cm));

  auto *logicPancreas = new G4LogicalVolume(pancreas, soft,
                                            "logicalPancreas",
                                            nullptr, nullptr, nullptr);
  logicPancreas->SetVisAttributes(nexus::YellowAlpha());
  rm = new G4RotationMatrix();
  rm->rotateY(90. * degree);
  new G4PVPlacement(rm,
                    G4ThreeVector(-0. * cm, 0.0, 2 * cm), // x0, 0, 2 cm
                    logicPancreas,
                    "pancreas",
                    logicTrunk,
                    false,
                    0, true);

  // BLADDER

  ax = 4.958 * cm;
  by = 3.458 * cm;
  cz = 3.458 * cm;

  auto *bladder = new G4Ellipsoid("bladder_out", ax, by, cz);

  ax = 4.706 * cm;
  by = 3.206 * cm;
  cz = 3.206 * cm;
  auto *inner = new G4Ellipsoid("innerBladder", ax, by, cz);

  auto *totalBladder = new G4SubtractionSolid("bladder", bladder, inner);

  auto *logicUrinaryBladder = new G4LogicalVolume(totalBladder, soft,
                                                  "logicalUrinaryBladder",
                                                  nullptr, nullptr, nullptr);

  auto *logicInnerBladder = new G4LogicalVolume(inner, water,
                                                "logicalInnerBladder",
                                                nullptr, nullptr, nullptr);

  logicUrinaryBladder->SetVisAttributes(nexus::DirtyWhiteAlpha());
  logicInnerBladder->SetVisAttributes(G4VisAttributes::GetInvisible());
  // Define rotation and position here!
  new G4PVPlacement(nullptr, G4ThreeVector(0 * cm, -4.5 * cm, -27. * cm),
                    logicUrinaryBladder,
                    "bladder",
                    logicTrunk,
                    false,
                    0, true);
  new G4PVPlacement(nullptr, G4ThreeVector(0 * cm, -4.5 * cm, -27. * cm),
                    logicInnerBladder,
                    "bladderInner",
                    logicTrunk,
                    false,
                    0, true);
  // RIGHT KIDNEY

  ax = 4.5 * cm; // a
  by = 1.5 * cm; // b
  cz = 5.5 * cm; // c

  auto *oneKidney = new G4Ellipsoid("OneKidney", ax, by, cz);

  xx = 6. * cm;
  yy = 12.00 * cm;
  zz = 12.00 * cm;
  G4VSolid *subtrKidney = new G4Box("SubtrKidney", xx / 2., yy / 2., zz / 2.);

  auto *kidney = new G4SubtractionSolid("Kidney",
                                        oneKidney,
                                        subtrKidney,
                                        nullptr,
                                        G4ThreeVector(6. * cm, // x0
                                                      0.0 * cm,
                                                      0.0 * cm));

  auto *logicKidney = new G4LogicalVolume(kidney,
                                          soft,
                                          "logicalKidney",
                                          nullptr, nullptr, nullptr);
  logicKidney->SetVisAttributes(nexus::CopperBrownAlpha());
  new G4PVPlacement(nullptr, G4ThreeVector(-6. * cm,    // xo
                                           6. * cm,     // yo
                                           -2.50 * cm), // zo
                    logicKidney,
                    "kidneyRight",
                    logicTrunk,
                    false,
                    0, true);

  rm = new G4RotationMatrix();
  rm->rotateY(180. * degree);
  new G4PVPlacement(rm, G4ThreeVector(6. * cm,     // xo
                                      6. * cm,     // yo
                                      -2.50 * cm), // zo
                    logicKidney,
                    "kidneyLeft",
                    logicTrunk,
                    false,
                    0, true);

  // SPLEEN

  ax = 3.5 * cm;
  by = 2. * cm;
  cz = 6. * cm;

  auto *spleen = new G4Ellipsoid("spleen", ax, by, cz);

  auto *logicSpleen = new G4LogicalVolume(spleen, soft,
                                          "logicalSpleen",
                                          nullptr, nullptr, nullptr);
  logicSpleen->SetVisAttributes(nexus::BrownAlpha());
  new G4PVPlacement(nullptr,
                    G4ThreeVector(11. * cm, 3. * cm, 2. * cm),
                    logicSpleen,
                    "spleen",
                    logicTrunk,
                    false,
                    0, true);

  // RIGHT LEG

  G4double rmin1 = 0. * cm;
  G4double rmin2 = 0. * cm;
  dz = 80.0 * cm;
  G4double rmax1 = 2.0 * cm;
  G4double rmax2 = 10. * cm;
  startphi = 0. * degree;
  deltaphi = 360. * degree;

  auto *leg1 = new G4Cons("Leg1",
                          rmin1, rmax1,
                          rmin2, rmax2, dz / 2.,
                          startphi, deltaphi);

  auto *logicRightLeg = new G4LogicalVolume(leg1,
                                            soft,
                                            "logicalRightLeg",
                                            nullptr, nullptr, nullptr);
  logicRightLeg->SetVisAttributes(nexus::WhiteAlpha());
  rm = new G4RotationMatrix();
  rm->rotateX(180. * degree);
  rm->rotateY(180. * degree);
  new G4PVPlacement(rm,
                    G4ThreeVector(-10. * cm, 0. * cm, -40. * cm),
                    logicRightLeg,
                    "legRight",
                    body_logic,
                    false,
                    0, true);
  new G4PVPlacement(rm,
                    G4ThreeVector(10. * cm, 0. * cm, -40. * cm),
                    logicRightLeg,
                    "legLeft",
                    body_logic,
                    false,
                    0, true);

  dz = 79.8 * cm;
  rmin1 = 0.0 * cm;
  rmin2 = 0.0 * cm;
  rmax1 = 1. * cm;
  rmax2 = 3.5 * cm;
  startphi = 0. * degree;
  deltaphi = 360. * degree;

  auto *leg_bone = new G4Cons("OneLeftLegBone",
                              rmin1, rmax1,
                              rmin2, rmax2, dz / 2.,
                              startphi, deltaphi);

  auto *logicLeftLegBone = new G4LogicalVolume(leg_bone, skeleton, "logicalLegBone",
                                               nullptr, nullptr, nullptr);
  logicLeftLegBone->SetVisAttributes(nexus::WhiteAlpha());
  // Define rotation and position here!
  new G4PVPlacement(nullptr,
                    G4ThreeVector(0.0 * cm, 0.0, 0.1 * cm),
                    logicLeftLegBone,
                    "boneLeftLeg",
                    logicRightLeg,
                    false,
                    0, true);

  // ARM

  dx = 1.4 * cm; // a
  dy = 2.7 * cm; // b
  // G4double dz= 46. * cm;//z0

  auto *leftArm = new G4EllipticalTube("OneLeftArmBone", dx, dy, 34.5 * cm);

  auto *logicLeftArmBone = new G4LogicalVolume(leftArm,
                                               skeleton,
                                               "logicalArm",
                                               nullptr, nullptr, nullptr);
  logicLeftArmBone->SetVisAttributes(nexus::WhiteAlpha());
  rm = new G4RotationMatrix();
  rm->rotateX(180. * degree);
  new G4PVPlacement(rm,
                    G4ThreeVector(18.4 * cm, 0.0, -0.5 * cm),
                    //-x0
                    logicLeftArmBone,
                    "boneLeftArm",
                    logicTrunk,
                    false, 0, true);
  new G4PVPlacement(rm,
                    G4ThreeVector(-18.4 * cm, 0.0, -0.5 * cm),
                    //-x0
                    logicLeftArmBone,
                    "boneRightArm",
                    logicTrunk,
                    false, 0, true);

  // SCAPULA

  G4double ax_in = 17. * cm;
  G4double by_in = 9.8 * cm;
  G4double ax_out = 19. * cm;
  G4double by_out = 9.8 * cm;
  dz = 16.4 * cm;

  auto *inner_scapula = new G4EllipticalTube("ScapulaIn", ax_in, by_in, (dz + 1. * cm) / 2);
  auto *outer_scapula = new G4EllipticalTube("ScapulaOut", ax_out, by_out, dz / 2);

  subtraction = new G4Box("subtraction", ax_out, ax_out, ax_out);

  xx = ax_out * 0.242; //(sin 14deg)
  yy = -ax_out * 0.97; // (cos 14 deg)

  rm = new G4RotationMatrix();
  rm->rotateZ(-14. * degree);

  auto *scapula_first = new G4SubtractionSolid("Scapula_first",
                                               outer_scapula,
                                               subtraction,
                                               rm,
                                               G4ThreeVector(xx, yy, 0. * cm));

  G4double xx2 = -ax_out * 0.62470; //(cos 51.34deg)
  G4double yy2 = ax_out * 0.78087;  // (sin 51.34 deg)

  rm2 = new G4RotationMatrix();
  rm2->rotateZ(-38.6598 * degree);

  auto *scapula_bone = new G4SubtractionSolid("Scapula",
                                              scapula_first,
                                              subtraction,
                                              rm2,
                                              G4ThreeVector(xx2, yy2, 0. * cm));

  auto *scapula = new G4SubtractionSolid("Scapula",
                                         scapula_bone,
                                         inner_scapula);

  auto *logicLeftScapula = new G4LogicalVolume(scapula,
                                               skeleton,
                                               "logicalLeftScapula",
                                               nullptr, nullptr, nullptr);

  logicLeftScapula->SetVisAttributes(nexus::WhiteAlpha());

  new G4PVPlacement(nullptr,
                    G4ThreeVector(0. * cm, 0. * cm, 24.1 * cm),
                    logicLeftScapula,
                    "boneLeftScapula",
                    logicTrunk,
                    false,
                    0, true);

  xx = -ax_out * 0.242; //(sin 14deg)
  yy = -ax_out * 0.97;  // (cos 14 deg)

  rm = new G4RotationMatrix();
  rm->rotateZ(14. * degree);

  scapula_first = new G4SubtractionSolid("Scapula_first",
                                         outer_scapula,
                                         subtraction,
                                         rm,
                                         G4ThreeVector(xx, yy, 0. * cm));

  xx2 = ax_out * 0.62470; //(cos 51.34deg)

  rm2 = new G4RotationMatrix();
  rm2->rotateZ(38.6598 * degree);

  scapula_bone = new G4SubtractionSolid("Scapula",
                                        scapula_first,
                                        subtraction,
                                        rm2,
                                        G4ThreeVector(xx2, yy2, 0. * cm));

  scapula = new G4SubtractionSolid("Scapula",
                                   scapula_bone,
                                   inner_scapula);

  auto *logicRightScapula = new G4LogicalVolume(scapula,
                                                skeleton,
                                                "logicalRightScapula",
                                                nullptr, nullptr, nullptr);

  logicRightScapula->SetVisAttributes(nexus::WhiteAlpha());

  new G4PVPlacement(nullptr,
                    G4ThreeVector(0. * cm, 0. * cm, 24.1 * cm),
                    logicRightScapula,
                    "boneRightScapula",
                    logicTrunk,
                    false,
                    0, true);

  // CLAVICLE

  G4double rMin = 0 * cm;
  G4double rMax = 0.7883 * cm;
  G4double rTor = 10 * cm;
  G4double pSPhi = 298.15 * degree;
  G4double pDPhi = 0.7 * rad;

  auto *clavicle = new G4Torus("Clavicle", rMin, rMax, rTor, pSPhi, pDPhi);

  auto *logicLeftClavicle = new G4LogicalVolume(clavicle,
                                                skeleton,
                                                "logicalLeftClavicle",
                                                nullptr, nullptr, nullptr);

  logicLeftClavicle->SetVisAttributes(nexus::WhiteAlpha());

  new G4PVPlacement(nullptr,
                    G4ThreeVector(0. * cm, 2. * cm, 33.25 * cm),
                    logicLeftClavicle,
                    "boneLeftClavicle",
                    logicTrunk,
                    false,
                    0, true);

  pSPhi = 201.75 * degree;

  clavicle = new G4Torus("Clavicle", rMin, rMax, rTor, pSPhi, pDPhi);

  auto *logicRightClavicle = new G4LogicalVolume(clavicle,
                                                 skeleton,
                                                 "logicalRightClavicle",
                                                 nullptr, nullptr, nullptr);

  logicRightClavicle->SetVisAttributes(nexus::WhiteAlpha());

  new G4PVPlacement(nullptr,
                    G4ThreeVector(0. * cm, 2. * cm, 33.25 * cm),
                    logicRightClavicle,
                    "boneRightClavicle",
                    logicTrunk,
                    false,
                    0, true);
}

G4ThreeVector HumanPhantom::GenerateVertex(const G4String & /*region*/) const
{
  G4ThreeVector vertex(0, 0, 0);
  G4VPhysicalVolume* VertexVolume;

  while (true)
  {
    vertex = cyl_gen_->GenerateVertex("VOLUME");
    VertexVolume = geom_navigator_->LocateGlobalPointAndSetup(vertex, 0, false);
    G4String vol_name = VertexVolume->GetName();
    // G4cout << vol_name << G4endl;

    if (G4StrUtil::starts_with(vol_name, "bone")) {
      if (((float)std::rand() / (float)RAND_MAX) < bone_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "heart")) {
      if (((float)std::rand() / (float)RAND_MAX) < heart_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "lung")) {
      if (((float)std::rand() / (float)RAND_MAX) < lung_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "liver")) {
      if (((float)std::rand() / (float)RAND_MAX) < liver_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "stomach")) {
      if (((float)std::rand() / (float)RAND_MAX) < stomach_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "intestine")) {
      if (((float)std::rand() / (float)RAND_MAX) < intestine_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "bladder")) {
      if (((float)std::rand() / (float)RAND_MAX) < bladder_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "kidney")) {
      if (((float)std::rand() / (float)RAND_MAX) < kidney_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "spleen")) {
      if (((float)std::rand() / (float)RAND_MAX) < spleen_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "pancreas")) {
      if (((float)std::rand() / (float)RAND_MAX) < pancreas_activity_) return vertex;
    } else if (G4StrUtil::starts_with(vol_name, "brain")) {
      if (((float)std::rand() / (float)RAND_MAX) < brain_activity_) return vertex;
    }
  }
}
