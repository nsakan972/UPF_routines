#ifndef GNUPLOT_EXPORTER_HPP
#define GNUPLOT_EXPORTER_HPP

#include <string>
#include <filesystem>
#include "../globals/globals.hpp"
#include "../UPF_reader/UPF_reader.hpp"

class GnuplotExporter {
public:
    explicit GnuplotExporter(const std::filesystem::path& output_dir, const std::string& element = "");

    // Export functions for different data types
    bool export_local_potential() const;
    bool export_nonlocal_potentials() const;
    bool export_projectors() const;
    bool export_orbital_values() const;
    bool export_total_potentials() const;
    
    // Export all data at once
    bool export_all() const;

private:
    std::filesystem::path output_dir_;
    std::string element_name_;
    
    // Helper functions
    bool write_gnuplot_script(const std::string& filename,
                             const std::string& title,
                             const std::string& plot_command) const;
    
    bool write_data_file(const std::string& filename,
                        const std::vector<double>& x_data,
                        const std::vector<double>& y_data) const;

    bool write_multi_data_file(const std::string& filename,
                              const std::vector<double>& x_data,
                              const std::map<std::string, std::vector<double>>& y_data_map) const;

    std::string get_quantum_number_label(int l) const;
    std::string get_orbital_type_name(int type) const;
};

#endif // GNUPLOT_EXPORTER_HPP
