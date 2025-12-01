#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include "types.hpp"

namespace easymrc {

// Event types for type (b) violations
enum EdgeEventType {
  VERTICAL_LEFT = 0,
  VERTICAL_RIGHT = 1,
  HORIZONTAL = 2,
  POINT_EVENT = 3
};

struct EdgeEvent {
  double x;
  EdgeEventType event_type;
  int entity_id;  // point index or edge index
  double y_value;
  double y_min, y_max;  // For vertical edges
  bool is_point;

  EdgeEvent()
      : x(0), event_type(POINT_EVENT), entity_id(-1),
        y_value(0), y_min(0), y_max(0), is_point(true) {}

  EdgeEvent(double x_coord, EdgeEventType type, int id, double y)
      : x(x_coord), event_type(type), entity_id(id),
        y_value(y), y_min(0), y_max(0), is_point(true) {}

  EdgeEvent(double x_coord, EdgeEventType type, int id,
           double ymin, double ymax)
      : x(x_coord), event_type(type), entity_id(id),
        y_value(0), y_min(ymin), y_max(ymax), is_point(false) {}

  bool operator<(const EdgeEvent& other) const {
    if (x != other.x) return x < other.x;
    return event_type < other.event_type;
  }
};

class TypeBViolationDetector {
 public:
  TypeBViolationDetector(const std::vector<RepresentativePoint>& points_p1,
                         const std::vector<RepresentativePoint>& points_p2,
                         const std::vector<RepresentativeEdge>& edges_p1,
                         const std::vector<RepresentativeEdge>& edges_p2,
                         double R, double r)
      : p1_points_(points_p1), p2_points_(points_p2),
        p1_edges_(edges_p1), p2_edges_(edges_p2),
        rule_distance_(R), sampling_radius_(r) {
    R_prime_ = R + r;  // Extended rule distance for type (b)
  }

  std::vector<ViolationTypeB> detect() {
    std::vector<ViolationTypeB> violations;

    // Generate events
    std::vector<EdgeEvent> events = generate_events();

    // Sort events
    std::sort(events.begin(), events.end());

    // Sweepline scan
    SegmentTree point_tree;

    for (const auto& event : events) {
      // Delete points that are too far left
      point_tree.erase_by_x(event.x - R_prime_);

      if (event.event_type == POINT_EVENT) {
        // Point event - insert into tree
        // (We'll handle this differently based on implementation)

      } else if (event.event_type == VERTICAL_LEFT ||
                 event.event_type == VERTICAL_RIGHT) {
        // Vertical edge event
        handle_vertical_edge_event(event, point_tree, violations);

      } else if (event.event_type == HORIZONTAL) {
        // Horizontal edge event
        handle_horizontal_edge_event(event, point_tree, violations);
      }
    }

    return violations;
  }

 private:
  const std::vector<RepresentativePoint>& p1_points_;
  const std::vector<RepresentativePoint>& p2_points_;
  const std::vector<RepresentativeEdge>& p1_edges_;
  const std::vector<RepresentativeEdge>& p2_edges_;
  double rule_distance_;
  double sampling_radius_;
  double R_prime_;

  std::vector<EdgeEvent> generate_events() {
    std::vector<EdgeEvent> events;

    // Add point events from p1
    for (size_t i = 0; i < p1_points_.size(); ++i) {
      const auto& p = p1_points_[i];
      events.emplace_back(p.coordinates.x(), POINT_EVENT,
                         static_cast<int>(i), p.coordinates.y());
    }

    // Add point events from p2
    for (size_t i = 0; i < p2_points_.size(); ++i) {
      const auto& p = p2_points_[i];
      events.emplace_back(p.coordinates.x(), POINT_EVENT,
                         static_cast<int>(i + p1_points_.size()),
                         p.coordinates.y());
    }

    // Add edge events from p1
    for (size_t i = 0; i < p1_edges_.size(); ++i) {
      const auto& edge = p1_edges_[i].edge;
      add_edge_events(edge, static_cast<int>(i), events);
    }

    // Add edge events from p2
    for (size_t i = 0; i < p2_edges_.size(); ++i) {
      const auto& edge = p2_edges_[i].edge;
      add_edge_events(edge, static_cast<int>(i + p1_edges_.size()), events);
    }

    return events;
  }

