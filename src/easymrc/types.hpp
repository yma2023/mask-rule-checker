#pragma once

#include <vector>
#include <set>
#include <utility>
#include <cmath>
#include <algorithm>
#include <limits>

namespace easymrc {

// Basic Point structure
struct Point {
  int x_coord;
  int y_coord;

  Point() : x_coord(0), y_coord(0) {}
  Point(int x, int y) : x_coord(x), y_coord(y) {}

  int x() const { return x_coord; }
  int y() const { return y_coord; }
};

struct Segment {
  Point start;
  Point end;

  Segment() = default;
  Segment(const Point& s, const Point& e) : start(s), end(e) {}
  Segment(int x1, int y1, int x2, int y2)
      : start(Point(x1, y1)), end(Point(x2, y2)) {}

  bool is_vertical() const {
    return start.x() == end.x();
  }

  bool is_horizontal() const {
    return start.y() == end.y();
  }

  double length() const {
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    return std::sqrt(dx * dx + dy * dy);
  }

  int min_x() const { return std::min(start.x(), end.x()); }
  int max_x() const { return std::max(start.x(), end.x()); }
  int min_y() const { return std::min(start.y(), end.y()); }
  int max_y() const { return std::max(start.y(), end.y()); }
};

struct Polygon {
  int id;
  std::vector<Point> vertices;
  std::vector<Segment> segments;

  Polygon() : id(-1) {}
  explicit Polygon(int polygon_id) : id(polygon_id) {}

  void add_vertex(const Point& p) {
    vertices.push_back(p);
  }

  void add_segment(const Segment& s) {
    segments.push_back(s);
  }

  // Build segments from vertices (assumes vertices are in clockwise order)
  void build_segments() {
    segments.clear();
    if (vertices.size() < 2) return;

    for (size_t i = 0; i < vertices.size(); ++i) {
      size_t next = (i + 1) % vertices.size();
      segments.emplace_back(vertices[i], vertices[next]);
    }
  }
};

struct BoundingBox {
  double min_x, min_y, max_x, max_y;
  int polygon_id;

  BoundingBox()
      : min_x(0), min_y(0), max_x(0), max_y(0), polygon_id(-1) {}

  BoundingBox(double minx, double miny, double maxx, double maxy, int pid = -1)
      : min_x(minx), min_y(miny), max_x(maxx), max_y(maxy), polygon_id(pid) {}

  // Expand bounding box by a given distance R
  void expand(double R) {
    min_x -= R;
    min_y -= R;
    max_x += R;
    max_y += R;
  }

  // Check if this bounding box overlaps with another
  bool overlaps(const BoundingBox& other) const {
    return !(max_x < other.min_x || other.max_x < min_x ||
             max_y < other.min_y || other.max_y < min_y);
  }
};

// Compute bounding box from polygon
inline BoundingBox compute_bounding_box(const Polygon& poly) {
  if (poly.segments.empty()) {
    return BoundingBox();
  }

  BoundingBox bbox;
  bbox.min_x = bbox.max_x = poly.segments[0].start.x();
  bbox.min_y = bbox.max_y = poly.segments[0].start.y();
  bbox.polygon_id = poly.id;

  for (const auto& seg : poly.segments) {
    bbox.min_x = std::min(bbox.min_x, (double)seg.min_x());
    bbox.min_y = std::min(bbox.min_y, (double)seg.min_y());
    bbox.max_x = std::max(bbox.max_x, (double)seg.max_x());
    bbox.max_y = std::max(bbox.max_y, (double)seg.max_y());
  }

  return bbox;
}

// Representative point structure
struct RepresentativePoint {
  Point coordinates;
  std::vector<Point> shielded_vertices;
  std::vector<Segment> shielded_edges;
  int polygon_id;

  RepresentativePoint() : polygon_id(-1) {}
  RepresentativePoint(const Point& p, int pid)
      : coordinates(p), polygon_id(pid) {}
};

// Representative edge structure
struct RepresentativeEdge {
  Segment edge;
  std::vector<Point> shielded_vertices;
  int polygon_id;

  RepresentativeEdge() : polygon_id(-1) {}
  RepresentativeEdge(const Segment& e, int pid)
      : edge(e), polygon_id(pid) {}
};

// Violation structures
struct Violation {
  Point point1, point2;
  double distance;
  int polygon_id_1, polygon_id_2;

  Violation() : distance(0), polygon_id_1(-1), polygon_id_2(-1) {}

  Violation(const Point& p1, const Point& p2, double dist, int pid1, int pid2)
      : point1(p1), point2(p2), distance(dist),
        polygon_id_1(pid1), polygon_id_2(pid2) {}
};

struct ViolationTypeB {
  Point point;
  Segment edge;
  double distance;
  int polygon_id_1, polygon_id_2;

  ViolationTypeB() : distance(0), polygon_id_1(-1), polygon_id_2(-1) {}

  ViolationTypeB(const Point& p, const Segment& e, double dist,
                 int pid1, int pid2)
      : point(p), edge(e), distance(dist),
        polygon_id_1(pid1), polygon_id_2(pid2) {}
};

struct WidthViolation {
  Segment edge1, edge2;
  double distance;
  Point closest_point_on_edge1;
  Point closest_point_on_edge2;
  int polygon_id;

  WidthViolation() : distance(0), polygon_id(-1) {}

  WidthViolation(const Segment& e1, const Segment& e2, double dist,
                 const Point& p1, const Point& p2, int pid)
      : edge1(e1), edge2(e2), distance(dist),
        closest_point_on_edge1(p1), closest_point_on_edge2(p2),
        polygon_id(pid) {}
};

// Utility functions
inline double euclidean_distance(const Point& p1, const Point& p2) {
  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  return std::sqrt(dx * dx + dy * dy);
}

inline double point_to_segment_distance(const Point& p, const Segment& seg) {
  double x1 = seg.start.x(), y1 = seg.start.y();
  double x2 = seg.end.x(), y2 = seg.end.y();
  double px = p.x(), py = p.y();

  double dx = x2 - x1;
  double dy = y2 - y1;

  if (dx == 0 && dy == 0) {
    // Segment is a point
    return euclidean_distance(p, seg.start);
  }

  // Parameter t of the projection point on the line
  double t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);

  // Clamp t to [0, 1] to stay within segment
  t = std::max(0.0, std::min(1.0, t));

  // Closest point on segment
  double closest_x = x1 + t * dx;
  double closest_y = y1 + t * dy;

  double dist_x = px - closest_x;
  double dist_y = py - closest_y;

  return std::sqrt(dist_x * dist_x + dist_y * dist_y);
}

}  // namespace easymrc
