#include "UPF_reader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

UPFReader::UPFReader(const std::string& filename)
    : filename_(filename) {
    // Initialize header with default values
    header_.element = "";
    header_.pseudo_type = "";
    header_.z_valence = 0.0;
    header_.mesh_size = 0;
    header_.l_max = 0;
    header_.is_ultrasoft = false;
    header_.has_so = false;
}

bool UPFReader::parse() {
    // Reset global validity flag
    g_upf_data_valid = false;

    // Load and parse the XML file
    pugi::xml_parse_result result = doc_.load_file(filename_.c_str());
    if (!result) {
        std::cerr << "Failed to parse UPF file: " << result.description() << "\n";
        return false;
    }

    // Clear D coefficients
    d_coefficients_.clear();

    // Parse different sections
    if (!parse_header() || !parse_mesh() || !parse_local() || !parse_nonlocal() || 
        !parse_wavefunctions() || !parse_dij()) {
        return false;
    }

    // Update global variables
    g_upf_header = {
        header_.element,
        header_.pseudo_type,
        header_.z_valence,
        header_.mesh_size,
        header_.l_max,
        header_.is_ultrasoft,
        header_.has_so
    };

    g_r_mesh = r_mesh_;
    
    // Convert and store orbitals in global storage
    for (const auto& [type, orbs] : orbitals_) {
        std::vector<GlobalOrbitalData> global_orbs;
        for (const auto& orb : orbs) {
            GlobalOrbitalData global_orb;
            global_orb.values = orb.values;
            global_orb.projector = orb.projector;
            global_orb.l = static_cast<int>(orb.l);
            global_orbs.push_back(global_orb);
        }
        g_orbitals[static_cast<int>(type)] = global_orbs;
    }

    // Calculate and store potentials
    calculate_potentials();

    // Calculate total potentials for each orbital momentum
    g_total_potentials.clear();
    for (const auto& [l, nonlocal_pot] : g_nonlocal_potentials) {
        const auto& projector_values = g_projectors[l];
        std::vector<double> total_potential(g_r_mesh.size(), 0.0);
        
        // Get number of projectors for this l
        size_t n_proj = d_coefficients_[l].size();
        size_t points_per_proj = projector_values.size() / n_proj;
        
        // V_l^total(r) = V_local(r) + Σ_{i,j} D_{i,j} P_{l,i}(r) P_{l,j}(r)
        for (size_t r = 0; r < g_r_mesh.size(); ++r) {
            total_potential[r] = g_local_potential[r];
            
            // Add the nonlocal contribution using D coefficients
            for (size_t i = 0; i < n_proj; ++i) {
                for (size_t j = 0; j < n_proj; ++j) {
                    total_potential[r] += d_coefficients_[l][i][j] * 
                                          projector_values[i * points_per_proj + r] * 
                                          projector_values[j * points_per_proj + r];
                }
            }
        }
        
        g_total_potentials[l] = std::move(total_potential);
    }

    // Set global validity flag
    g_upf_data_valid = true;

    return true;
}

bool UPFReader::parse_header() {
    // Get the PP_HEADER node
    pugi::xml_node header = doc_.child("UPF").child("PP_HEADER");
    if (!header) {
        std::cerr << "Error: PP_HEADER section not found\n";
        return false;
    }

    // Extract header information
    header_.element = header.attribute("element").as_string();
    header_.pseudo_type = header.attribute("pseudo_type").as_string();
    header_.z_valence = header.attribute("z_valence").as_double();
    header_.mesh_size = header.attribute("mesh_size").as_int();
    header_.l_max = header.attribute("l_max").as_int();
    header_.is_ultrasoft = std::string(header.attribute("is_ultrasoft").as_string()) == "T";
    header_.has_so = std::string(header.attribute("has_so").as_string()) == "T";

    return true;
}

