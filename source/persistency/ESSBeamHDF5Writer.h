#ifndef ESSBEAM_HDF5WRITER_H
#define ESSBEAM_HDF5WRITER_H

#include "ESSBeamhdf5_functions.h"

#include <hdf5.h>
#include <iostream>

namespace nexus {

  class ESSBeamHDF5Writer {

  public:
    //! constructor
    ESSBeamHDF5Writer();
    /// destructor
    ~ESSBeamHDF5Writer();

    //! open file
    void Open(std::string filename);

    //! close file
    void Close();

    void WriteRunInfo(const char* param_key, const char* param_value);
    void WriteSensorDataInfo(int evt_number, unsigned int sensor_id, unsigned int time_bin, unsigned int charge);
    void WriteHitInfo(int evt_number, int particle_indx, int hit_indx, float hit_position_x, float hit_position_y, float hit_position_z, float hit_time, float hit_energy, const char* label);
    void WriteParticleInfo(int evt_number, int particle_indx, const char* particle_name, char primary, const char* mother_name, float initial_vertex_x, float initial_vertex_y, float initial_vertex_z, float initial_vertex_t, float final_vertex_x, float final_vertex_y, float final_vertex_z, float final_vertex_t, const char* initial_volume, const char* final_volume, float momentum_x, float momentum_y, float momentum_z, float final_momentum_x, float final_momentum_y, float final_momentum_z, float kin_energy, const char* creator_proc);
    void WriteSensorPosInfo(unsigned int sensor_id, float x, float y, float z);

  private:
    size_t _file; ///< HDF5 file

    bool _isOpen;
    bool _firstEvent; ///< First event

    size_t _group; ///< group for everything

    //Datasets
    size_t _runTable;
    size_t _snsDataTable;
    size_t _hitInfoTable;
    size_t _particleInfoTable;
    size_t _snsPosTable;

    size_t _memtypeRun;
    size_t _memtypeSnsData;
    size_t _memtypeHitInfo;
    size_t _memtypeParticleInfo;
    size_t _memtypeSnsPos;

    size_t _irun; ///< counter for configuration parameters
    size_t _ismp; ///< counter for written waveform samples
    size_t _ihit; ///< counter for true information
    size_t _ipart; ///< counter for particle information
    size_t _ipos; ///< counter for sensor positions

  };

} // namespace nexus

#endif