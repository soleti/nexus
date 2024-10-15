// ----------------------------------------------------------------------------
// petalosim | HumanPhantom.h
//
// This class implements the geometry of a NEMA Sensitivity phantom.
//
// The CRYSP Collaboration
// ----------------------------------------------------------------------------

#ifndef HUMAN_PHANTOM_H
#define HUMAN_PHANTOM_H

#include "GeometryBase.h"

class G4Material;
class G4GenericMessenger;
class G4Navigator;

namespace nexus
{
  class CylinderPointSampler2020;
}

using namespace nexus;

class HumanPhantom: public GeometryBase
{

 public:
  HumanPhantom();
  ~HumanPhantom();

  void Construct();

  G4ThreeVector GenerateVertex(const G4String &/*region*/) const;

 private:
  G4Navigator* geom_navigator_;
  G4double bone_activity_;
  G4double brain_activity_;
  G4double intestine_activity_;
  G4double liver_activity_;
  G4double lung_activity_;
  G4double stomach_activity_;
  G4double spleen_activity_;
  G4double kidney_activity_;
  G4double bladder_activity_;
  G4double leg_activity_;
  G4double pancreas_activity_;
  G4double heart_activity_;
  CylinderPointSampler2020* cyl_gen_;
};

#endif
