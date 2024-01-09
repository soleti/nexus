// ----------------------------------------------------------------------------
// nexus | PmtR12860.cc
//
// Geometry of the Hamamatsu R12860 photomultiplier.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "PmtR12860.h"

#include "SensorSD.h"
#include "OpticalMaterialProperties.h"
#include "MaterialsList.h"
#include "Visibilities.h"
// #include "ellipse_intersect_circle.hh"

#include <G4NistManager.hh>
#include <G4Polycone.hh>

#include <G4Tubs.hh>
#include <G4Ellipsoid.hh>
#include <G4UnionSolid.hh>
#include <G4Sphere.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4VisAttributes.hh>
#include <G4SDManager.hh>

#include <CLHEP/Units/SystemOfUnits.h>

namespace nexus {

  using namespace CLHEP;

  PmtR12860::PmtR12860(): GeometryBase()
  {
        m1_h = 190.;
    m1_r = 254.;

    m2_h = 5.; // -5 -> 0;
    m2_r = m1_r;

    m3_h = 190.; // ?
    m3_r = 254.;

    //               R_1
    //         ._______________
    //       .    .            |
    //     . theta .           |
    //   ._________.___________|
    //     Rtorus    R_2

    m4_torus_r = 80.; // ?
    m4_torus_angle = 45.*deg; // ?

    m4_r_2 = 254./2;
    m4_r_1 = (m4_r_2+m4_torus_r) - m4_torus_r*cos(m4_torus_angle);

    m4_h = m4_torus_r*sin(m4_torus_angle) + 5.0*mm;

    m5_r = m4_r_2;
    m5_h = 65.;
    m6_r = m5_r;
    m6_h = 190./2;
    m7_r = 75./2; // omit the torus for simplicity

    m8_r = 75./2;

    // (x/m6_r)^2 + (y/m6_h)^2 = 1
    // x = m8_r = 75/2
    // -> y = sqrt( (254^2-75^2)/254^2 * 190^2/4 )
    // >>> ( (254.**2-75.**2)/254.**2 * 190.**2/4 ) ** 0.5
    // 90.764151727223648
    // m8_h = 145-y = 55
    m8_h = 55.+15.;

    m9_r = 51.50/2;
    m9_h = 30.;
  }



  PmtR12860::~PmtR12860()
  {
  }


G4Polycone* PmtR12860::construct_polycone_neck(G4String solidname, double P_I_R, double P_I_H, double thickness )
  {
      G4Polycone* solid_IV = nullptr ;

      // "absolute" frame ellipse params
      // double e_cx = 0. ;
      // double e_cy = -m2_h ;   // lower ellipsoid half is pushed down a little : -5
      // double e_ax = P_I_R ;
      // double e_ay = P_I_H ;

      // "absolute" frame torus circle params
      // double c_cx = m4_torus_r + m4_r_2 ;      // 207.  torus_x
      double neck_offset_z = -210. + m4_h/2 ;  // see _1_4 below
      // double c_cy = neck_offset_z -m4_h/2 ;    // -210. torus_z  (see _1_4 below)
      // double c_r = m4_torus_r-thickness ;      //       torus_r

      // int n = 1000000 ;
      // bool verbose = false ;
      // printf("[Ellipse_Intersect_Circle \n");
      // Ellipse_Intersect_Circle ec = Ellipse_Intersect_Circle::make( e_cx, e_cy, e_ax, e_ay, c_cx, c_cy, c_r, n, verbose );
      // printf("]Ellipse_Intersect_Circle (%10.4f, %10.4f) \n", ec.intersect.p[0].x, ec.intersect.p[0].y );
      // intersect coordinates in  "absolute" relative to top Union 1_9 frame

      double ec_r =  144.3135 ;
      double ec_z = -163.5849 - neck_offset_z ;  // get back into neck frame

          G4double phiStart = 0.00*deg ;
          G4double phiTotal = 360.00*deg ;
          G4int numZPlanes = 2 ;
          G4double zPlane[] = {  -m4_h/2         , ec_z  } ;
          G4double rInner[] = {  0.0             , 0.0   } ;
          G4double rOuter[] = {  m5_r + thickness, ec_r  } ;

          // m4_r_2 == m5_r :
          // m5_r + thickness : matches radius of the tubs beneath
          G4cout << "RINNER " << rOuter[0] << " " << rOuter[1] << G4endl;
          G4Polycone* _solid_IV = new G4Polycone(
                                  solidname+"_IV",
                                  phiStart,
                                  phiTotal,
                                  numZPlanes,
                                  zPlane,
                                  rInner,
                                  rOuter
                                  );


      return solid_IV ;
  }


