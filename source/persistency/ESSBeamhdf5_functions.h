#ifndef ESSBEAM_HDF5_FUNCTIONS_H
#define ESSBEAM_HDF5_FUNCTIONS_H

#include <hdf5.h>
#include <iostream>

#define CONFLEN 300
#define STRLEN 20

  typedef struct{
     char param_key[CONFLEN];
     char param_value[CONFLEN];
   } run_info_t;

  typedef struct{
    int32_t event_id;
    unsigned int sensor_id;
    uint64_t time_bin;
    unsigned int charge;
  } sns_data_t;

  typedef struct{
    int32_t event_id;
    float x;
    float y;
    float z;
    float time;
    float energy;
    char label[STRLEN];
    int particle_id;
    int hit_id;
  } hit_info_t;

  typedef struct{
    int32_t event_id;
    int particle_id;
    char name[STRLEN];
    char primary;
    char mother_name[STRLEN];
    float initial_x;
    float initial_y;
    float initial_z;
    float initial_t;
    float final_x;
    float final_y;
    float final_z;
    float final_t;
    char initial_volume[STRLEN];
    char final_volume[STRLEN];
    float initial_momentum_x;
    float initial_momentum_y;
    float initial_momentum_z;
    float final_momentum_x;
    float final_momentum_y;
    float final_momentum_z;
    float kin_energy;
    char creator_proc[CONFLEN];
  } particle_info_t;

  typedef struct{
    unsigned int sensor_id;
    float x;
    float y;
    float z;
  } sns_pos_t;

  hsize_t createESSBeamRunType();
  hsize_t createESSBeamSensorDataType();
  hsize_t createESSBeamHitInfoType();
  hsize_t createESSBeamParticleInfoType();
  hsize_t createESSBeamSensorPosType();

  hid_t createESSBeamTable(hid_t group, std::string& table_name, hsize_t memtype);
  hid_t createESSBeamGroup(hid_t file, std::string& groupName);

  void writeESSBeamRun(run_info_t* runData, hid_t dataset, hid_t memtype, hsize_t counter);
  void writeESSBeamSnsData(sns_data_t* snsData, hid_t dataset, hid_t memtype, hsize_t counter);
  void writeESSBeamHit(hit_info_t* hitInfo, hid_t dataset, hid_t memtype, hsize_t counter);
  void writeESSBeamParticle(particle_info_t* particleInfo, hid_t dataset, hid_t memtype, hsize_t counter);
  void writeESSBeamSnsPos(sns_pos_t* snsPos, hid_t dataset, hid_t memtype, hsize_t counter);


#endif