void UPFReader::display_info() const {
    std::cout << "UPF File Information:\n";
    std::cout << "------------------\n";
    std::cout << "Element: " << header_.element << "\n";
    std::cout << "Pseudo Type: " << header_.pseudo_type << "\n";
    std::cout << "Z Valence: " << header_.z_valence << "\n";
    std::cout << "Mesh Size: " << header_.mesh_size << "\n";
    std::cout << "L Max: " << header_.l_max << "\n";
    std::cout << "Is Ultrasoft: " << (header_.is_ultrasoft ? "Yes" : "No") << "\n";
    std::cout << "Has Spin-Orbit: " << (header_.has_so ? "Yes" : "No") << "\n";
    
    // Display orbital information
    for (const auto& [type, orbs] : orbitals_) {
        std::cout << "\nOrbital Type: " << get_type_name(type) << "\n";
        for (const auto& orb : orbs) {
            std::cout << "  " << get_orbital_name(orb.l) << " orbital: " 
                      << orb.values.size() << " points\n";
        }
    }
}

bool UPFReader::parse_mesh() {
    pugi::xml_node mesh = doc_.child("UPF").child("PP_MESH").child("PP_R");
    if (!mesh) {
        std::cerr << "Error: PP_MESH/PP_R section not found\n";
        return false;
    }

    std::string mesh_str = mesh.text().get();
    std::istringstream iss(mesh_str);
    double value;
    while (iss >> value) {
        r_mesh_.push_back(value);
    }

    return true;
}

bool UPFReader::parse_local() {
    pugi::xml_node local = doc_.child("UPF").child("PP_LOCAL");
    if (!local) {
        return true; // Local potential is optional
    }

    std::string local_str = local.text().get();
    std::istringstream iss(local_str);
    std::vector<double> values;
    double value;
    while (iss >> value) {
        values.push_back(value);
    }

    if (!values.empty()) {
        OrbitalData data;
        data.values = values;
        data.projector = {};
        data.l = QuantumNumber::S;
        orbitals_[OrbitalType::LOCAL].push_back(data);
    }

    return true;
}

bool UPFReader::parse_nonlocal() {
    pugi::xml_node nonlocal = doc_.child("UPF").child("PP_NONLOCAL");
    if (!nonlocal) {
        return true; // Nonlocal potential is optional
    }

    for (int l = 0; l <= header_.l_max; ++l) {
        std::string beta_name = "PP_BETA." + std::to_string(l + 1);
        pugi::xml_node beta = nonlocal.child(beta_name.c_str());
        if (!beta) continue;

        // Get nonlocal potential
        std::string beta_str = beta.text().get();
        std::istringstream iss(beta_str);
        std::vector<double> values;
        double value;
        while (iss >> value) {
            values.push_back(value);
        }

        // Get projector function
        std::string proj_name = "PP_BETA_" + std::to_string(l + 1);
        pugi::xml_node proj = nonlocal.child(proj_name.c_str());
        std::vector<double> projector;
        
        if (proj) {
            std::string proj_str = proj.text().get();
            std::istringstream proj_iss(proj_str);
            while (proj_iss >> value) {
                projector.push_back(value);
            }
        } else {
            // If no explicit projector, use the beta function as projector
            projector = values;
        }

        if (!values.empty()) {
            OrbitalData data;
            data.values = values;
            data.projector = projector;
            data.l = static_cast<QuantumNumber>(l);
            orbitals_[OrbitalType::NONLOCAL].push_back(data);
        }
    }

    return true;
}

bool UPFReader::parse_wavefunctions() {
    pugi::xml_node chi = doc_.child("UPF").child("PP_CHI");
    if (!chi) {
        return true; // Wavefunctions are optional
    }

    for (int l = 0; l <= header_.l_max; ++l) {
        std::string chi_name = "PP_CHI." + std::to_string(l + 1);
        pugi::xml_node wfc = chi.child(chi_name.c_str());
        if (!wfc) continue;

        std::string wfc_str = wfc.text().get();
        std::istringstream iss(wfc_str);
        std::vector<double> values;
        double value;
        while (iss >> value) {
            values.push_back(value);
        }

        if (!values.empty()) {
            OrbitalData data;
            data.values = values;
            data.projector = {};
            data.l = static_cast<QuantumNumber>(l);
            orbitals_[OrbitalType::WAVEFUNCTION].push_back(data);
        }
    }

    return true;
}





