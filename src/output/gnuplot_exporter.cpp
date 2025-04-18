#include "gnuplot_exporter.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

GnuplotExporter::GnuplotExporter(const std::filesystem::path& output_dir, const std::string& element)
    : output_dir_(output_dir), element_name_(element) {
    std::filesystem::create_directories(output_dir_);
}

bool GnuplotExporter::export_local_potential() const {
    auto data_file = output_dir_ / (element_name_ + "_local_potential.dat");
    auto script_file = output_dir_ / "plot_local_potential.gp";
    
    if (!write_data_file(data_file.string(), g_r_mesh, g_local_potential)) {
        return false;
    }

    std::string plot_command = "plot '" + data_file.filename().string() + "' using 1:2 with lines title 'V_{loc}(r)'";
    std::string title = "Local Potential for " + element_name_;

    // Write gnuplot script
    if (!write_gnuplot_script(script_file.string(), title, plot_command)) {
        return false;
    }

    return true;
}

bool GnuplotExporter::export_nonlocal_potentials() const {
    auto data_file = output_dir_ / (element_name_ + "_nonlocal_potentials.dat");
    auto script_file = output_dir_ / "plot_nonlocal_potentials.gp";
    
    std::map<std::string, std::vector<double>> y_data_map;
    for (const auto& [l, data] : g_nonlocal_potentials) {
        y_data_map[std::to_string(l)] = data;
    }

    if (!write_multi_data_file(data_file.string(), g_r_mesh, y_data_map)) {
        return false;
    }

    std::stringstream plot_cmd;
    plot_cmd << "plot ";
    bool first = true;
    int i = 0;
    for (const auto& [l, _] : g_nonlocal_potentials) {
        if (!first) plot_cmd << ", ";
        plot_cmd << "'" << data_file.filename().string() << "' using 1:" 
                << (i+2)
                << " with lines title 'V_{nl," << l << "}(r)'";
        first = false;
        i++;
    }
    
    std::string title = "Non-local Potentials for " + element_name_;
    return write_gnuplot_script(script_file.string(), title, plot_cmd.str());
}

bool GnuplotExporter::export_projectors() const {
    auto data_file = output_dir_ / (element_name_ + "_projectors.dat");
    auto script_file = output_dir_ / "plot_projectors.gp";
    
    std::map<std::string, std::vector<double>> y_data_map;
    for (const auto& [l, data] : g_projectors) {
        y_data_map[std::to_string(l)] = data;
    }

    if (!write_multi_data_file(data_file.string(), g_r_mesh, y_data_map)) {
        return false;
    }

    std::stringstream plot_cmd;
    plot_cmd << "plot ";
    bool first = true;
    int i = 0;
    for (const auto& [l, _] : g_projectors) {
        if (!first) plot_cmd << ", ";
        plot_cmd << "'" << data_file.filename().string() << "' using 1:" 
                << (i+2)
                << " with lines title 'P_{" << l << "}(r)'";
        first = false;
        i++;
    }
    
    std::string title = "Projector Functions for " + element_name_;
    return write_gnuplot_script(script_file.string(), title, plot_cmd.str());
}

bool GnuplotExporter::export_orbital_values() const {
    for (const auto& [type, orbitals] : g_orbitals) {
        // Get orbital type name (s, p, d, ...)
        std::string orbital_type;
        switch(type) {
            case 0: orbital_type = "s"; break;
            case 1: orbital_type = "p"; break;
            case 2: orbital_type = "d"; break;
            case 3: orbital_type = "f"; break;
            default: orbital_type = std::to_string(type);
        }
        
        // Create data file for this orbital type
        auto data_file = output_dir_ / (element_name_ + "_orbital_" + orbital_type + ".dat");
        auto script_file = output_dir_ / ("plot_orbitals_" + orbital_type + ".gp");
        
        std::map<std::string, std::vector<double>> y_data_map;
        int i = 0;
        for (const auto& orb : orbitals) {
            y_data_map[std::to_string(i++)] = orb.values;
        }

        if (!write_multi_data_file(data_file.string(), g_r_mesh, y_data_map)) {
            return false;
        }

        std::stringstream plot_cmd;
        plot_cmd << "plot ";
        bool first = true;
        i = 0;
        for (const auto& orb : orbitals) {
            if (!first) plot_cmd << ", ";
            plot_cmd << "'" << data_file.filename().string() << "' using 1:" 
                    << (i+2)
                    << " with lines title '" + element_name_ + " " + orbital_type + " l=" + std::to_string(orb.l) + "'";
            first = false;
            i++;
        }
        
        std::string title = "Orbital Values for " + element_name_ + 
                           " (" + orbital_type + ")";
        if (!write_gnuplot_script(script_file.string(), title, plot_cmd.str())) {
            return false;
        }
    }

    return true;
}

