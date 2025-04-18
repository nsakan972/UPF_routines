#ifndef MAIN_HPP
#define MAIN_HPP

#include <string>
#include "../UPF_reader/UPF_reader.hpp"
#include <iostream>
#include <filesystem>
#include "../output/gnuplot_exporter.hpp"

// Program exit codes
enum ExitCode {
    SUCCESS = 0,
    ERROR_INVALID_ARGS = 1,
    ERROR_FILE_NOT_FOUND = 2,
    ERROR_FILE_READ = 3,
    ERROR_XML_PARSE = 4,
    ERROR_FILE_WRITE = 5
};

// Utility functions
bool file_exists(const std::string& filename);
void print_usage(const char* program_name);

#endif // MAIN_HPP
