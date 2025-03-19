// ----------------------------------------------------------------------------
// nexus | KZenChimney.h
//
// KZen Chimney.
//
// KamLAND-Zen geometry of the chimney.
// ----------------------------------------------------------------------------

#ifndef KZEN_CHIMNEY_H
#define KZEN_CHIMNEY_H

#include "GeometryBase.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class SpherePointSampler; }


namespace nexus {

  /// Spherical chamber filled with xenon (liquid or gas)

  class KZenChimney: public GeometryBase
  {
  public:
    /// Constructor
    KZenChimney();
    /// Destructor
    ~KZenChimney();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& /*region*/) const;

    void Construct();

  private:
    G4double chimney_radius_;   ///< Radius of the chimney
    G4double chimney_thickness_; ///< Thickness of the chimney wall
    G4double chimney_height_;    ///< Height of the chimney

    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
