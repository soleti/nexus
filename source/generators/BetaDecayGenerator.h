// ----------------------------------------------------------------------------
// nexus | BetaDecayGenerator.h
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

  class BetaDecayGenerator: public G4VPrimaryGenerator
  {
  public:
    /// Constructor
    BetaDecayGenerator();
    /// Destructor
    ~BetaDecayGenerator();

    /// This method is invoked at the beginning of the event. It sets
    /// a primary vertex (that is, a given position and time)
    /// in the event.
    void GeneratePrimaryVertex(G4Event*);

  private:

    /// Generate a random kinetic energy with flat probability in
    //  the interval [energy_min, energy_max].
    G4double RandomEnergy(G4double emin, G4double emax) const;
    void SetupIBDSampler();

  private:
    G4GenericMessenger* msg_;

    G4ParticleDefinition* particle_definition_;

    const GeometryBase* geom_; ///< Pointer to the detector geometry

    G4String region_;

    bool initialized_;
    G4RandGeneral *IBDSampler_; ///< Pointer to the RNG distribution
    G4RandGeneral *anti_nu_e_sampler_;
    G4RandGeneral *electron_disapp_sampler_;
    const G4ParticleDefinition* muon_ =
      G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    const G4double muon_mass_ = muon_->GetPDGMass();
  };

} // end namespace nexus

#endif
