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

// GDSII Binary Format Support
// Reference: gdsii_to_text.cpp

namespace gdsii {
  // Record types
  enum RecordType : uint8_t {
    HEADER    = 0x00,
    BGNLIB    = 0x01,
    LIBNAME   = 0x02,
    UNITS     = 0x03,
    ENDLIB    = 0x04,
    BGNSTR    = 0x05,
    STRNAME   = 0x06,
    ENDSTR    = 0x07,
    BOUNDARY  = 0x08,
    LAYER     = 0x0D,
    DATATYPE  = 0x0E,
    XY        = 0x10,
    ENDEL     = 0x11
  };

  // Data types
  enum DataType : uint8_t {
    NO_DATA   = 0x00,
    BIT_ARRAY = 0x01,
    INT16     = 0x02,
    INT32     = 0x03,
    REAL64    = 0x05,
    ASCII     = 0x06
  };

  // Helper functions to write GDSII records
  inline void write_record_header(std::ofstream& file, uint16_t length,
                                   uint8_t rtype, uint8_t dtype) {
    uint8_t header[4];
    header[0] = (length >> 8) & 0xFF;
    header[1] = length & 0xFF;
    header[2] = rtype;
    header[3] = dtype;
    file.write(reinterpret_cast<char*>(header), 4);
  }

  inline void write_int16(std::ofstream& file, int16_t value) {
    uint8_t bytes[2];
    bytes[0] = (value >> 8) & 0xFF;
    bytes[1] = value & 0xFF;
    file.write(reinterpret_cast<char*>(bytes), 2);
  }

  inline void write_int32(std::ofstream& file, int32_t value) {
    uint8_t bytes[4];
    bytes[0] = (value >> 24) & 0xFF;
    bytes[1] = (value >> 16) & 0xFF;
    bytes[2] = (value >> 8) & 0xFF;
    bytes[3] = value & 0xFF;
    file.write(reinterpret_cast<char*>(bytes), 4);
  }

  inline void write_string(std::ofstream& file, const std::string& str) {
    file.write(str.c_str(), str.length());
    if (str.length() % 2 == 1) {
      file.put('\0');  // Padding for odd-length strings
    }
  }

  inline void write_real64(std::ofstream& file, double value) {
    // GDSII uses a special 64-bit floating point format
    // For simplicity, we'll just store 0.0 for now
    // A full implementation would convert IEEE 754 to GDSII format
    uint8_t bytes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (value != 0.0) {
      // Simple conversion (not fully accurate for all values)
      int sign = value < 0 ? 1 : 0;
      value = std::abs(value);
      int exponent = 64;  // Base exponent
      while (value >= 1.0 && exponent < 127) {
        value /= 16.0;
        exponent++;
      }
      while (value < 0.0625 && exponent > 0) {
        value *= 16.0;
        exponent--;
      }
      uint64_t mantissa = static_cast<uint64_t>(value * (1ULL << 56));
      uint64_t result = (static_cast<uint64_t>(sign) << 63) |
                        (static_cast<uint64_t>(exponent) << 56) |
                        (mantissa & 0x00FFFFFFFFFFFFFFULL);
      for (int i = 0; i < 8; ++i) {
        bytes[i] = (result >> (56 - i * 8)) & 0xFF;
      }
    }
    file.write(reinterpret_cast<char*>(bytes), 8);
  }

  inline int16_t read_int16(std::ifstream& file) {
    uint8_t bytes[2];
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (static_cast<int16_t>(bytes[0]) << 8) | static_cast<int16_t>(bytes[1]);
  }

  inline int32_t read_int32(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return (static_cast<int32_t>(bytes[0]) << 24) |
           (static_cast<int32_t>(bytes[1]) << 16) |
           (static_cast<int32_t>(bytes[2]) << 8) |
           static_cast<int32_t>(bytes[3]);
  }

