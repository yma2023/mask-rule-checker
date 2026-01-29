#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include "easymrc/easymrc.hpp"

using namespace easymrc;

// 空白の除去
std::string trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

// 設定ファイル読み込み
EasyMRC::Config load_rule_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open rule file: " + filename);
  }

  EasyMRC::Config config;
  std::string line;
  int line_number = 0;

  while (std::getline(file, line)) {
    line_number++;

    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }

    line = trim(line);

    if (line.empty()) continue;

    // key: value形式を解析
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
      std::cerr << "Warning: Invalid format at line " << line_number
                << " (expected 'key: value'): " << line << std::endl;
      continue;
    }

    std::string key = trim(line.substr(0, colon_pos));
    std::string value = trim(line.substr(colon_pos + 1));

    // 設定項目ごとに代入
    if (key == "rule_distance") {
        config.rule_distance_R = std::stod(value);

    } else if (key == "sampling_multiplier") {
        config.sampling_radius_multiplier = std::stod(value);
        
    } else if (key == "threads") {
        if (value == "auto" || value == "0") {
            config.num_threads = 0;
        } else {
            config.num_threads = std::stoi(value);
        }

    } else if (key == "space_check") {
        config.enable_space_check = (value == "true" || value == "1");

    } else if (key == "width_check") {
        config.enable_width_check = (value == "true" || value == "1");

    } else if (key == "parallel") {
        config.enable_parallel = (value == "true" || value == "1");

    } else {
        std::cerr << "Warning: Unknown parameter '" << key
                  << "' at line " << line_number << std::endl;
    }

  }

  file.close();
  return config;
}

// 使用方法の表示
void print_usage(const char* program_name) {
  std::cerr << "Usage: " << program_name << " <input_file> <output_file> <rule_file>\n";
  std::cerr << "\nArguments:\n";
  std::cerr << "  input_file      Input image file (PGM, PNG, or PPM format)\n";
  std::cerr << "  output_file     Output violations file (JSON format)\n";
  std::cerr << "  rule_file       Rule configuration file\n";
  std::cerr << "\nRule file format:\n";
  std::cerr << "  # Comment line\n";
  std::cerr << "  rule_distance: 50.0\n";
  std::cerr << "  sampling_multiplier: 4.0\n";
  std::cerr << "  threads: 8  # or 'auto'\n";
  std::cerr << "  space_check: true\n";
  std::cerr << "  width_check: true\n";
  std::cerr << "  parallel: true\n";
  std::cerr << "\nExamples:\n";
  std::cerr << "  " << program_name << " mask.pgm violations.json rules.txt\n";
  std::cerr << "  " << program_name << " test_pattern.pgm results.json my_rules.txt\n";
}

// JSONログ出力
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
  // 引数チェック
  if (argc != 4) {
    std::cerr << "Error: Expected exactly 3 arguments, got " << (argc - 1) << std::endl;
    print_usage(argv[0]);
    return 1;
  }

  std::string input_file = argv[1];
  std::string output_file = argv[2];
  std::string rule_file = argv[3];

  // 設定ファイル読み込み
  EasyMRC::Config config;
  try {
    config = load_rule_file(rule_file);
  } catch (const std::exception& e) {
    std::cerr << "Error loading rule file: " << e.what() << std::endl;
    return 1;
  }

  // 入力ファイルの拡張子チェック
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

    // 画像読み込み → ポリゴン抽出
    std::cout << "Loading image file...\n";
    std::vector<Polygon> polygons = format_conversion(input_file);
    std::cout << "  Polygons extracted: " << polygons.size() << "\n\n";

    // MRC を実行
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

    // JSONファイルへ出力
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
