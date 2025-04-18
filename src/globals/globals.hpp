#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <string>
#include <vector>
#include <map>

// Forward declare UPFReader to avoid circular dependency
class UPFReader;

// Global flag indicating if UPF data is valid
extern bool g_upf_data_valid;

// Global UPF data structures
struct UPFHeader {
    std::string element;
    std::string pseudo_type;
    double z_valence;
    int mesh_size;
    int l_max;
    bool is_ultrasoft;
    bool has_so;
};

extern UPFHeader g_upf_header;

// Global mesh data
extern std::vector<double> g_r_mesh;

// Global orbital data structure
struct GlobalOrbitalData {
    std::vector<double> values;
    std::vector<double> projector;
    int l;  // Corresponds to UPFReader::QuantumNumber
};

// Global orbitals map
extern std::map<int, std::vector<GlobalOrbitalData>> g_orbitals;  // Key is UPFReader::OrbitalType

// Global variables declarations
extern std::vector<double> g_local_potential;
extern std::map<int, std::vector<double>> g_nonlocal_potentials;  // Key is UPFReader::QuantumNumber
extern std::map<int, std::vector<double>> g_projectors;  // Key is UPFReader::QuantumNumber
extern std::map<int, std::vector<double>> g_total_potentials; // V_l^total(r) = V_local(r) + V_l^nonlocal(r) * P_l(r)

#endif // GLOBALS_HPP
