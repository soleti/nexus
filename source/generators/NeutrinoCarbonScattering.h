// ----------------------------------------------------------------------------
// nexus | NeutrinoCarbonScattering.h
//
// This generator simulates an electron and a positron from the same vertex,
// with total kinetic energy settable by parameter.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef BETA_DECAY_GEN_H
#define BETA_DECAY_GEN_H

#include <G4VPrimaryGenerator.hh>
#include <G4ParticleTable.hh>
#include <G4ParticleDefinition.hh>

#include <Randomize.hh>

class G4GenericMessenger;
class G4Event;
class G4ParticleDefinition;


namespace nexus {

  class GeometryBase;

  class NeutrinoCarbonScattering: public G4VPrimaryGenerator
  {
  public:
    /// Constructor
    NeutrinoCarbonScattering();
    /// Destructor
    ~NeutrinoCarbonScattering();

    /// This method is invoked at the beginning of the event. It sets
    /// a primary vertex (that is, a given position and time)
    /// in the event.
    void GeneratePrimaryVertex(G4Event*);

  private:

    /// Generate a random kinetic energy with flat probability in
    //  the interval [energy_min, energy_max].
    void SetupSamplers();

  private:
    G4GenericMessenger* msg_;

    const GeometryBase* geom_; ///< Pointer to the detector geometry

    G4String region_;

    bool initialized_;
    G4RandGeneral *positron_sampler_; ///< Pointer to the RNG distribution
    G4RandGeneral *electron_sampler_;

    const G4ParticleDefinition* electron_ =
      G4ParticleTable::GetParticleTable()->FindParticle("e-");

    const G4ParticleDefinition* positron_ =
      G4ParticleTable::GetParticleTable()->FindParticle("e+");

    const G4ParticleDefinition* neutron_ =
      G4ParticleTable::GetParticleTable()->FindParticle(2112);
    const G4double positron_mass_ = positron_->GetPDGMass();
    const G4double neutron_mass_ = neutron_->GetPDGMass();
  };

} // end namespace nexus

#endif
