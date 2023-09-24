// ----------------------------------------------------------------------------
// nexus | PmtR12860.h
//
// Geometry of the Hamamatsu R12860 photomultiplier.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef PMT_R12860_H
#define PMT_R12860_H

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

    /// Sets a sensitive detector associated to the

    void Construct();

  private:
    G4double pmt_diam_, pmt_length_; ///< PMT dimensions
  };

  inline G4double PmtR12860::Diameter() const { return pmt_diam_; }
  inline G4double PmtR12860::Length() const { return pmt_length_; }

} // end namespace nexus

#endif