std::string UPFReader::get_orbital_name(QuantumNumber l) const {
    switch (l) {
        case QuantumNumber::S: return "s";
        case QuantumNumber::P: return "p";
        case QuantumNumber::D: return "d";
        case QuantumNumber::F: return "f";
        default: return "unknown";
    }
}

std::string UPFReader::get_type_name(OrbitalType type) const {
    switch (type) {
        case OrbitalType::LOCAL: return "local";
        case OrbitalType::NONLOCAL: return "nonlocal";
        case OrbitalType::WAVEFUNCTION: return "wavefunction";
        default: return "unknown";
    }
}

std::vector<double> UPFReader::get_local_potential() const {
    for (const auto& orbital : orbitals_.at(OrbitalType::LOCAL)) {
        if (orbital.l == QuantumNumber::S) {
            return orbital.values;
        }
    }
    return std::vector<double>();
}

std::vector<double> UPFReader::get_nonlocal_potential(QuantumNumber l) const {
    for (const auto& orbital : orbitals_.at(OrbitalType::NONLOCAL)) {
        if (orbital.l == l) {
            return orbital.values;
        }
    }
    return std::vector<double>();
}

std::vector<double> UPFReader::get_projector(QuantumNumber l) const {
    for (const auto& orbital : orbitals_.at(OrbitalType::NONLOCAL)) {
        if (orbital.l == l) {
            return orbital.projector;
        }
    }
    return std::vector<double>();
}

bool UPFReader::parse_dij() {
    pugi::xml_node dij = doc_.child("UPF").child("PP_NONLOCAL").child("PP_DIJ");
    if (!dij) {
        std::cerr << "Error: PP_DIJ section not found\n";
        return false;
    }

    std::string dij_str = dij.text().get();
    std::istringstream iss(dij_str);
    
    // Initialize D coefficients matrix for each l
    for (int l = 0; l <= header_.l_max; ++l) {
        size_t n_proj = 0;
        // Count projectors for this l
        for (const auto& orbital : orbitals_[OrbitalType::NONLOCAL]) {
            if (orbital.l == static_cast<QuantumNumber>(l)) {
                n_proj++;
            }
        }
        
        // Initialize matrix
        std::vector<std::vector<double>> d_matrix(n_proj, std::vector<double>(n_proj));
        
        // Read D coefficients
        for (size_t i = 0; i < n_proj; ++i) {
            for (size_t j = 0; j < n_proj; ++j) {
                double value;
                if (!(iss >> value)) {
                    std::cerr << "Error: Not enough D coefficients in PP_DIJ\n";
                    return false;
                }
                d_matrix[i][j] = value;
            }
        }
        
        d_coefficients_[l] = std::move(d_matrix);
    }

    return true;
}

void UPFReader::calculate_potentials() {
    // Store local potential
    g_local_potential = get_local_potential();

    // Store nonlocal potentials and projectors for each quantum number
    for (int l = 0; l <= header_.l_max; ++l) {
        auto qn = static_cast<QuantumNumber>(l);
        g_nonlocal_potentials[static_cast<int>(qn)] = get_nonlocal_potential(qn);
        
        // Get all projectors for this l and combine them into a single vector
        std::vector<double> combined_projectors;
        for (const auto& orbital : orbitals_[OrbitalType::NONLOCAL]) {
            if (orbital.l == qn) {
                combined_projectors.insert(combined_projectors.end(), 
                                        orbital.projector.begin(), 
                                        orbital.projector.end());
            }
        }
        g_projectors[static_cast<int>(qn)] = std::move(combined_projectors);
    }
}