  void add_edge_events(const Segment& edge, int edge_id,
                      std::vector<EdgeEvent>& events) {

    if (edge.is_vertical()) {
      // Vertical edge: create LEFT and RIGHT events
      double x0 = edge.start.x();
      double y_min = std::min(edge.start.y(), edge.end.y());
      double y_max = std::max(edge.start.y(), edge.end.y());

      events.emplace_back(x0, VERTICAL_LEFT, edge_id, y_min, y_max);
      events.emplace_back(x0 + R_prime_, VERTICAL_RIGHT, edge_id,
                         y_min, y_max);

    } else if (edge.is_horizontal()) {
      // Horizontal edge: create single event at right end
      double x_max = std::max(edge.start.x(), edge.end.x());
      double y0 = edge.start.y();

      events.emplace_back(x_max + sampling_radius_, HORIZONTAL,
                         edge_id, y0);
    }
  }

  void handle_vertical_edge_event(const EdgeEvent& event,
                                  SegmentTree& point_tree,
                                  std::vector<ViolationTypeB>& violations) {

    // Query points in y-range [y_min - r, y_max + r]
    double y_min = event.y_min - sampling_radius_;
    double y_max = event.y_max + sampling_radius_;

    auto found_points = point_tree.range_query(y_min, y_max);

    // Get the edge
    int edge_idx = event.entity_id;
    const RepresentativeEdge* edge_ptr = nullptr;

    if (edge_idx < (int)p1_edges_.size()) {
      edge_ptr = &p1_edges_[edge_idx];
    } else {
      edge_ptr = &p2_edges_[edge_idx - p1_edges_.size()];
    }

    if (!edge_ptr) return;

    // Check violations
    for (const auto& point : found_points) {
      check_point_edge_violation(point, *edge_ptr, violations);
    }
  }

  void handle_horizontal_edge_event(const EdgeEvent& event,
                                    SegmentTree& point_tree,
                                    std::vector<ViolationTypeB>& violations) {

    // Query points in y-range [y0 - R', y0 + R']
    double y0 = event.y_value;
    double y_min = y0 - R_prime_;
    double y_max = y0 + R_prime_;

    auto found_points = point_tree.range_query(y_min, y_max);

    // Get the edge
    int edge_idx = event.entity_id;
    const RepresentativeEdge* edge_ptr = nullptr;

    if (edge_idx < (int)p1_edges_.size()) {
      edge_ptr = &p1_edges_[edge_idx];
    } else {
      edge_ptr = &p2_edges_[edge_idx - p1_edges_.size()];
    }

    if (!edge_ptr) return;

    // Check violations
    for (const auto& point : found_points) {
      check_point_edge_violation(point, *edge_ptr, violations);
    }
  }

  void check_point_edge_violation(const RepresentativePoint& point,
                                  const RepresentativeEdge& edge,
                                  std::vector<ViolationTypeB>& violations) {

    // Check all shielded vertices of the point against shielded vertices
    // of the edge
    for (const auto& point_v : point.shielded_vertices) {
      for (const auto& point_e : edge.shielded_vertices) {
        double distance = euclidean_distance(point_v, point_e);

        if (distance < rule_distance_) {
          violations.emplace_back(point_v, edge.edge, distance,
                                 point.polygon_id, edge.polygon_id);
        }
      }
    }
  }
};

// Main function for type (b) violation detection
inline std::vector<ViolationTypeB> detect_type_b_violations(
    const std::vector<RepresentativePoint>& points_p1,
    const std::vector<RepresentativePoint>& points_p2,
    const std::vector<RepresentativeEdge>& edges_p1,
    const std::vector<RepresentativeEdge>& edges_p2,
    double R, double r) {

  TypeBViolationDetector detector(points_p1, points_p2,
                                  edges_p1, edges_p2, R, r);
  return detector.detect();
}

// Check violations for a single polygon pair (both type a and b)
inline void check_space_violations_complete(
    const Polygon& poly1,
    const Polygon& poly2,
    double R,
    double r,
    std::vector<Violation>& violations_a,
    std::vector<ViolationTypeB>& violations_b) {

  // Sample representatives for both polygons
  std::vector<RepresentativePoint> rep_points_1, rep_points_2;
  std::vector<RepresentativeEdge> rep_edges_1, rep_edges_2;

  sample_representatives(poly1, r, rep_points_1, rep_edges_1);
  sample_representatives(poly2, r, rep_points_2, rep_edges_2);

  // Detect type (a) violations
  violations_a = detect_type_a_violations(rep_points_1, rep_points_2, R, r);

  // Detect type (b) violations
  violations_b = detect_type_b_violations(rep_points_1, rep_points_2,
                                         rep_edges_1, rep_edges_2, R, r);
}

}  // namespace easymrc
