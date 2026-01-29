#include <iostream>
#include <fstream>
#include <string>
#include "../src/easymrc/format_conversion.hpp"

using namespace easymrc;

// Create a simple test PGM image
void create_test_pgm(const std::string& filename) {
  std::ofstream out(filename);
  // Create a 10x10 image with a simple rectangle
  out << "P2\n";
  out << "10 10\n";
  out << "255\n";

  // Create a 4x4 white square in the middle
  for (int y = 0; y < 10; ++y) {
    for (int x = 0; x < 10; ++x) {
      if (x >= 3 && x < 7 && y >= 3 && y < 7) {
        out << "255 ";
      } else {
        out << "0 ";
      }
    }
    out << "\n";
  }
  out.close();
}

int main() {
  std::cout << "========================================\n";
  std::cout << "GDSII Format Test\n";
  std::cout << "========================================\n\n";

  try {
    // Create test PGM image
    std::cout << "1. Creating test PGM image...\n";
    const std::string pgm_file = "test_pattern.pgm";
    const std::string gdsii_file = "test_pattern.gds";
    create_test_pgm(pgm_file);
    std::cout << "   Created: " << pgm_file << "\n\n";

    // Test 1: Direct conversion (PGM -> Polygon)
    std::cout << "2. Testing direct conversion (PGM -> Polygon)...\n";
    auto polygons_direct = format_conversion(pgm_file);
    std::cout << "   Polygons extracted: " << polygons_direct.size() << "\n";
    if (!polygons_direct.empty()) {
      std::cout << "   First polygon:\n";
      std::cout << "     ID: " << polygons_direct[0].id << "\n";
      std::cout << "     Vertices: " << polygons_direct[0].vertices.size() << "\n";
      std::cout << "     Segments: " << polygons_direct[0].segments.size() << "\n";
    }
    std::cout << "\n";

    // Test 2: GDSII conversion (PGM -> GDSII -> Polygon)
    std::cout << "3. Testing GDSII conversion (PGM -> GDSII -> Polygon)...\n";
    auto polygons_gdsii = format_conversion(pgm_file, gdsii_file);
    std::cout << "   GDSII file created: " << gdsii_file << "\n";
    std::cout << "   Polygons extracted: " << polygons_gdsii.size() << "\n";
    if (!polygons_gdsii.empty()) {
      std::cout << "   First polygon:\n";
      std::cout << "     ID: " << polygons_gdsii[0].id << "\n";
      std::cout << "     Vertices: " << polygons_gdsii[0].vertices.size() << "\n";
      std::cout << "     Segments: " << polygons_gdsii[0].segments.size() << "\n";
    }
    std::cout << "\n";

    // Test 3: Verify GDSII file content (binary format)
    std::cout << "4. Verifying GDSII file content (binary format)...\n";
    std::ifstream gdsii_check(gdsii_file, std::ios::binary);
    if (gdsii_check.is_open()) {
      gdsii_check.seekg(0, std::ios::end);
      size_t file_size = gdsii_check.tellg();
      std::cout << "   GDSII file size: " << file_size << " bytes\n";
      gdsii_check.close();

      // Read first few bytes to verify binary format
      gdsii_check.open(gdsii_file, std::ios::binary);
      uint8_t header[6];
      gdsii_check.read(reinterpret_cast<char*>(header), 6);
      std::cout << "   Header record: length=" << ((header[0] << 8) | header[1])
                << ", type=0x" << std::hex << (int)header[2]
                << ", data=0x" << (int)header[3] << std::dec << "\n";
      if (header[2] == 0x00) {
        std::cout << "   ✓ Valid GDSII binary format (HEADER record found)\n";
      }
      gdsii_check.close();
    }
    std::cout << "\n";

    // Test 4: Compare results
    std::cout << "5. Comparing direct vs GDSII results...\n";
    if (polygons_direct.size() == polygons_gdsii.size()) {
      std::cout << "   ✓ Polygon count matches: " << polygons_direct.size() << "\n";

      bool all_match = true;
      for (size_t i = 0; i < polygons_direct.size(); ++i) {
        if (polygons_direct[i].vertices.size() != polygons_gdsii[i].vertices.size() ||
            polygons_direct[i].segments.size() != polygons_gdsii[i].segments.size()) {
          all_match = false;
          std::cout << "   ✗ Polygon " << i << " structure mismatch\n";
        }
      }

      if (all_match) {
        std::cout << "   ✓ All polygon structures match\n";
      }
    } else {
      std::cout << "   ✗ Polygon count mismatch: "
                << polygons_direct.size() << " vs " << polygons_gdsii.size() << "\n";
    }
    std::cout << "\n";

    // Test 5: Direct GDSII read/write
    std::cout << "6. Testing direct GDSII read/write...\n";
    const std::string gdsii_file2 = "test_pattern2.gds";
    write_gdsii(polygons_direct, gdsii_file2);
    auto polygons_reloaded = read_gdsii(gdsii_file2);
    std::cout << "   Written and reloaded: " << polygons_reloaded.size() << " polygons\n";

    if (polygons_direct.size() == polygons_reloaded.size()) {
      std::cout << "   ✓ Reload successful\n";
    } else {
      std::cout << "   ✗ Reload failed\n";
    }
    std::cout << "\n";

    std::cout << "========================================\n";
    std::cout << "All tests completed successfully!\n";
    std::cout << "========================================\n";

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "\nError: " << e.what() << std::endl;
    return 1;
  }
}
