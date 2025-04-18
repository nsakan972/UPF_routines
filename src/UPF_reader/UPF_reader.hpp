#ifndef UPF_READER_HPP
#define UPF_READER_HPP

#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <pugixml.hpp>
#include "../globals/globals.hpp"

class UPFReader {
public:
    explicit UPFReader(const std::string& filename);
    
    bool parse();
    void display_info() const;

    // Orbital types
    enum class OrbitalType {
        LOCAL,
        NONLOCAL,
        WAVEFUNCTION
    };

    // Orbital quantum numbers
    enum class QuantumNumber {
        S = 0,
        P = 1,
        D = 2,
        F = 3
    };

private:
    std::string filename_;
    pugi::xml_document doc_;
    
    // Helper functions for parsing specific sections
    bool parse_header();
    bool parse_mesh();
    bool parse_local();
    bool parse_nonlocal();
    bool parse_wavefunctions();
    
    // Helper functions
    std::string get_orbital_name(QuantumNumber l) const;
    std::string get_type_name(OrbitalType type) const;
    
    // Helper functions for potential calculation
    void calculate_potentials();
    std::vector<double> get_local_potential() const;
    std::vector<double> get_nonlocal_potential(QuantumNumber l) const;
    std::vector<double> get_projector(QuantumNumber l) const;
    
    // Temporary data storage during parsing
    struct Header {
        std::string element;
        std::string pseudo_type;
        double z_valence;
        int mesh_size;
        int l_max;
        bool is_ultrasoft;
        bool has_so;
    } header_;

    std::vector<double> r_mesh_;
    
    struct OrbitalData {
        std::vector<double> values;
        std::vector<double> projector;
        QuantumNumber l;
    };
    
    std::map<OrbitalType, std::vector<OrbitalData>> orbitals_;
};

#endif // UPF_READER_HPP
