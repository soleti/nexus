// -----------------------------------------------------------------------------
//  nexus | GenericCircularPhotosensor.h
//
//  Geometry of a configurable box-shaped photosensor.
//
//  The NEXT Collaboration
// -----------------------------------------------------------------------------

#ifndef GENERIC_CIRCULAR_PHOTOSENSOR_H
#define GENERIC_CIRCULAR_PHOTOSENSOR_H

#include "GeometryBase.h"
#include <G4MaterialPropertyVector.hh>

class G4Material;
class G4GenericMessenger;
class G4MaterialPropertiesTable;

namespace nexus {

  class GenericCircularPhotosensor: public GeometryBase
  {
  public:
    // Constructor for a circular sensor
    // The default thickness corresponds to a typical value for
    // a silicon photomultiplier.
    GenericCircularPhotosensor(G4String name,   G4double diameter,
                               G4double thickness = 2.0 * mm);

    // Destructor
    ~GenericCircularPhotosensor();

    //
    void Construct();

    //
    G4double GetDiameter()       const;
    G4double GetThickness()   const;
    const G4String& GetName() const;

    void SetVisibility           (G4bool visibility);
    void SetWithWLSCoating       (G4bool with_wls_coating);
    void SetWindowRefractiveIndex(G4MaterialPropertyVector* rindex);
    void SetOpticalProperties    (G4MaterialPropertiesTable* mpt);
    void SetTimeBinning          (G4double time_binning);
    void SetSensorDepth          (G4int sensor_depth);
    void SetMotherDepth          (G4int mother_depth);
    void SetNamingOrder          (G4int naming_order);

  private:

    void ComputeDimensions();
    void DefineMaterials();

    G4String name_;

    G4double diameter_, thickness_;
    G4double window_thickness_;
    G4double sensarea_thickness_;
    G4double wls_thickness_;
    G4double reduced_size_;

    G4Material* case_mat_;
    G4Material* window_mat_;
    G4Material* sensitive_mat_;
    G4Material* wls_mat_;

    G4bool                     with_wls_coating_;
    G4MaterialPropertyVector*  window_rindex_;
    G4MaterialPropertiesTable* sensitive_mpt_;

    G4int    sensor_depth_;
    G4int    mother_depth_;
    G4int    naming_order_;
    G4double time_binning_;

    G4bool visibility_;
  };


  inline G4double GenericCircularPhotosensor::GetDiameter()    const { return diameter_; }
  inline G4double GenericCircularPhotosensor::GetThickness()   const { return thickness_; }
  inline const G4String& GenericCircularPhotosensor::GetName() const { return name_; }

  inline void GenericCircularPhotosensor::SetVisibility(G4bool visibility)
  { visibility_ = visibility; }

  inline void GenericCircularPhotosensor::SetWithWLSCoating(G4bool with_wls_coating)
  { with_wls_coating_ = with_wls_coating; }

  inline void GenericCircularPhotosensor::SetWindowRefractiveIndex(G4MaterialPropertyVector* rindex)
  { window_rindex_ = rindex; }

  inline void GenericCircularPhotosensor::SetOpticalProperties(G4MaterialPropertiesTable* mpt)
  { sensitive_mpt_ = mpt; }

  inline void GenericCircularPhotosensor::SetTimeBinning(G4double time_binning)
  { time_binning_ = time_binning; }

  inline void GenericCircularPhotosensor::SetSensorDepth(G4int sensor_depth)
  { sensor_depth_ = sensor_depth; }

  inline void GenericCircularPhotosensor::SetMotherDepth(G4int mother_depth)
  { mother_depth_ = mother_depth; }

  inline void GenericCircularPhotosensor::SetNamingOrder(G4int naming_order)
  { naming_order_ = naming_order; }


} // namespace nexus

#endif
