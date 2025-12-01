#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>

#include "easymrc/easymrc.hpp"

using namespace easymrc;

void print_usage(const char* program_name) {
  std::cerr << "Usage: " << program_name << " <input_file> [options]\n";
  std::cerr << "\nOptions:\n";
  std::cerr << "  -r <distance>    Rule distance (default: 50.0)\n";
  std::cerr << "  -m <multiplier>  Sampling radius multiplier (default: 4.0)\n";
  std::cerr << "  -t <threads>     Number of threads (default: auto)\n";
  std::cerr << "  -o <output>      Output violations file (default: violations.json)\n";
  std::cerr << "  --no-space       Disable space checking\n";
  std::cerr << "  --no-width       Disable width checking\n";
  std::cerr << "  --no-parallel    Disable parallel execution\n";
  std::cerr << "\nSupported formats: PGM, PNG (future), PPM (future)\n";
  std::cerr << "\nExamples:\n";
  std::cerr << "  " << program_name << " mask.pgm -r 50 -t 8\n";
  std::cerr << "  " << program_name << " test_pattern.pgm -r 100 -m 4 -o results.json\n";
}

void write_json_output(const std::string& filename,
                      const EasyMRC::Results& results,
                      double execution_time_ms) {
  std::ofstream out(filename);
  if (!out.is_open()) {
    throw std::runtime_error("Cannot open output file: " + filename);
  }

  out << "{\n";
  out << "  \"execution_time_ms\": " << execution_time_ms << ",\n";
  out << "  \"space_violations\": {\n";
  out << "    \"type_a\": [\n";

  for (size_t i = 0; i < results.space_violations_type_a.size(); ++i) {
    const auto& vio = results.space_violations_type_a[i];
    out << "      {\n";
    out << "        \"point1\": [" << vio.point1.x() << ", "
        << vio.point1.y() << "],\n";
    out << "        \"point2\": [" << vio.point2.x() << ", "
        << vio.point2.y() << "],\n";
    out << "        \"distance\": " << vio.distance << ",\n";
    out << "        \"polygon_id_1\": " << vio.polygon_id_1 << ",\n";
    out << "        \"polygon_id_2\": " << vio.polygon_id_2 << "\n";
    out << "      }";
    if (i < results.space_violations_type_a.size() - 1) out << ",";
    out << "\n";
  }

  out << "    ],\n";
  out << "    \"type_b\": [\n";

  for (size_t i = 0; i < results.space_violations_type_b.size(); ++i) {
    const auto& vio = results.space_violations_type_b[i];
    out << "      {\n";
    out << "        \"point\": [" << vio.point.x() << ", "
        << vio.point.y() << "],\n";
    out << "        \"edge\": [[" << vio.edge.start.x() << ", "
        << vio.edge.start.y() << "], [" << vio.edge.end.x() << ", "
        << vio.edge.end.y() << "]],\n";
    out << "        \"distance\": " << vio.distance << ",\n";
    out << "        \"polygon_id_1\": " << vio.polygon_id_1 << ",\n";
    out << "        \"polygon_id_2\": " << vio.polygon_id_2 << "\n";
    out << "      }";
    if (i < results.space_violations_type_b.size() - 1) out << ",";
    out << "\n";
  }

  out << "    ]\n";
  out << "  },\n";
  out << "  \"width_violations\": [\n";

  for (size_t i = 0; i < results.width_violations.size(); ++i) {
    const auto& vio = results.width_violations[i];
    out << "    {\n";
    out << "      \"edge1\": [[" << vio.edge1.start.x() << ", "
        << vio.edge1.start.y() << "], [" << vio.edge1.end.x() << ", "
        << vio.edge1.end.y() << "]],\n";
    out << "      \"edge2\": [[" << vio.edge2.start.x() << ", "
        << vio.edge2.start.y() << "], [" << vio.edge2.end.x() << ", "
        << vio.edge2.end.y() << "]],\n";
    out << "      \"distance\": " << vio.distance << ",\n";
    out << "      \"polygon_id\": " << vio.polygon_id << "\n";
    out << "    }";
    if (i < results.width_violations.size() - 1) out << ",";
    out << "\n";
  }

  out << "  ],\n";
  out << "  \"summary\": {\n";
  out << "    \"total_space_violations\": "
      << results.total_space_violations() << ",\n";
  out << "    \"total_width_violations\": "
      << results.width_violations.size() << ",\n";
  out << "    \"total_violations\": " << results.total_violations() << "\n";
  out << "  }\n";
  out << "}\n";

  out.close();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  std::string input_file = argv[1];
  std::string output_file = "violations.json";

  EasyMRC::Config config;

  // Parse command line arguments
  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-r" && i + 1 < argc) {
      config.rule_distance_R = std::stod(argv[++i]);
    } else if (arg == "-m" && i + 1 < argc) {
      config.sampling_radius_multiplier = std::stod(argv[++i]);
    } else if (arg == "-t" && i + 1 < argc) {
      config.num_threads = std::stoi(argv[++i]);
    } else if (arg == "-o" && i + 1 < argc) {
      output_file = argv[++i];
    } else if (arg == "--no-space") {
      config.enable_space_check = false;
    } else if (arg == "--no-width") {
      config.enable_width_check = false;
    } else if (arg == "--no-parallel") {
      config.enable_parallel = false;
    } else {
      std::cerr << "Unknown option: " << arg << std::endl;
      print_usage(argv[0]);
      return 1;
    }
  }

  // Verify file type
  std::string ext = input_file.substr(input_file.find_last_of(".") + 1);
  if (ext != "pgm" && ext != "png" && ext != "ppm") {
    std::cerr << "Error: Unsupported file type '" << ext << "'. "
              << "Supported formats: pgm, png, ppm\n";
    return 1;
  }

  try {
    std::cout << "========================================\n";
    std::cout << "EasyMRC - Efficient Mask Rule Checking\n";
    std::cout << "========================================\n\n";

    std::cout << "Configuration:\n";
    std::cout << "  Input file: " << input_file << "\n";
    std::cout << "  Rule distance: " << config.rule_distance_R << "\n";
    std::cout << "  Sampling multiplier: "
              << config.sampling_radius_multiplier << "\n";
    std::cout << "  Threads: ";
    if (config.num_threads == 0) {
      std::cout << "auto (" << std::thread::hardware_concurrency() << ")\n";
    } else {
      std::cout << config.num_threads << "\n";
    }
    std::cout << "  Space check: "
              << (config.enable_space_check ? "enabled" : "disabled") << "\n";
    std::cout << "  Width check: "
              << (config.enable_width_check ? "enabled" : "disabled") << "\n";
    std::cout << "  Parallel: "
              << (config.enable_parallel ? "enabled" : "disabled") << "\n\n";

    // Load polygons from image
    std::cout << "Loading image file...\n";
    std::vector<Polygon> polygons = format_conversion(input_file);
    std::cout << "  Polygons extracted: " << polygons.size() << "\n\n";

    // Run EasyMRC
    std::cout << "Running EasyMRC...\n";

    EasyMRC checker(config);

    auto start = std::chrono::high_resolution_clock::now();
    auto results = checker.run(polygons);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);

    std::cout << "\nResults:\n";
    std::cout << "  Execution time: " << duration.count() << " ms\n";
    std::cout << "  Space violations (type a): "
              << results.space_violations_type_a.size() << "\n";
    std::cout << "  Space violations (type b): "
              << results.space_violations_type_b.size() << "\n";
    std::cout << "  Width violations: "
              << results.width_violations.size() << "\n";
    std::cout << "  Total violations: " << results.total_violations() << "\n\n";

    // Write output
    std::cout << "Writing violations to: " << output_file << "\n";
    write_json_output(output_file, results, duration.count());

    std::cout << "\n========================================\n";
    std::cout << "EasyMRC completed successfully!\n";
    std::cout << "========================================\n";

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "\nError: " << e.what() << std::endl;
    return 1;
  }
}