  inline std::string read_string(std::ifstream& file, int length) {
    std::vector<char> buffer(length);
    file.read(buffer.data(), length);
    std::string str(buffer.begin(), buffer.end());
    // Remove null padding
    size_t pos = str.find('\0');
    if (pos != std::string::npos) {
      str = str.substr(0, pos);
    }
    return str;
  }
}

// Write polygons to GDSII binary format
inline void write_gdsii(const std::vector<Polygon>& polygons,
                        const std::string& filename) {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open GDSII file for writing: " + filename);
  }

  // HEADER record
  gdsii::write_record_header(file, 6, gdsii::HEADER, gdsii::INT16);
  gdsii::write_int16(file, 600);  // Version 6.0.0

  // BGNLIB record (12 modification/access times, all zeros for simplicity)
  gdsii::write_record_header(file, 28, gdsii::BGNLIB, gdsii::INT16);
  for (int i = 0; i < 12; ++i) {
    gdsii::write_int16(file, 0);
  }

  // LIBNAME record
  std::string libname = "EASYMRC_LIB";
  int libname_len = libname.length();
  if (libname_len % 2 == 1) libname_len++;  // Padding
  gdsii::write_record_header(file, 4 + libname_len, gdsii::LIBNAME, gdsii::ASCII);
  gdsii::write_string(file, libname);

  // UNITS record (database unit and user unit)
  gdsii::write_record_header(file, 20, gdsii::UNITS, gdsii::REAL64);
  gdsii::write_real64(file, 0.001);  // User unit: 0.001 (1 unit = 1nm if meter-based)
  gdsii::write_real64(file, 1e-9);   // Database unit in meters: 1nm

  // Write each polygon as a separate structure
  for (const auto& poly : polygons) {
    // BGNSTR record
    gdsii::write_record_header(file, 28, gdsii::BGNSTR, gdsii::INT16);
    for (int i = 0; i < 12; ++i) {
      gdsii::write_int16(file, 0);
    }

    // STRNAME record
    std::string strname = "POLY_" + std::to_string(poly.id);
    int strname_len = strname.length();
    if (strname_len % 2 == 1) strname_len++;  // Padding
    gdsii::write_record_header(file, 4 + strname_len, gdsii::STRNAME, gdsii::ASCII);
    gdsii::write_string(file, strname);

    // BOUNDARY record
    gdsii::write_record_header(file, 4, gdsii::BOUNDARY, gdsii::NO_DATA);

    // LAYER record
    gdsii::write_record_header(file, 6, gdsii::LAYER, gdsii::INT16);
    gdsii::write_int16(file, 0);  // Layer 0

    // DATATYPE record
    gdsii::write_record_header(file, 6, gdsii::DATATYPE, gdsii::INT16);
    gdsii::write_int16(file, 0);  // Datatype 0

    // XY record (coordinates)
    // GDSII requires polygons to be closed (first point == last point)
    int num_points = poly.vertices.size();
    bool needs_closing = (num_points > 0 &&
                          (poly.vertices[0].x() != poly.vertices[num_points-1].x() ||
                           poly.vertices[0].y() != poly.vertices[num_points-1].y()));
    int total_points = needs_closing ? num_points + 1 : num_points;

    gdsii::write_record_header(file, 4 + total_points * 8, gdsii::XY, gdsii::INT32);
    for (const auto& vertex : poly.vertices) {
      gdsii::write_int32(file, vertex.x());
      gdsii::write_int32(file, vertex.y());
    }
    if (needs_closing) {
      gdsii::write_int32(file, poly.vertices[0].x());
      gdsii::write_int32(file, poly.vertices[0].y());
    }

    // ENDEL record
    gdsii::write_record_header(file, 4, gdsii::ENDEL, gdsii::NO_DATA);

    // ENDSTR record
    gdsii::write_record_header(file, 4, gdsii::ENDSTR, gdsii::NO_DATA);
  }

  // ENDLIB record
  gdsii::write_record_header(file, 4, gdsii::ENDLIB, gdsii::NO_DATA);

  file.close();
}

