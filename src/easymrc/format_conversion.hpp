#pragma once

#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include "types.hpp"

namespace easymrc {

// Simple image structure (replaces OpenCV for simplicity)
struct Image {
  int width;
  int height;
  std::vector<unsigned char> data;

  Image() : width(0), height(0) {}
  Image(int w, int h) : width(w), height(h), data(w * h, 0) {}

  unsigned char& at(int x, int y) {
    return data[y * width + x];
  }

  const unsigned char& at(int x, int y) const {
    return data[y * width + x];
  }

  bool is_mask_pixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;
    return at(x, y) == 255;
  }
};

// Read PGM (Portable GrayMap) format - simple text-based format
// This is a simplified alternative to PNG for testing
inline Image read_pgm(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open image file: " + filename);
  }

  std::string magic;
  file >> magic;

  if (magic != "P2" && magic != "P5") {
    throw std::runtime_error("Unsupported PGM format: " + magic);
  }

  int width, height, max_val;
  file >> width >> height >> max_val;

  Image img(width, height);

  if (magic == "P2") {
    // ASCII format
    for (int y = height - 1; y >= 0; --y) {  // Bottom-up
      for (int x = 0; x < width; ++x) {
        int val;
        file >> val;
        img.at(x, y) = static_cast<unsigned char>(val);
      }
    }
  } else {
    // Binary format
    file.get();  // Skip newline
    for (int y = height - 1; y >= 0; --y) {  // Bottom-up
      for (int x = 0; x < width; ++x) {
        img.at(x, y) = static_cast<unsigned char>(file.get());
      }
    }
  }

  return img;
}

// Direction enumeration for clockwise traversal
enum Direction { RIGHT = 0, DOWN = 1, LEFT = 2, UP = 3 };

// Direction vectors: right, down, left, up
const int dx[] = {1, 0, -1, 0};
const int dy[] = {0, -1, 0, 1};

class FormatConverter {
 public:
  FormatConverter(const Image& img) : image_(img) {
    visited_.resize(img.height, std::vector<bool>(img.width, false));
  }

  std::vector<Polygon> convert() {
    std::vector<Polygon> polygons;
    int polygon_id = 0;

    // Scan from bottom-left to top-right
    for (int y = 0; y < image_.height; ++y) {
      for (int x = 0; x < image_.width; ++x) {
        if (image_.is_mask_pixel(x, y) && !visited_[y][x]) {
          Polygon poly = trace_polygon(x, y, polygon_id++);
          if (!poly.segments.empty()) {
            polygons.push_back(poly);
          }
        }
      }
    }

    return polygons;
  }

 private:
  const Image& image_;
  std::vector<std::vector<bool>> visited_;

  Polygon trace_polygon(int start_x, int start_y, int polygon_id) {
    Polygon poly(polygon_id);

    int x = start_x;
    int y = start_y;
    Direction dir = RIGHT;  // Start going right

    Point start_corner(x, y);  // Bottom-left corner of pixel
    Point current_corner = start_corner;

    std::vector<Point> corners;
    corners.push_back(current_corner);

    // Mark starting pixel as visited
    visited_[y][x] = true;

    // Trace boundary clockwise
    bool first_iteration = true;
    Direction prev_dir = dir;

    do {
      // Try to continue in current direction
      int next_x = x + dx[dir];
      int next_y = y + dy[dir];

      if (image_.is_mask_pixel(next_x, next_y) &&
          (first_iteration || next_x != start_x || next_y != start_y ||
           corners.size() < 4)) {
        // Move to next pixel
        x = next_x;
        y = next_y;
        visited_[y][x] = true;

        // Update corner position based on direction
        current_corner = get_next_corner(current_corner, dir);

        // Try to turn right (clockwise)
        Direction new_dir = static_cast<Direction>((dir + 3) % 4);

        // Check if we can turn right
        int turn_x = x + dx[new_dir];
        int turn_y = y + dy[new_dir];

        if (image_.is_mask_pixel(turn_x, turn_y)) {
          // Record corner when direction changes
          if (new_dir != dir) {
            corners.push_back(current_corner);
          }
          dir = new_dir;
        } else if (prev_dir != dir) {
          // Direction changed, record corner
          corners.push_back(current_corner);
        }

        prev_dir = dir;
        first_iteration = false;

      } else {
        // Can't continue, turn left
        dir = static_cast<Direction>((dir + 1) % 4);
      }

      // Safety check to prevent infinite loop
      if (corners.size() > (size_t)(image_.width * image_.height)) {
        break;
      }

    } while (current_corner.x() != start_corner.x() ||
             current_corner.y() != start_corner.y() ||
             first_iteration);

    // Build polygon from corners
    for (size_t i = 0; i < corners.size(); ++i) {
      poly.vertices.push_back(corners[i]);
    }

    poly.build_segments();

    return poly;
  }

  Point get_next_corner(const Point& corner, Direction dir) {
    switch (dir) {
      case RIGHT:
        return Point(corner.x() + 1, corner.y());
      case DOWN:
        return Point(corner.x(), corner.y() - 1);
      case LEFT:
        return Point(corner.x() - 1, corner.y());
      case UP:
        return Point(corner.x(), corner.y() + 1);
      default:
        return corner;
    }
  }
};

// Main conversion function
inline std::vector<Polygon> format_conversion(const std::string& image_file) {
  Image img = read_pgm(image_file);
  FormatConverter converter(img);
  return converter.convert();
}

// Alternative: convert from raw pixel data
inline std::vector<Polygon> format_conversion_from_data(
    const std::vector<std::vector<unsigned char>>& pixel_data) {

  if (pixel_data.empty() || pixel_data[0].empty()) {
    return {};
  }

  int height = pixel_data.size();
  int width = pixel_data[0].size();

  Image img(width, height);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      img.at(x, y) = pixel_data[y][x];
    }
  }

  FormatConverter converter(img);
  return converter.convert();
}

}  // namespace easymrc