bool GnuplotExporter::export_total_potentials() const {
    auto data_file = output_dir_ / (element_name_ + "_total_potentials.dat");
    auto script_file = output_dir_ / "plot_total_potentials.gp";
    
    std::map<std::string, std::vector<double>> y_data_map;
    for (const auto& [l, data] : g_total_potentials) {
        y_data_map[std::to_string(l)] = data;
    }

    if (!write_multi_data_file(data_file.string(), g_r_mesh, y_data_map)) {
        return false;
    }

    std::stringstream plot_cmd;
    plot_cmd << "plot ";
    bool first = true;
    int i = 0;
    for (const auto& [l, _] : g_total_potentials) {
        if (!first) plot_cmd << ", ";
        plot_cmd << "'" << data_file.filename().string() << "' using 1:" 
                << (i+2)
                << " with lines title 'V_{tot," << l << "}(r)'";
        first = false;
        i++;
    }
    
    std::string title = "Total Potentials for " + element_name_;
    return write_gnuplot_script(script_file.string(), title, plot_cmd.str());
}

bool GnuplotExporter::export_all() const {
    if (!g_upf_data_valid) {
        std::cerr << "Error: No valid UPF data available for plotting\n";
        return false;
    }

    return export_local_potential() &&
           export_nonlocal_potentials() &&
           export_projectors() &&
           export_orbital_values() &&
           export_total_potentials();
}

bool GnuplotExporter::write_gnuplot_script(const std::string& filename,
                                         const std::string& title,
                                         const std::string& plot_command) const {
    std::ofstream script(filename);
    if (!script) {
        std::cerr << "Failed to create gnuplot script file: " << filename << "\n";
        return false;
    }

    // Common settings for both linear and log scale
    auto write_common_settings = [&script](const std::string& title, bool logscale) {
        script << "set title '" << title << (logscale ? " (log scale)" : "") << "' enhanced\n"
               << "set xlabel 'r (a_{0})" << (logscale ? " [log]" : "") << "' enhanced\n"  // Bohr radius
               << "set ylabel 'V(r) (Ry)' enhanced\n"
               << "set grid\n";
        if (logscale) {
            script << "set logscale x\n";
        } else {
            script << "unset logscale x\n";
        }
    };

    // Helper for writing plots in different formats
    auto write_plot = [&](const std::string& terminal, const std::string& suffix = "") {
        // Linear scale plot
        write_common_settings(title, false);
        script << "set terminal " << terminal << "\n";
        if (!suffix.empty()) {
            script << "set output '" << std::filesystem::path(filename).stem().string() 
                   << suffix << ".eps'\n";
        }
        script << plot_command << "\n\n";

        // Log scale plot
        write_common_settings(title, true);
        script << "set terminal " << terminal << "\n";
        if (!suffix.empty()) {
            script << "set output '" << std::filesystem::path(filename).stem().string() 
                   << suffix << "_log.eps'\n";
        }
        script << plot_command << "\n\n";
    };

    // X11 terminal (interactive)
    write_plot("x11");
    script << "pause -1 'Press any key to continue'\n\n";

    // Color PostScript output
    write_plot("postscript enhanced color", "_color");

    // Monochrome PostScript output
    write_plot("postscript enhanced monochrome", "_mono");

    // Reset terminal to interactive mode
    script << "set terminal x11\n"
           << "set output\n";

    return true;
}

bool GnuplotExporter::write_data_file(const std::string& filename,
                                    const std::vector<double>& x_data,
                                    const std::vector<double>& y_data) const {
    if (x_data.size() != y_data.size()) {
        std::cerr << "Error: x and y data sizes do not match\n";
        return false;
    }

    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to create data file: " << filename << "\n";
        return false;
    }

    file << std::scientific;  // Use scientific notation for floating-point numbers
    for (size_t i = 0; i < x_data.size(); ++i) {
        file << x_data[i] << "\t" << y_data[i] << "\n";
    }

    return true;
}

bool GnuplotExporter::write_multi_data_file(const std::string& filename,
                                          const std::vector<double>& x_data,
                                          const std::map<std::string, std::vector<double>>& y_data_map) const {
    // Check if all y_data vectors have the same size as x_data
    for (const auto& [_, y_data] : y_data_map) {
        if (x_data.size() != y_data.size()) {
            std::cerr << "Error: x and y data sizes do not match\n";
            return false;
        }
    }

    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to create data file: " << filename << "\n";
        return false;
    }

    file << std::scientific;  // Use scientific notation for floating-point numbers

    // Write header
    file << "# r";
    for (const auto& [label, _] : y_data_map) {
        file << "\t" << label;
    }
    file << "\n";

    // Write data
    for (size_t i = 0; i < x_data.size(); ++i) {
        file << x_data[i];
        for (const auto& [_, y_data] : y_data_map) {
            file << "\t" << y_data[i];
        }
        file << "\n";
    }

    return true;
}

std::string GnuplotExporter::get_quantum_number_label(int l) const {
    switch (l) {
        case static_cast<int>(UPFReader::QuantumNumber::S): return "s";
        case static_cast<int>(UPFReader::QuantumNumber::P): return "p";
        case static_cast<int>(UPFReader::QuantumNumber::D): return "d";
        case static_cast<int>(UPFReader::QuantumNumber::F): return "f";
        default: return "unknown";
    }
}

std::string GnuplotExporter::get_orbital_type_name(int type) const {
    switch (type) {
        case static_cast<int>(UPFReader::OrbitalType::LOCAL): return "local";
        case static_cast<int>(UPFReader::OrbitalType::NONLOCAL): return "nonlocal";
        case static_cast<int>(UPFReader::OrbitalType::WAVEFUNCTION): return "wavefunction";
        default: return "unknown";
    }
}
