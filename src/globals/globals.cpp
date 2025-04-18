#include "globals.hpp"

// Global flag indicating if UPF data is valid
bool g_upf_data_valid = false;

// Global UPF header data
struct UPFHeader g_upf_header = {
    "",  // element
    "",  // pseudo_type
    0,    // z_valence
    false // relativistic
};

// Global mesh data
std::vector<double> g_r_mesh;

// Global orbital data
std::map<int, std::vector<GlobalOrbitalData>> g_orbitals;

// Global potential data
std::vector<double> g_local_potential;
std::map<int, std::vector<double>> g_nonlocal_potentials;
std::map<int, std::vector<double>> g_projectors;
std::map<int, std::vector<double>> g_total_potentials;