  void PmtR12860::Construct()
  {
    // PMT BODY //////////////////////////////////////////////////////

    G4UnionSolid *pmt_solid = NULL;
    G4UnionSolid *pmt_solid2 = NULL;
    G4double thickness = 0 *cm;
    double P_I_R = m1_r + thickness;
    double P_I_H = m1_h + thickness;

    G4String solidname = "PMTR12860";

    G4Ellipsoid *solid_I = new G4Ellipsoid(
        solidname + "_I",
        P_I_R,
        P_I_R,
        P_I_H,
        0,    // pzBottomCut -> equator
        P_I_H // pzTopCut -> top
    );

    G4Tubs *solid_II = new G4Tubs(
        solidname + "_II",
        0.0,
        P_I_R,
        m2_h / 2,
        0. * deg,
        360. * deg);
    // I+II

    pmt_solid = new G4UnionSolid(
        solidname + "_1_2",
        solid_I,  // upper hemi-ellipsoid
        solid_II, // thin equatorial cylinder, pushed down in z, top at z=0
        0,
        G4ThreeVector(0, 0, -m2_h / 2));

    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    G4Ellipsoid *solid_III = new G4Ellipsoid(
        solidname + "_III",
        P_I_R,
        P_I_R,
        P_I_H,
        -P_I_H,
        0);
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    // +III
    pmt_solid = new G4UnionSolid(
        solidname + "_1_3",
        pmt_solid,
        solid_III,
        0,
        G4ThreeVector(0, 0, -m2_h));
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    G4Polycone *solid_IV = nullptr;
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    double neck_offset_z = -210. + m4_h/2 ;  // see _1_4 below
    double ec_r =  144.3135 ;
    double ec_z = -163.5849 - neck_offset_z ;  // get back into neck frame

    G4double phiStart = 0.00*deg ;
    G4double phiTotal = 360.00*deg ;
    G4int numZPlanes = 2 ;
    G4double zPlane[] = {  -m4_h/2         , ec_z  } ;
    G4double rInner[] = {  0.0             , 0.0   } ;
    G4double rOuter[] = {  m5_r + thickness, ec_r  } ;

    // m4_r_2 == m5_r :
    // m5_r + thickness : matches radius of the tubs beneath
    G4cout << "RINNER " << rOuter[0] << " " << rOuter[1] << G4endl;
    solid_IV = new G4Polycone(
                            solidname+"_IV",
                            phiStart,
                            phiTotal,
                            numZPlanes,
                            zPlane,
                            rInner,
                            rOuter
                            );

    // solid_IV = construct_polycone_neck(solidname, P_I_R, P_I_H, thickness);
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    // +IV
    pmt_solid = new G4UnionSolid(
        solidname + "_1_4",
        pmt_solid,
        solid_IV,
        0,
        G4ThreeVector(0, 0, -210. * mm + m4_h / 2));
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    G4Tubs *solid_V = new G4Tubs(
        solidname + "_V",
        0.0,
        m5_r + thickness,
        m5_h / 2,
        0.0 * deg,
        360.0 * deg);
    // +V
    pmt_solid = new G4UnionSolid(
        solidname + "_1_5",
        pmt_solid,
        solid_V,
        0,
        G4ThreeVector(0, 0, -210. * mm - m5_h / 2));
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    double P_VI_R = m6_r + thickness;
    double P_VI_H = 95. * mm + thickness;
    G4Ellipsoid *solid_VI = new G4Ellipsoid(
        solidname + "_VI",
        P_VI_R,
        P_VI_R,
        P_VI_H,
        -90. * mm,
        0);
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    // // +VI
    pmt_solid = new G4UnionSolid(
        solidname + "_1_6",
        pmt_solid,
        solid_VI,
        0,
        G4ThreeVector(0, 0, -275. * mm));

    G4cout << "M8_H " << m8_h << G4endl;

    G4Tubs *solid_VIII = new G4Tubs(
        solidname + "_VIII",
        0.0,
        m8_r + thickness,
        m8_h / 2,
        0.0 * deg,
        360.0 * deg);
    // +VIII
    pmt_solid = new G4UnionSolid(
        solidname + "_1_8",
        pmt_solid,
        solid_VIII,
        0,
        G4ThreeVector(0, 0, -420. * mm + m8_h / 2));

    double *r_IX_in = new double[2];
    r_IX_in[0] = 0.0;
    r_IX_in[1] = 0.0;
    double *r_IX = new double[2];
    r_IX[0] = m9_r + thickness;
    r_IX[1] = m9_r + thickness;
    double *z_IX = new double[2];
    z_IX[0] = -(m9_h + thickness);
    z_IX[1] = 0;

    G4Polycone *solid_IX = new G4Polycone(
        solidname + "_IX",
        0.0 * deg,
        360. * deg,
        2,
        z_IX,
        r_IX_in,
        r_IX);
    G4cout << __FILE__ << ":" << __LINE__ << G4endl;

    // +VIII
    pmt_solid = new G4UnionSolid(
        solidname + "_1_9",
        pmt_solid,
        solid_IX,
        0,
        G4ThreeVector(0, 0, -420. * mm));

    pmt_diam_   = 508 * mm;
    pmt_length_ = 66 * mm;

    // G4Tubs* pmt_solid =
    //   new G4Tubs("PMT_R12860", 0., pmt_diam_/2., pmt_length_/2., 0., twopi);

    G4Material* aluminum =
      G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");

    G4LogicalVolume* pmt_logic =
      new G4LogicalVolume(pmt_solid, aluminum, "PMT_R12860");

    this->SetLogicalVolume(pmt_logic);


    // PMT WINDOW ////////////////////////////////////////////////////

    G4double window_diam = pmt_diam_;
    G4double window_length = 6. * mm;

    G4Tubs* window_solid =
      new G4Tubs("PMT_WINDOW", 0., window_diam/2., window_length/2., 0., twopi);

    G4Material* quartz =
      G4NistManager::Instance()->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
    quartz->SetMaterialPropertiesTable(opticalprops::FusedSilica());

    G4LogicalVolume* window_logic =
      new G4LogicalVolume(window_solid, quartz, "PMT_WINDOW");

    new G4PVPlacement(0, G4ThreeVector(0., 0., (pmt_length_-window_length)/2.),
		      window_logic, "PMT_WINDOW", pmt_logic, false, 0, false);

    G4VisAttributes wndw_col = nexus::WhiteAlpha();
    window_logic->SetVisAttributes(wndw_col);


    // PHOTOCATHODE //////////////////////////////////////////////////

    G4double phcath_diam   = 460 * mm;
    G4double phcath_height =  4.0 * mm;
    G4double phcath_thickn =  0.1 * mm;
    G4double phcath_posz   =-16.0 * mm;

    G4Tubs* phcath_solid =
      new G4Tubs("PHOTOCATHODE", 0, phcath_diam / 2., phcath_height / 2., 0, twopi);

    G4LogicalVolume* phcath_logic =
      new G4LogicalVolume(phcath_solid, aluminum, "PHOTOCATHODE");

    G4VisAttributes vis_solid;
    vis_solid.SetForceSolid(true);
    phcath_logic->SetVisAttributes(vis_solid);

    //G4PVPlacement* phcath_physi =
      new G4PVPlacement(0, G4ThreeVector(0.,0.,phcath_posz), phcath_logic,
			"PHOTOCATHODE", window_logic, false, 0, false);

    // Sensitive detector
    SensorSD* pmtsd = new SensorSD("/PMT_R12860/Pmt");
    pmtsd->SetDetectorVolumeDepth(2);
    pmtsd->SetTimeBinning(100.*nanosecond);
    G4SDManager::GetSDMpointer()->AddNewDetector(pmtsd);
    phcath_logic->SetSensitiveDetector(pmtsd);

    // OPTICAL SURFACES //////////////////////////////////////////////

    // The values for the efficiency are chosen in order to match
    // the curve of the quantum efficiency provided by Hamamatsu:
    // http://sales.hamamatsu.com/en/products/electron-tube-division/detectors/photomultiplier-tubes/part-R12860.php
    // The source of light is point-like, isotropic and it has been placed at a
    // distance of 25 cm from the surface of the PMT window.
    // The quantity to be compared with the Hamamatsu curve is:
    // number of detected photons/ number of photons that reach the PMT window.
    // The total number of generated photons is taken as
    // number of photons that reach the PMT window because
    // light is generated only in that fraction of solid angle that subtends the
    // window of the PMT.

    const G4int entries = 30;

    G4double ENERGIES[entries] =
      {1.72194*eV, 1.77114*eV, 1.82324*eV, 1.87848*eV, 1.93719*eV,
       1.99968*eV,  2.06633*eV, 2.13759*eV, 2.21393*eV, 2.29593*eV,
       2.38423*eV, 2.47960*eV, 2.58292*eV, 2.69522*eV, 2.81773*eV,
       2.95190*eV, 3.0995*eV, 3.26263*eV, 3.44389*eV, 3.64647*eV,
       3.87438*eV, 4.13267*eV, 4.42786*eV, 4.76846*eV, 5.16583*eV,
       5.63545*eV, 6.19900*eV, 6.88778*eV, 7.74875*eV, 8.85571*eV};
    G4double EFFICIENCY[entries] =
      { 1, 1, 1, 1, 1,
      	1, 1, 1, 1, 1,
    	  1, 1, 1, 1, 1,
    	  1, 1, 1, 1, 1,
    	  1, 1, 1, 1, 1,
        1, 1, 1, 1, 1};
      // { 0.00000, 0.00028, 0.00100, 0.00500, 0.00100,
    	// 0.02200, 0.04500, 0.07000, 0.11500, 0.16000,
    	// 0.20500, 0.23500, 0.27000, 0.29000, 0.31300,
    	// 0.35200, 0.38000, 0.38000, 0.37300, 0.37300,
    	// 0.37000, 0.36000, 0.35500, 0.33500, 0.31000,
    	// 0.29500, 0.27500, 0.23000, 0.52000, 0.00000};

    G4double REFLECTIVITY[entries] =
      { 0., 0., 0., 0., 0.,
	0., 0., 0., 0., 0.,
	0., 0., 0., 0., 0.,
	0., 0., 0., 0., 0.,
	0., 0., 0., 0., 0.,
	0., 0., 0., 0., 0. };

    G4MaterialPropertiesTable* phcath_mpt = new G4MaterialPropertiesTable();
    phcath_mpt->AddProperty("EFFICIENCY", ENERGIES, EFFICIENCY, entries);
    phcath_mpt->AddProperty("REFLECTIVITY", ENERGIES, REFLECTIVITY, entries);

    G4OpticalSurface* phcath_opsur =
      new G4OpticalSurface("PHOTOCATHODE", unified, polished, dielectric_metal);
    phcath_opsur->SetMaterialPropertiesTable(phcath_mpt);

    new G4LogicalSkinSurface("PHOTOCATHODE", phcath_logic, phcath_opsur);
  }


} // end namespace nexus
