#pragma once

#include <vector>
#include <thread>
#include <algorithm>
#include "types.hpp"
#include "type_a_violations.hpp"
#include "type_b_violations.hpp"
#include "width_check.hpp"
#include "sampling.hpp"

namespace easymrc {

// Parallel space checking for multiple polygon pairs
class ParallelSpaceChecker {
 public:
  ParallelSpaceChecker(const std::vector<Polygon>& polygons,
                       const std::vector<std::pair<int, int>>& pairs,
                       double R,
                       double multiplier = 4.0,
                       int num_threads = 0)
      : polygons_(polygons), pairs_(pairs),
        rule_distance_(R), radius_multiplier_(multiplier) {

    if (num_threads <= 0) {
      num_threads_ = std::thread::hardware_concurrency();
      if (num_threads_ == 0) num_threads_ = 4;  // Default fallback
    } else {
      num_threads_ = num_threads;
    }
  }

  void check_parallel(std::vector<Violation>& violations_a,
                     std::vector<ViolationTypeB>& violations_b) {

    int total_pairs = pairs_.size();
    int pairs_per_thread = (total_pairs + num_threads_ - 1) / num_threads_;

    std::vector<std::thread> threads;
    std::vector<std::vector<Violation>> thread_results_a(num_threads_);
    std::vector<std::vector<ViolationTypeB>> thread_results_b(num_threads_);

    for (int t = 0; t < num_threads_; ++t) {
      int start_idx = t * pairs_per_thread;
      int end_idx = std::min((t + 1) * pairs_per_thread, total_pairs);

      if (start_idx >= end_idx) break;  // No work for this thread

      threads.emplace_back([this, start_idx, end_idx, t,
                           &thread_results_a, &thread_results_b]() {
        for (int i = start_idx; i < end_idx; ++i) {
          int poly1_id = pairs_[i].first;
          int poly2_id = pairs_[i].second;

          const auto& poly1 = polygons_[poly1_id];
          const auto& poly2 = polygons_[poly2_id];

          // Calculate sampling radius
          double r1 = calculate_sampling_radius(poly1, radius_multiplier_);
          double r2 = calculate_sampling_radius(poly2, radius_multiplier_);
          double r = std::max(r1, r2);

          // Sample representatives
          std::vector<RepresentativePoint> rep_points_1, rep_points_2;
          std::vector<RepresentativeEdge> rep_edges_1, rep_edges_2;

          sample_representatives(poly1, r, rep_points_1, rep_edges_1);
          sample_representatives(poly2, r, rep_points_2, rep_edges_2);

          // Check type (a) violations
          auto vio_a = detect_type_a_violations(rep_points_1, rep_points_2,
                                                rule_distance_, r);
          thread_results_a[t].insert(thread_results_a[t].end(),
                                     vio_a.begin(), vio_a.end());

          // Check type (b) violations
          auto vio_b = detect_type_b_violations(rep_points_1, rep_points_2,
                                                rep_edges_1, rep_edges_2,
                                                rule_distance_, r);
          thread_results_b[t].insert(thread_results_b[t].end(),
                                     vio_b.begin(), vio_b.end());
        }
      });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
      thread.join();
    }

    // Aggregate results
    for (const auto& result : thread_results_a) {
      violations_a.insert(violations_a.end(), result.begin(), result.end());
    }

    for (const auto& result : thread_results_b) {
      violations_b.insert(violations_b.end(), result.begin(), result.end());
    }
  }

 private:
  const std::vector<Polygon>& polygons_;
  const std::vector<std::pair<int, int>>& pairs_;
  double rule_distance_;
  double radius_multiplier_;
  int num_threads_;
};

// Parallel width checking for multiple polygons
class ParallelWidthChecker {
 public:
  ParallelWidthChecker(const std::vector<Polygon>& polygons,
                       double R,
                       double multiplier = 4.0,
                       int num_threads = 0)
      : polygons_(polygons),
        rule_distance_(R),
        radius_multiplier_(multiplier) {

    if (num_threads <= 0) {
      num_threads_ = std::thread::hardware_concurrency();
      if (num_threads_ == 0) num_threads_ = 4;
    } else {
      num_threads_ = num_threads;
    }
  }

  std::vector<WidthViolation> check_parallel() {
    int total_polys = polygons_.size();
    int polys_per_thread = (total_polys + num_threads_ - 1) / num_threads_;

    std::vector<std::thread> threads;
    std::vector<std::vector<WidthViolation>> thread_results(num_threads_);

    for (int t = 0; t < num_threads_; ++t) {
      int start_idx = t * polys_per_thread;
      int end_idx = std::min((t + 1) * polys_per_thread, total_polys);

      if (start_idx >= end_idx) break;

      threads.emplace_back([this, start_idx, end_idx, t,
                           &thread_results]() {
        for (int i = start_idx; i < end_idx; ++i) {
          const auto& poly = polygons_[i];
          double r = calculate_sampling_radius(poly, radius_multiplier_);

          auto violations = check_width_violations(poly, rule_distance_, r);

          thread_results[t].insert(thread_results[t].end(),
                                  violations.begin(), violations.end());
        }
      });
    }

    // Wait for all threads
    for (auto& thread : threads) {
      thread.join();
    }

    // Aggregate results
    std::vector<WidthViolation> all_violations;
    for (const auto& result : thread_results) {
      all_violations.insert(all_violations.end(),
                           result.begin(), result.end());
    }

    return all_violations;
  }

 private:
  const std::vector<Polygon>& polygons_;
  double rule_distance_;
  double radius_multiplier_;
  int num_threads_;
};

// Main parallel space checking function
inline void parallel_space_check(
    const std::vector<Polygon>& polygons,
    const std::vector<std::pair<int, int>>& pairs,
    double R,
    std::vector<Violation>& violations_a,
    std::vector<ViolationTypeB>& violations_b,
    double multiplier = 4.0,
    int num_threads = 0) {

  ParallelSpaceChecker checker(polygons, pairs, R, multiplier, num_threads);
  checker.check_parallel(violations_a, violations_b);
}

// Main parallel width checking function
inline std::vector<WidthViolation> parallel_width_check(
    const std::vector<Polygon>& polygons,
    double R,
    double multiplier = 4.0,
    int num_threads = 0) {

  ParallelWidthChecker checker(polygons, R, multiplier, num_threads);
  return checker.check_parallel();
}

}  // namespace easymrc
