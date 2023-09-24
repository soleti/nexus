// ----------------------------------------------------------------------------
// nexus | Fiber_barrel_meth.cc
//
// Cylinder containing optical fibers with a methacrylate window
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------


#ifndef FIBER_BARREL_METH_H
#define FIBER_BARREL_METH_H

#include "GeometryBase.h"
#include "GenericWLSFiber.h"
#include "GenericCircularPhotosensor.h"
#include "CylinderPointSampler2020.h"
#include "MaterialsList.h"

#include <G4MaterialPropertyVector.hh>


class G4Material;
class G4GenericMessenger;
class G4MaterialPropertiesTable;
class CylinderPointSampler2020;

namespace nexus
{
    class CylinderPointSampler2020;
    class Fiber_barrel_meth: public GeometryBase
    {
        public:
        Fiber_barrel_meth();
        ~Fiber_barrel_meth();
        void Construct();
        G4ThreeVector GenerateVertex(const G4String& region) const;

        private:
        // fiber
        G4String fiber_type_;   // Y11 or B2
        G4bool coated_;         // true or false for the fibers to be coated
        G4double diameter_;     // diameter of the cylindrical optical fibre
        G4double length_;       // length of the cylindrical optical fibre

        // methacrylate
        G4bool methacrylate_;
        G4double window_thickness_;

        // cylinder
        G4double radius_cyl_;         //radius of the cylinder
        G4double teflon_thickness_;   // thickness of the outer teflon cover
        G4bool caps_visibility_;
        G4bool teflon_visibility_;

        // sensor
        GenericCircularPhotosensor* photo_sensor_;
        G4String sensor_type_;        // SiPM, PMT, PERFECT, ...
        G4bool sensor_visibility_;

        CylinderPointSampler2020* cyl_vertex_gen_; // this creates photons homogeneously in a cylinder
        G4GenericMessenger*   msg_;


    };
} // namespace nexus
#endif