// Read polygons from GDSII binary format
inline std::vector<Polygon> read_gdsii(const std::string& filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open GDSII file for reading: " + filename);
  }

  std::vector<Polygon> polygons;
  int current_polygon_id = 0;
  Polygon current_poly;
  bool in_boundary = false;
  std::vector<Point> current_vertices;

  while (file.peek() != EOF) {
    // Read record header
    uint8_t header[4];
    file.read(reinterpret_cast<char*>(header), 4);
    if (file.gcount() < 4) break;

    uint16_t record_length = (static_cast<uint16_t>(header[0]) << 8) |
                             static_cast<uint16_t>(header[1]);
    uint8_t rtype = header[2];
    // uint8_t dtype = header[3];  // Not used in current implementation

    int payload_size = record_length - 4;

    // Process record based on type
    switch (rtype) {
      case gdsii::BGNSTR: {
        // Start of structure - skip timestamp data
        file.seekg(payload_size, std::ios::cur);
        current_poly = Polygon(current_polygon_id++);
        current_vertices.clear();
        break;
      }

      case gdsii::STRNAME: {
        // Structure name - skip for now
        file.seekg(payload_size, std::ios::cur);
        break;
      }

      case gdsii::BOUNDARY: {
        // Start of boundary element
        in_boundary = true;
        current_vertices.clear();
        break;
      }

      case gdsii::LAYER:
      case gdsii::DATATYPE: {
        // Layer/datatype - skip
        file.seekg(payload_size, std::ios::cur);
        break;
      }

      case gdsii::XY: {
        // Read coordinates
        int num_points = payload_size / 8;
        for (int i = 0; i < num_points; ++i) {
          int32_t x = gdsii::read_int32(file);
          int32_t y = gdsii::read_int32(file);
          current_vertices.push_back(Point(x, y));
        }
        break;
      }

      case gdsii::ENDEL: {
        // End of element
        if (in_boundary && !current_vertices.empty()) {
          // Remove the last point if it's a duplicate of the first (GDSII closing)
          if (current_vertices.size() > 1) {
            const Point& first = current_vertices[0];
            const Point& last = current_vertices[current_vertices.size() - 1];
            if (first.x() == last.x() && first.y() == last.y()) {
              current_vertices.pop_back();
            }
          }

          // Add vertices to polygon
          for (const auto& vertex : current_vertices) {
            current_poly.add_vertex(vertex);
          }

          // Build segments from vertices
          current_poly.build_segments();

          current_vertices.clear();
          in_boundary = false;
        }
        break;
      }

      case gdsii::ENDSTR: {
        // End of structure
        if (!current_poly.vertices.empty()) {
          polygons.push_back(current_poly);
        }
        break;
      }

      case gdsii::ENDLIB: {
        // End of library
        break;
      }

      default: {
        // Skip unknown records
        file.seekg(payload_size, std::ios::cur);
        break;
      }
    }
  }

  file.close();
  return polygons;
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

  // Convert PGM image to polygons
  // If gdsii_filename is provided, the flow is: PGM -> GDSII (save) -> Polygon
  // Otherwise, direct conversion: PGM -> Polygon
  std::vector<Polygon> convert(const std::string& gdsii_filename = "") {
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

    // If GDSII filename is provided, write to GDSII and read back
    if (!gdsii_filename.empty()) {
      write_gdsii(polygons, gdsii_filename);
      polygons = read_gdsii(gdsii_filename);
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
// If gdsii_filename is provided, the flow is: PGM -> GDSII (save) -> Polygon
// Otherwise, direct conversion: PGM -> Polygon
inline std::vector<Polygon> format_conversion(
    const std::string& image_file,
    const std::string& gdsii_filename = "") {
  Image img = read_pgm(image_file);
  FormatConverter converter(img);
  return converter.convert(gdsii_filename);
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
