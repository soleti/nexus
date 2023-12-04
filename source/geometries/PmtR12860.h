// ----------------------------------------------------------------------------
// nexus | PmtR12860.h
//
// Geometry of the Hamamatsu R12860 photomultiplier.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef PMT_R12860_H
#define PMT_R12860_H
#include <G4Polycone.hh>

#include "GeometryBase.h"

#include <G4ThreeVector.hh>


namespace nexus {

  /// Geometry model for the Hamamatsu R12860 photomultiplier (PMT).

  class PmtR12860: public GeometryBase
  {
  public:
    /// Constructor
    PmtR12860();
    /// Destructor
    ~PmtR12860();

    /// Returns the PMT diameter
    G4double Diameter() const;
    /// Returns the PMT length
    G4double Length() const;

    G4Polycone* construct_polycone_neck(G4String solidname, double P_I_R, double P_I_H, double thickness );

    /// Sets a sensitive detector associated to the

    void Construct();

  private:
      double m1_h;
    double m1_r;
    double m2_h;
    double m2_r;
    double m3_h;
    double m3_r;
    double m4_torus_r;
    double m4_torus_angle;
    double m4_r_1;
    double m4_r_2;
    double m4_h;
    double m5_r;
    double m5_h;
    double m6_r;
    double m6_h;
    double m7_r;
    double m8_r;
    double m8_h;
    double m9_r;
    double m9_h;
    G4double pmt_diam_, pmt_length_; ///< PMT dimensions
  };

  inline G4double PmtR12860::Diameter() const { return pmt_diam_; }
  inline G4double PmtR12860::Length() const { return pmt_length_; }

} // end namespace nexus

#endif
