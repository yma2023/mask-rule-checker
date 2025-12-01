#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Generate PGM test images for EasyMRC testing

void create_pgm(const std::string& filename,
                const std::vector<std::vector<int>>& pattern) {
  int height = pattern.size();
  if (height == 0) return;
  int width = pattern[0].size();

  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Cannot create file: " << filename << std::endl;
    return;
  }

  file << "P2\n";
  file << width << " " << height << "\n";
  file << "255\n";

  // Write bottom-up (PGM is top-down, but we want bottom-up origin)
  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      file << pattern[y][x];
      if (x < width - 1) file << " ";
    }
    file << "\n";
  }

  file.close();
  std::cout << "Created: " << filename << " (" << width << "x" << height << ")"
            << std::endl;
}

void generate_test_rectangle() {
  std::cout << "\nGenerating test_rectangle.pgm..." << std::endl;

  // Simple 20x20 image with 10x10 rectangle
  std::vector<std::vector<int>> pattern(20, std::vector<int>(20, 0));

  // Fill a 10x10 rectangle at position (5,5)
  for (int y = 5; y < 15; ++y) {
    for (int x = 5; x < 15; ++x) {
      pattern[y][x] = 255;
    }
  }

  create_pgm("test_rectangle.pgm", pattern);
  std::cout << "  Expected: 1 polygon, 4 vertices, 4 segments" << std::endl;
}

void generate_test_l_shape() {
  std::cout << "\nGenerating test_l_shape.pgm..." << std::endl;

  // 25x25 image with L-shaped pattern
  std::vector<std::vector<int>> pattern(25, std::vector<int>(25, 0));

  // Vertical part of L: (5,5) to (10,20)
  for (int y = 5; y < 20; ++y) {
    for (int x = 5; x < 10; ++x) {
      pattern[y][x] = 255;
    }
  }

  // Horizontal part of L: (5,5) to (20,10)
  for (int y = 5; y < 10; ++y) {
    for (int x = 5; x < 20; ++x) {
      pattern[y][x] = 255;
    }
  }

  create_pgm("test_l_shape.pgm", pattern);
  std::cout << "  Expected: 1 polygon, 6 vertices" << std::endl;
}

void generate_test_two_rectangles() {
  std::cout << "\nGenerating test_two_rectangles.pgm..." << std::endl;

  // 30x20 image with two separated rectangles
  std::vector<std::vector<int>> pattern(20, std::vector<int>(30, 0));

  // Rectangle 1: (2,2) to (10,10)
  for (int y = 2; y < 10; ++y) {
    for (int x = 2; x < 10; ++x) {
      pattern[y][x] = 255;
    }
  }

  // Rectangle 2: (20,2) to (28,10)
  for (int y = 2; y < 10; ++y) {
    for (int x = 20; x < 28; ++x) {
      pattern[y][x] = 255;
    }
  }

  create_pgm("test_two_rectangles.pgm", pattern);
  std::cout << "  Expected: 2 polygons" << std::endl;
}

void generate_test_close_rectangles() {
  std::cout << "\nGenerating test_close_rectangles.pgm..." << std::endl;

  // Two rectangles with small spacing (for space violation test)
  std::vector<std::vector<int>> pattern(20, std::vector<int>(30, 0));

  // Rectangle 1: (2,2) to (10,18)
  for (int y = 2; y < 18; ++y) {
    for (int x = 2; x < 10; ++x) {
      pattern[y][x] = 255;
    }
  }

  // Rectangle 2: (13,2) to (21,18) - only 3 units away
  for (int y = 2; y < 18; ++y) {
    for (int x = 13; x < 21; ++x) {
      pattern[y][x] = 255;
    }
  }

  create_pgm("test_close_rectangles.pgm", pattern);
  std::cout << "  Expected: Space violation with R >= 4" << std::endl;
}

void generate_test_thin_stripe() {
  std::cout << "\nGenerating test_thin_stripe.pgm..." << std::endl;

  // Thin horizontal stripe (for width violation test)
  std::vector<std::vector<int>> pattern(20, std::vector<int>(50, 0));

  // Thin stripe: height = 2, width = 40
  for (int y = 9; y < 11; ++y) {
    for (int x = 5; x < 45; ++x) {
      pattern[y][x] = 255;
    }
  }

  create_pgm("test_thin_stripe.pgm", pattern);
  std::cout << "  Expected: Width violation with R >= 3" << std::endl;
}

void generate_test_complex_pattern() {
  std::cout << "\nGenerating test_complex_pattern.pgm..." << std::endl;

  // More complex pattern with multiple features
  std::vector<std::vector<int>> pattern(50, std::vector<int>(50, 0));

  // Feature 1: Large square
  for (int y = 5; y < 20; ++y) {
    for (int x = 5; x < 20; ++x) {
      pattern[y][x] = 255;
    }
  }

  // Feature 2: Small square nearby
  for (int y = 5; y < 12; ++y) {
    for (int x = 23; x < 30; ++x) {
      pattern[y][x] = 255;
    }
  }

  // Feature 3: Thin line
  for (int y = 30; y < 32; ++y) {
    for (int x = 5; x < 40; ++x) {
      pattern[y][x] = 255;
    }
  }

  // Feature 4: T-shape
  for (int y = 40; y < 45; ++y) {
    for (int x = 20; x < 30; ++x) {
      pattern[y][x] = 255;
    }
  }
  for (int y = 35; y < 45; ++y) {
    for (int x = 23; x < 27; ++x) {
      pattern[y][x] = 255;
    }
  }

  create_pgm("test_complex_pattern.pgm", pattern);
  std::cout << "  Expected: Multiple polygons with various violations" << std::endl;
}

int main() {
  std::cout << "========================================" << std::endl;
  std::cout << "EasyMRC Test Data Generator" << std::endl;
  std::cout << "========================================" << std::endl;

  generate_test_rectangle();
  generate_test_l_shape();
  generate_test_two_rectangles();
  generate_test_close_rectangles();
  generate_test_thin_stripe();
  generate_test_complex_pattern();

  std::cout << "\n========================================" << std::endl;
  std::cout << "Test data generation completed!" << std::endl;
  std::cout << "========================================" << std::endl;

  return 0;
}
