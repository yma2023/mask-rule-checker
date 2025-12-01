#pragma once

#include <vector>
#include <set>
#include <algorithm>
#include "types.hpp"

namespace easymrc {

// Event types for sweepline algorithm
enum EventType {
  LEFT_EVENT = 0,
  RIGHT_EVENT = 1
};

struct Event {
  double x;
  EventType type;
  int polygon_id;
  double y_min, y_max;

  Event() : x(0), type(LEFT_EVENT), polygon_id(-1), y_min(0), y_max(0) {}

  Event(double x_coord, EventType t, int pid, double ymin, double ymax)
      : x(x_coord), type(t), polygon_id(pid), y_min(ymin), y_max(ymax) {}

  bool operator<(const Event& other) const {
    if (x != other.x) return x < other.x;
    if (type != other.type) return type < other.type;
    return y_min < other.y_min;
  }
};

struct Interval {
  double y_min, y_max;
  int polygon_id;

  Interval() : y_min(0), y_max(0), polygon_id(-1) {}

  Interval(double ymin, double ymax, int pid)
      : y_min(ymin), y_max(ymax), polygon_id(pid) {}

  bool overlaps(const Interval& other) const {
    return !(y_max < other.y_min || other.y_max < y_min);
  }

  bool operator<(const Interval& other) const {
    if (y_min != other.y_min) return y_min < other.y_min;
    if (y_max != other.y_max) return y_max < other.y_max;
    return polygon_id < other.polygon_id;
  }
};

class CandidatePairGenerator {
 public:
  CandidatePairGenerator(const std::vector<Polygon>& polygons, double R)
      : polygons_(polygons), rule_distance_(R) {}

  std::vector<std::pair<int, int>> generate() {
    // Step 1: Compute and expand bounding boxes
    std::vector<BoundingBox> bboxes;
    for (const auto& poly : polygons_) {
      BoundingBox bbox = compute_bounding_box(poly);
      bbox.expand(rule_distance_);
      bboxes.push_back(bbox);
    }

    // Step 2: Generate events
    std::vector<Event> events;
    for (const auto& bbox : bboxes) {
      events.emplace_back(bbox.min_x, LEFT_EVENT, bbox.polygon_id,
                         bbox.min_y, bbox.max_y);
      events.emplace_back(bbox.max_x, RIGHT_EVENT, bbox.polygon_id,
                         bbox.min_y, bbox.max_y);
    }

    // Step 3: Sort events
    std::sort(events.begin(), events.end());

    // Step 4: Sweepline scan
    std::set<Interval> active_intervals;
    std::set<std::pair<int, int>> candidate_pairs_set;

    for (const auto& event : events) {
      if (event.type == LEFT_EVENT) {
        // Search for overlapping intervals
        Interval query_interval(event.y_min, event.y_max, event.polygon_id);

        for (const auto& active : active_intervals) {
          if (query_interval.overlaps(active)) {
            // Found overlapping interval, add as candidate pair
            int id1 = event.polygon_id;
            int id2 = active.polygon_id;

            // Ensure i < j to avoid duplicates
            if (id1 != id2) {
              if (id1 > id2) std::swap(id1, id2);
              candidate_pairs_set.insert({id1, id2});
            }
          }
        }

        // Insert current interval
        active_intervals.insert(query_interval);

      } else {  // RIGHT_EVENT
        // Remove interval from active set
        Interval to_remove(event.y_min, event.y_max, event.polygon_id);
        active_intervals.erase(to_remove);
      }
    }

    // Convert set to vector
    std::vector<std::pair<int, int>> candidate_pairs(
        candidate_pairs_set.begin(), candidate_pairs_set.end());

    return candidate_pairs;
  }

  // Get statistics
  struct Statistics {
    int total_polygons;
    int candidate_pairs;
    double reduction_ratio;
  };

  Statistics get_statistics(int num_pairs) const {
    Statistics stats;
    stats.total_polygons = polygons_.size();
    stats.candidate_pairs = num_pairs;

    int total_possible_pairs = stats.total_polygons *
                               (stats.total_polygons - 1) / 2;

    if (total_possible_pairs > 0) {
      stats.reduction_ratio = 1.0 - (double)num_pairs / total_possible_pairs;
    } else {
      stats.reduction_ratio = 0.0;
    }

    return stats;
  }

 private:
  const std::vector<Polygon>& polygons_;
  double rule_distance_;
};

// Main function for candidate pair generation
inline std::vector<std::pair<int, int>> candidate_pair_generation(
    const std::vector<Polygon>& polygons, double R) {

  CandidatePairGenerator generator(polygons, R);
  return generator.generate();
}

// Get statistics about candidate pairs
inline auto get_candidate_pair_statistics(
    const std::vector<Polygon>& polygons,
    const std::vector<std::pair<int, int>>& pairs) {

  struct Stats {
    int total_polygons;
    int candidate_pairs;
    int total_possible_pairs;
    double reduction_ratio;
  };

  Stats stats;
  stats.total_polygons = polygons.size();
  stats.candidate_pairs = pairs.size();
  stats.total_possible_pairs = stats.total_polygons *
                                (stats.total_polygons - 1) / 2;

  if (stats.total_possible_pairs > 0) {
    stats.reduction_ratio = 1.0 - (double)stats.candidate_pairs /
                                   stats.total_possible_pairs;
  } else {
    stats.reduction_ratio = 0.0;
  }

  return stats;
}

}  // namespace easymrc
