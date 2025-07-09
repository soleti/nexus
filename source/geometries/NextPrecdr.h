// ----------------------------------------------------------------------------
// nexus | NextPrecdr.h
//
// Cylinder filled with xenon, surrounded by a copper shell.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef NEXT_PRECDR_H
#define NEXT_PRECDR_H

#include "GeometryBase.h"

class G4Material;
class G4GenericMessenger;
namespace nexus { class CylinderPointSampler; }

namespace nexus {

  /// Cylindrical chamber filled with xenon (liquid or gas), surrounded by copper shell

  class NextPrecdr: public GeometryBase
  {
  public:
    /// Constructor
    NextPrecdr();
    /// Destructor
    ~NextPrecdr();

    /// Return vertex within region <region> of the chamber
    G4ThreeVector GenerateVertex(const G4String& /*region*/) const;

    void Construct();

  private:
    G4bool liquid_;     ///< Whether xenon is liquid or not
    G4double pressure_; ///< Pressure (if gaseous state was selected)
    G4double radius_;   ///< Inner radius of the xenon cylinder (2m)
    G4double height_;   ///< Height of the xenon cylinder (4m)
    G4double shell_thickness_; ///< Thickness of copper shell (4cm)

    /// Vertexes random generator for xenon and shell
    CylinderPointSampler* xenon_vertex_gen_;
    CylinderPointSampler* shell_vertex_gen_;
    CylinderPointSampler* shell_endcap_vertex_gen_;

    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
