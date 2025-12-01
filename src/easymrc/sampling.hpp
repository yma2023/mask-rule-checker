#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include "types.hpp"

namespace easymrc {

class RepresentativeSampler {
 public:
  RepresentativeSampler(const Polygon& poly, double sampling_radius)
      : polygon_(poly), r_(sampling_radius) {}

  // Sample representative points and edges
  void sample(std::vector<RepresentativePoint>& rep_points,
              std::vector<RepresentativeEdge>& rep_edges) {

    if (polygon_.vertices.empty()) return;

    // Select representative points
    std::vector<int> rep_indices = select_representative_points();

    // Build representative points with shielded information
    for (int idx : rep_indices) {
      RepresentativePoint rep_point(polygon_.vertices[idx], polygon_.id);

      // Find shielded vertices (within distance r)
      for (size_t i = 0; i < polygon_.vertices.size(); ++i) {
        double dist = euclidean_distance(polygon_.vertices[idx],
                                        polygon_.vertices[i]);
        if (dist <= r_) {
          rep_point.shielded_vertices.push_back(polygon_.vertices[i]);
        }
      }

      // Find shielded edges (within distance r)
      for (const auto& seg : polygon_.segments) {
        double dist = point_to_segment_distance(polygon_.vertices[idx], seg);
        if (dist <= r_) {
          rep_point.shielded_edges.push_back(seg);
        }
      }

      rep_points.push_back(rep_point);
    }

    // Select representative edges (length > r)
    for (const auto& seg : polygon_.segments) {
      if (seg.length() > r_) {
        RepresentativeEdge rep_edge(seg, polygon_.id);

        // Find shielded vertices near this edge
        for (const auto& vertex : polygon_.vertices) {
          double dist = point_to_segment_distance(vertex, seg);
          if (dist <= r_) {
            rep_edge.shielded_vertices.push_back(vertex);
          }
        }

        rep_edges.push_back(rep_edge);
      }
    }
  }

  // Get reduction statistics
  struct SamplingStats {
    int original_vertices;
    int representative_points;
    int representative_edges;
    double reduction_ratio;
  };

  SamplingStats get_statistics(int num_rep_points) const {
    SamplingStats stats;
    stats.original_vertices = polygon_.vertices.size();
    stats.representative_points = num_rep_points;
    stats.representative_edges = 0;

    for (const auto& seg : polygon_.segments) {
      if (seg.length() > r_) {
        stats.representative_edges++;
      }
    }

    if (stats.original_vertices > 0) {
      stats.reduction_ratio = 1.0 - (double)stats.representative_points /
                                     stats.original_vertices;
    } else {
      stats.reduction_ratio = 0.0;
    }

    return stats;
  }

 private:
  const Polygon& polygon_;
  double r_;

  // Calculate average edge length
  double calculate_average_edge_length() const {
    if (polygon_.segments.empty()) return 0.0;

    double total_length = 0.0;
    for (const auto& seg : polygon_.segments) {
      total_length += seg.length();
    }

    return total_length / polygon_.segments.size();
  }

  // Calculate distance along boundary from vertex i to vertex j
  double distance_along_boundary(int start_idx, int end_idx) const {
    double dist = 0.0;
    int n = polygon_.vertices.size();

    int current = start_idx;
    while (current != end_idx) {
      int next = (current + 1) % n;
      dist += euclidean_distance(polygon_.vertices[current],
                                polygon_.vertices[next]);
      current = next;

      // Safety check
      if (dist > r_ * 2) break;
    }

    return dist;
  }

  // Find next representative point using greedy algorithm
  int find_next_representative(int current_idx) const {
    int n = polygon_.vertices.size();
    int max_dist_idx = (current_idx + 1) % n;
    double max_dist = 0.0;

    for (int i = 1; i < n; ++i) {
      int idx = (current_idx + i) % n;
      double cumulative_dist = distance_along_boundary(current_idx, idx);

      if (cumulative_dist > max_dist && cumulative_dist <= r_) {
        max_dist = cumulative_dist;
        max_dist_idx = idx;
      }

      // Stop if we've gone beyond r
      if (cumulative_dist > r_) break;
    }

    return max_dist_idx;
  }

  // Select representative points using greedy algorithm
  std::vector<int> select_representative_points() const {
    std::vector<int> representatives;

    if (polygon_.vertices.empty()) return representatives;

    int n = polygon_.vertices.size();
    std::vector<bool> covered(n, false);

    int current = 0;
    representatives.push_back(current);
    covered[current] = true;

    // Mark vertices within r of current as covered
    for (int i = 0; i < n; ++i) {
      if (distance_along_boundary(current, i) <= r_) {
        covered[i] = true;
      }
    }

    // Continue until all vertices are covered
    int iterations = 0;
    while (iterations < n) {
      // Find next uncovered vertex
      int next = -1;
      for (int i = 1; i < n; ++i) {
        int idx = (current + i) % n;
        if (!covered[idx]) {
          next = idx;
          break;
        }
      }

      if (next == -1) break;  // All covered

      // Find the farthest vertex within r from current
      int next_rep = find_next_representative(current);
      representatives.push_back(next_rep);
      covered[next_rep] = true;

      // Mark vertices within r of next_rep as covered
      for (int i = 0; i < n; ++i) {
        if (distance_along_boundary(next_rep, i) <= r_) {
          covered[i] = true;
        }
      }

      current = next_rep;
      iterations++;
    }

    return representatives;
  }
};

// Main sampling function
inline void sample_representatives(
    const Polygon& polygon,
    double sampling_radius,
    std::vector<RepresentativePoint>& rep_points,
    std::vector<RepresentativeEdge>& rep_edges) {

  RepresentativeSampler sampler(polygon, sampling_radius);
  sampler.sample(rep_points, rep_edges);
}

// Calculate optimal sampling radius (r = 4 * average_edge_length)
inline double calculate_sampling_radius(const Polygon& polygon,
                                        double multiplier = 4.0) {
  if (polygon.segments.empty()) return 0.0;

  double total_length = 0.0;
  for (const auto& seg : polygon.segments) {
    total_length += seg.length();
  }

  double avg_length = total_length / polygon.segments.size();
  return multiplier * avg_length;
}

// Sample representatives for all polygons
inline void sample_all_polygons(
    const std::vector<Polygon>& polygons,
    double multiplier,
    std::vector<std::vector<RepresentativePoint>>& all_rep_points,
    std::vector<std::vector<RepresentativeEdge>>& all_rep_edges) {

  all_rep_points.resize(polygons.size());
  all_rep_edges.resize(polygons.size());

  for (size_t i = 0; i < polygons.size(); ++i) {
    double r = calculate_sampling_radius(polygons[i], multiplier);
    sample_representatives(polygons[i], r, all_rep_points[i],
                          all_rep_edges[i]);
  }
}

}  // namespace easymrc
