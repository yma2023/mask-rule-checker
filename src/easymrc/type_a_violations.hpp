#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include "types.hpp"

namespace easymrc {

// Segment tree for efficient range queries
class SegmentTree {
 public:
  struct CompareByY {
    bool operator()(const RepresentativePoint& a,
                   const RepresentativePoint& b) const {
      if (a.coordinates.y() != b.coordinates.y())
        return a.coordinates.y() < b.coordinates.y();
      return a.coordinates.x() < b.coordinates.x();
    }
  };

  void insert(const RepresentativePoint& p) {
    tree_.insert(p);
  }

  void erase_by_x(double x_threshold) {
    auto it = tree_.begin();
    while (it != tree_.end()) {
      if (it->coordinates.x() < x_threshold) {
        it = tree_.erase(it);
      } else {
        ++it;
      }
    }
  }

  std::vector<RepresentativePoint> range_query(double y_min, double y_max) {
    std::vector<RepresentativePoint> result;

    // Create dummy point for lower_bound
    RepresentativePoint dummy_low;
    dummy_low.coordinates = Point(0, static_cast<int>(y_min));

    auto start = tree_.lower_bound(dummy_low);

    for (auto it = start; it != tree_.end(); ++it) {
      if (it->coordinates.y() > y_max) break;
      result.push_back(*it);
    }

    return result;
  }

  size_t size() const { return tree_.size(); }

  void clear() { tree_.clear(); }

 private:
  std::set<RepresentativePoint, CompareByY> tree_;
};

class TypeAViolationDetector {
 public:
  TypeAViolationDetector(const std::vector<RepresentativePoint>& points_p1,
                         const std::vector<RepresentativePoint>& points_p2,
                         double R, double r)
      : p1_points_(points_p1), p2_points_(points_p2),
        rule_distance_(R), sampling_radius_(r) {
    R_prime_ = R + 2 * r;  // Extended rule distance
  }

  std::vector<Violation> detect() {
    std::vector<Violation> violations;

    // Step 1: Merge and sort all representative points by x-coordinate
    struct PointWithPolygon {
      RepresentativePoint point;
      int polygon_owner;  // 0 for p1, 1 for p2

      bool operator<(const PointWithPolygon& other) const {
        if (point.coordinates.x() != other.point.coordinates.x())
          return point.coordinates.x() < other.point.coordinates.x();
        return point.coordinates.y() < other.point.coordinates.y();
      }
    };

    std::vector<PointWithPolygon> all_points;
    for (const auto& p : p1_points_) {
      all_points.push_back({p, 0});
    }
    for (const auto& p : p2_points_) {
      all_points.push_back({p, 1});
    }

    std::sort(all_points.begin(), all_points.end());

    // Step 2: Sweepline scan
    SegmentTree tree_p1, tree_p2;

    for (const auto& current : all_points) {
      double x = current.point.coordinates.x();

      // a) Delete points that are too far left
      tree_p1.erase_by_x(x - R_prime_);
      tree_p2.erase_by_x(x - R_prime_);

      // b) Search for nearby points
      double y = current.point.coordinates.y();
      double y_min = y - R_prime_;
      double y_max = y + R_prime_;

      std::vector<RepresentativePoint> found_points;

      if (current.polygon_owner == 0) {
        // Current point belongs to p1, search in p2
        found_points = tree_p2.range_query(y_min, y_max);
      } else {
        // Current point belongs to p2, search in p1
        found_points = tree_p1.range_query(y_min, y_max);
      }

      // c) Check violations for found points
      for (const auto& found : found_points) {
        check_violation(current.point, found, violations);
      }

      // d) Insert current point into appropriate tree
      if (current.polygon_owner == 0) {
        tree_p1.insert(current.point);
      } else {
        tree_p2.insert(current.point);
      }
    }

    return violations;
  }

 private:
  const std::vector<RepresentativePoint>& p1_points_;
  const std::vector<RepresentativePoint>& p2_points_;
  double rule_distance_;
  double sampling_radius_;
  double R_prime_;

  void check_violation(const RepresentativePoint& v,
                      const RepresentativePoint& q,
                      std::vector<Violation>& violations) {

    // Check all pairs of shielded vertices
    for (const auto& point_v : v.shielded_vertices) {
      for (const auto& point_q : q.shielded_vertices) {
        double distance = euclidean_distance(point_v, point_q);

        if (distance < rule_distance_) {
          violations.emplace_back(point_v, point_q, distance,
                                 v.polygon_id, q.polygon_id);
        }
      }
    }
  }
};

// Main function for type (a) violation detection
inline std::vector<Violation> detect_type_a_violations(
    const std::vector<RepresentativePoint>& points_p1,
    const std::vector<RepresentativePoint>& points_p2,
    double R, double r) {

  TypeAViolationDetector detector(points_p1, points_p2, R, r);
  return detector.detect();
}

// Check violations for a single polygon pair
inline std::vector<Violation> check_space_violations_type_a(
    const Polygon& poly1,
    const Polygon& poly2,
    double R,
    double r) {

  // Sample representatives for both polygons
  std::vector<RepresentativePoint> rep_points_1, rep_points_2;
  std::vector<RepresentativeEdge> rep_edges_1, rep_edges_2;

  sample_representatives(poly1, r, rep_points_1, rep_edges_1);
  sample_representatives(poly2, r, rep_points_2, rep_edges_2);

  // Detect type (a) violations
  return detect_type_a_violations(rep_points_1, rep_points_2, R, r);
}

}  // namespace easymrc
