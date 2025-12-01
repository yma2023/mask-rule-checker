#pragma once

#include <vector>
#include <algorithm>
#include "types.hpp"
#include "type_a_violations.hpp"
#include "type_b_violations.hpp"

namespace easymrc {

// Edge direction utilities
inline bool is_edge_upward(const Segment& edge) {
  return edge.start.y() < edge.end.y();
}

inline bool is_edge_downward(const Segment& edge) {
  return edge.start.y() > edge.end.y();
}

inline bool is_edge_rightward(const Segment& edge) {
  return edge.start.x() < edge.end.x();
}

inline bool is_edge_leftward(const Segment& edge) {
  return edge.start.x() > edge.end.x();
}

// Check if two vertical edges are opposite
inline bool are_opposite_vertical(const Segment& e1, const Segment& e2) {
  if (!e1.is_vertical() || !e2.is_vertical()) return false;

  bool e1_up = is_edge_upward(e1);
  bool e2_up = is_edge_upward(e2);

  // Opposite means one is upward, the other is downward
  return e1_up != e2_up;
}

// Check if two horizontal edges are opposite
inline bool are_opposite_horizontal(const Segment& e1, const Segment& e2) {
  if (!e1.is_horizontal() || !e2.is_horizontal()) return false;

  bool e1_right = is_edge_rightward(e1);
  bool e2_right = is_edge_rightward(e2);

  // Opposite means one is rightward, the other is leftward
  return e1_right != e2_right;
}

// Check if two edges are opposite (both vertical or both horizontal,
// and pointing in opposite directions)
inline bool are_opposite(const Segment& e1, const Segment& e2) {
  if (e1.is_vertical() && e2.is_vertical()) {
    return are_opposite_vertical(e1, e2);
  }

  if (e1.is_horizontal() && e2.is_horizontal()) {
    return are_opposite_horizontal(e1, e2);
  }

  // One vertical and one horizontal are not considered opposite
  // (they are adjacent corners, not width violations)
  return false;
}

// Calculate segment-to-segment distance
inline double segment_to_segment_distance(const Segment& s1,
                                          const Segment& s2,
                                          Point& closest_p1,
                                          Point& closest_p2) {

  // For parallel segments (most common in width checking),
  // we need to find the minimum distance
  double min_dist = std::numeric_limits<double>::max();

  // Try all endpoint combinations
  std::vector<Point> points_s1 = {s1.start, s1.end};
  std::vector<Point> points_s2 = {s2.start, s2.end};

  for (const auto& p1 : points_s1) {
    double dist = point_to_segment_distance(p1, s2);
    if (dist < min_dist) {
      min_dist = dist;
      closest_p1 = p1;
      // Find closest point on s2
      // (simplified - just use midpoint for now)
      closest_p2 = Point((s2.start.x() + s2.end.x()) / 2,
                        (s2.start.y() + s2.end.y()) / 2);
    }
  }

  for (const auto& p2 : points_s2) {
    double dist = point_to_segment_distance(p2, s1);
    if (dist < min_dist) {
      min_dist = dist;
      closest_p2 = p2;
      closest_p1 = Point((s1.start.x() + s1.end.x()) / 2,
                        (s1.start.y() + s1.end.y()) / 2);
    }
  }

  return min_dist;
}

class WidthChecker {
 public:
  WidthChecker(const Polygon& poly, double R, double r)
      : polygon_(poly), rule_distance_(R), sampling_radius_(r) {}

  std::vector<WidthViolation> check() {
    // Step 1: Sample representatives
    std::vector<RepresentativePoint> rep_points;
    std::vector<RepresentativeEdge> rep_edges;

    sample_representatives(polygon_, sampling_radius_,
                          rep_points, rep_edges);

    // Step 2: Run sweepline (same as space check, but on single polygon)
    // This detects all close edge pairs

    // For simplicity, we'll do a brute-force check for width violations
    // In practice, you would use the same sweepline algorithm
    std::vector<WidthViolation> violations;

    // Check all pairs of representative edges
    for (size_t i = 0; i < rep_edges.size(); ++i) {
      for (size_t j = i + 1; j < rep_edges.size(); ++j) {
        const auto& edge1 = rep_edges[i].edge;
        const auto& edge2 = rep_edges[j].edge;

        // Step 3: Filter - only check opposite edges
        if (!are_opposite(edge1, edge2)) continue;

        // Calculate minimum distance
        Point closest_p1, closest_p2;
        double dist = segment_to_segment_distance(edge1, edge2,
                                                  closest_p1, closest_p2);

        // Step 4: Check if it violates the rule
        if (dist < rule_distance_) {
          violations.emplace_back(edge1, edge2, dist,
                                 closest_p1, closest_p2,
                                 polygon_.id);
        }
      }
    }

    return violations;
  }

 private:
  const Polygon& polygon_;
  double rule_distance_;
  double sampling_radius_;
};

// Main width checking function
inline std::vector<WidthViolation> check_width_violations(
    const Polygon& polygon, double R, double r) {

  WidthChecker checker(polygon, R, r);
  return checker.check();
}

// Check width violations for all polygons
inline std::vector<WidthViolation> check_all_width_violations(
    const std::vector<Polygon>& polygons,
    double R,
    double multiplier = 4.0) {

  std::vector<WidthViolation> all_violations;

  for (const auto& poly : polygons) {
    double r = calculate_sampling_radius(poly, multiplier);
    auto violations = check_width_violations(poly, R, r);

    all_violations.insert(all_violations.end(),
                         violations.begin(), violations.end());
  }

  return all_violations;
}

}  // namespace easymrc
