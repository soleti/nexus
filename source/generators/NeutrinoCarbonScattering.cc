// ----------------------------------------------------------------------------
// nexus | NeutrinoCarbonScattering.cc
//
// This generator simulates an electron and a positron from the same vertex,
// with total kinetic energy settable by parameter.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "NeutrinoCarbonScattering.h"
#include "GeometryBase.h"
#include "FactoryBase.h"

#include "DetectorConstruction.h"

#include <G4GenericMessenger.hh>
#include <G4RunManager.hh>
#include <G4PrimaryVertex.hh>
#include <G4Event.hh>
#include <G4RandomDirection.hh>
#include <Randomize.hh>
#include <G4OpticalPhoton.hh>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>

using namespace nexus;
using namespace CLHEP;

REGISTER_CLASS(NeutrinoCarbonScattering, G4VPrimaryGenerator)

NeutrinoCarbonScattering::NeutrinoCarbonScattering():
G4VPrimaryGenerator(), msg_(0),
geom_(0), initialized_(false)
{
  msg_ = new G4GenericMessenger(this, "/Generator/NeutrinoCarbonScattering/",
    "Control commands of single-particle generator.");

  msg_->DeclareProperty("region", region_,
    "Set the region of the geometry where the vertex will be generated.");

  DetectorConstruction* detconst = (DetectorConstruction*) G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  geom_ = detconst->GetGeometry();

}


void NeutrinoCarbonScattering::SetupSamplers() {
  G4int npti = 1000;
  G4double* positron_pdf = new G4double[npti];
  G4double E_max = 16.83 * MeV;
  G4double E;
  G4double energy_step = E_max / npti;

  for (G4int ptn = 0; ptn < npti; ptn++) {

    E = energy_step * ptn;

    if (E < positron_mass_) {
      positron_pdf[ptn] = 0;
    } else if (E > E_max) {
      positron_pdf[ptn] = 0;
    } else {
      G4double p = sqrt(E*E - positron_mass_*positron_mass_);
      G4double beta = p/E;
      G4double eta = 17. / 137. / beta;
      positron_pdf[ptn] = E*pow((E_max-E),2) * 2 * pi * eta/(exp(2*pi*eta) - 1);
    }
  }

  positron_sampler_ = new G4RandGeneral(positron_pdf, npti);

  G4double electron_pdf[] = {
    0,
    9.14E+00,
    2.76E+01,
    5.36E+01,
    7.83E+01,
    1.06E+02,
    1.19E+02,
    1.33E+02,
    1.49E+02,
    1.67E+02,
    1.85E+02,
    2.03E+02,
    2.22E+02,
    2.42E+02,
    2.61E+02,
    2.82E+02,
    3.02E+02,
    3.24E+02,
    3.46E+02,
    3.68E+02,
    3.91E+02,
    4.15E+02,
    4.38E+02,
    4.58E+02,
    4.75E+02,
    4.9E+02,
    5.03E+02,
    5.14E+02,
    5.23E+02,
    5.3E+02,
    5.35E+02,
    5.36E+02,
    5.3E+02,
    5.22E+02,
    5.13E+02,
    4.99E+02,
    4.82E+02,
    4.6E+02,
    4.36E+02,
    4E+02,
    3.64E+02,
    3.31E+02,
    2.98E+02,
    2.61E+02,
    2.27E+02,
    1.92E+02,
    1.58E+02,
    1.18E+02,
    7.84E+01,
    4.63E+01,
    3.89E+00
  };

  electron_sampler_ = new G4RandGeneral(electron_pdf, 51);

  delete[] positron_pdf;

}


NeutrinoCarbonScattering::~NeutrinoCarbonScattering()
{
  delete msg_;
}


void NeutrinoCarbonScattering::GeneratePrimaryVertex(G4Event* event)
{

  if (!initialized_) {
      SetupSamplers();
      initialized_ = true;
  }

  G4ParticleDefinition* neutron =
    G4ParticleTable::GetParticleTable()->FindParticle(2112);

  // Generate an initial position for the particle using the geometry
  G4ThreeVector pos = geom_->GenerateVertex(region_);

  // Particle generated at start-of-event
  G4double time_electron = G4UniformRand() * 2.8 * ms;
  G4double time_positron = time_electron + G4RandExponential::shoot() * 15.9 * ms;

  G4PrimaryVertex* vertex_electron = new G4PrimaryVertex(pos, time_electron);

  G4double E_max_electron = 35.5 * MeV;
  G4double electron_kinetic_energy = electron_sampler_->fire() * E_max_electron;

  G4ThreeVector momentum_direction_electron = G4RandomDirection();
  // Calculate cartesian components of momentum
  G4double energy_electron = electron_kinetic_energy + positron_mass_;
  G4double pmod_electron = std::sqrt(energy_electron*energy_electron - positron_mass_*positron_mass_);
  G4double px_electron = pmod_electron * momentum_direction_electron.x();
  G4double py_electron = pmod_electron * momentum_direction_electron.y();
  G4double pz_electron = pmod_electron * momentum_direction_electron.z();

  G4PrimaryParticle* electron_particle =
    new G4PrimaryParticle(electron_, px_electron, py_electron, pz_electron);

  vertex_electron->SetPrimary(electron_particle);
  event->AddPrimaryVertex(vertex_electron);

  // Create a new vertex
  G4PrimaryVertex* vertex_positron = new G4PrimaryVertex(pos, time_positron);

  G4double E_max = 16.83 * MeV;
  G4double positron_kinetic_energy = positron_sampler_->fire() * E_max - positron_mass_;

  G4ThreeVector _momentum_direction = G4RandomDirection();

  // Calculate cartesian components of momentum
  G4double energy = positron_kinetic_energy + positron_mass_;
  G4double pmod = std::sqrt(energy*energy - positron_mass_*positron_mass_);
  G4double px = pmod * _momentum_direction.x();
  G4double py = pmod * _momentum_direction.y();
  G4double pz = pmod * _momentum_direction.z();

  // Create the new primary particle and set it some properties
  G4PrimaryParticle* positron_particle =
    new G4PrimaryParticle(positron_, px, py, pz);

  // Add particle to the vertex and this to the event
  vertex_positron->SetPrimary(positron_particle);

  G4double neutron_fit[] = {-3.29e-06,  1.08e-03, -2.69e-03,  2.92e-02};

  G4double kinetic_energy2 = neutron_fit[3] +
                             positron_kinetic_energy*neutron_fit[2] +
                             positron_kinetic_energy*positron_kinetic_energy*neutron_fit[1] +
                             positron_kinetic_energy*positron_kinetic_energy*positron_kinetic_energy*neutron_fit[0];

  // Generate random direction by default
  G4ThreeVector _momentum_direction2 = G4RandomDirection();
  //   // Calculate cartesian components of momentum
  G4double energy2 = kinetic_energy2 + neutron_mass_;
  G4double pmod2 = std::sqrt(energy2*energy2 - neutron_mass_*neutron_mass_);
  G4double px2 = pmod2 * _momentum_direction2.x();
  G4double py2 = pmod2 * _momentum_direction2.y();
  G4double pz2 = pmod2 * _momentum_direction2.z();

  // Create the new primary particle and set it some properties
  G4PrimaryParticle* neutron_particle =
    new G4PrimaryParticle(neutron, px2, py2, pz2);

  // // Add particle to the vertex and this to the event
  vertex_positron->SetPrimary(neutron_particle);

  event->AddPrimaryVertex(vertex_positron);
}
