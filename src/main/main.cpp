#include "main.hpp"

bool file_exists(const std::string& filename) {
    return std::filesystem::exists(filename);
}

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " <upf_file>\n";
    std::cerr << "Read and process Universal Pseudopotential File (UPF)\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  upf_file   Path to the UPF file to process\n";
}

int main(int argc, char* argv[]) {
    // Check command line arguments each argument must be a filename
    if (argc == 1) {
        print_usage(argv[0]);
        return ERROR_INVALID_ARGS;
    }

    for (int i = 1; i < argc; ++i) {
        std::string upf_filename = argv[i];

        // Check if file exists
        if (!file_exists(upf_filename)) {
            std::cerr << "Error: File '" << upf_filename << "' not found\n";
            return ERROR_FILE_NOT_FOUND;
        }

        try {
            // Create UPF reader instance
            UPFReader reader(upf_filename);

            // Read and parse the UPF file
            if (!reader.parse()) {
                std::cerr << "Error: Failed to parse UPF file\n";
                return ERROR_XML_PARSE;
            }

            // Process and display the UPF data
            reader.display_info();

            // Get element name from global header
            std::string element = g_upf_header.element;

            // Create output directory for this element
            std::string output_dir = "gnuplot/" + element;

            // Export data using gnuplot exporter with element name
            GnuplotExporter exporter(output_dir, element);
            if (!exporter.export_all()) {
                std::cerr << "Error: Failed to export orbital data\n";
                return ERROR_FILE_WRITE;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return ERROR_FILE_READ;
        }
    }

    return SUCCESS;
}
