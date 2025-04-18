# UPF Routines

A C++ code for reading and analyzing Universal Pseudopotential File (UPF) data from Quantum ESPRESSO. This tool provides functionality to read UPF files, calculate various potentials, and generate publication-ready plots using Gnuplot.

## Features

- Read UPF files using pugiXML
- Calculate total potentials from local potentials and projectors
- Export data in a format suitable for plotting
- Generate Gnuplot scripts for visualization
- Support for multiple output formats (X11, PostScript color/mono)

## Dependencies

- C++17 or later
- CMake 3.10 or later
- pugiXML
- Gnuplot (for visualization)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

1. Place your executable where UPF files are like ina the `/UPF_data/nc-sr-05_pbe_standard_upf/` directory
2. Run the executable:
   ```bash
   ./Optical_properties UPF_file_1 UPF_file_2 ...
   ```
3. The program will generate:
   - Data files (.dat) containing potential values
   - Gnuplot scripts (.gp) for visualization
   - PostScript output files when running the scripts

## Output Files

- `element_local_potential.dat`: Local potential data
- `element_nonlocal_potentials.dat`: Non-local potentials
- `element_projectors.dat`: Projector functions
- `element_total_potentials.dat`: Total potentials
- Corresponding `.gp` files for plotting

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Authors

- Original implementation by [Nenad Sakan](https://github.com/nsakan)

## Acknowledgments

- Quantum ESPRESSO team for the UPF format specification
- pugiXML developers for the XML parsing library