#pragma once

// EasyMRC - Efficient Mask Rule Checking
// Main include file

#include "types.hpp"
#include "format_conversion.hpp"
#include "candidate_pairs.hpp"
#include "sampling.hpp"
#include "type_a_violations.hpp"
#include "type_b_violations.hpp"
#include "width_check.hpp"
#include "parallel.hpp"

namespace easymrc {


class EasyMRC {
 public:
  struct Config {
    double rule_distance_R;             // Space/width rule distance
    double sampling_radius_multiplier;  // Usually 4.0
    int num_threads;                    // 0 = auto-detect
    bool enable_space_check;
    bool enable_width_check;
    bool enable_parallel;

    Config()
        : rule_distance_R(50.0),
          sampling_radius_multiplier(4.0),
          num_threads(0),
          enable_space_check(true),
          enable_width_check(true),
          enable_parallel(true) {}
  };

  struct Results {
    std::vector<Violation> space_violations_type_a;
    std::vector<ViolationTypeB> space_violations_type_b;
    std::vector<WidthViolation> width_violations;

    int total_space_violations() const {
      return space_violations_type_a.size() +
             space_violations_type_b.size();
    }

    int total_violations() const {
      return total_space_violations() + width_violations.size();
    }
  };

  EasyMRC(const Config& config = Config()) : config_(config) {}

  Results run(const std::vector<Polygon>& polygons) {
    Results results;

    if (config_.enable_space_check) {
      check_space_rules(polygons, results);
    }

    if (config_.enable_width_check) {
      check_width_rules(polygons, results);
    }

    return results;
  }

  // Run MRC from image file
  Results run_from_image(const std::string& image_file) {
    auto polygons = format_conversion(image_file);
    return run(polygons);
  }

 private:
  Config config_;

  void check_space_rules(const std::vector<Polygon>& polygons,
                        Results& results) {

    // 候補ペア生成
    auto pairs = candidate_pair_generation(polygons, config_.rule_distance_R);

    if (config_.enable_parallel && pairs.size() > 10) {
      // 並列処理でチェック
      parallel_space_check(polygons, pairs, config_.rule_distance_R,
                          results.space_violations_type_a,
                          results.space_violations_type_b,
                          config_.sampling_radius_multiplier,
                          config_.num_threads);
    } else {
      // 逐次処理でチェック
      for (const auto& pair : pairs) {
        const auto& poly1 = polygons[pair.first];
        const auto& poly2 = polygons[pair.second];

        double r1 = calculate_sampling_radius(poly1,
                                             config_.sampling_radius_multiplier);
        double r2 = calculate_sampling_radius(poly2,
                                             config_.sampling_radius_multiplier);
        double r = std::max(r1, r2);

        std::vector<RepresentativePoint> rep_points_1, rep_points_2;
        std::vector<RepresentativeEdge> rep_edges_1, rep_edges_2;

        sample_representatives(poly1, r, rep_points_1, rep_edges_1);
        sample_representatives(poly2, r, rep_points_2, rep_edges_2);

        auto vio_a = detect_type_a_violations(rep_points_1, rep_points_2,
                                              config_.rule_distance_R, r);
        auto vio_b = detect_type_b_violations(rep_points_1, rep_points_2,
                                              rep_edges_1, rep_edges_2,
                                              config_.rule_distance_R, r);

        results.space_violations_type_a.insert(
            results.space_violations_type_a.end(),
            vio_a.begin(), vio_a.end());

        results.space_violations_type_b.insert(
            results.space_violations_type_b.end(),
            vio_b.begin(), vio_b.end());
      }
    }
  }

  void check_width_rules(const std::vector<Polygon>& polygons,
                        Results& results) {

    if (config_.enable_parallel && polygons.size() > 10) {
      results.width_violations = parallel_width_check(
          polygons, config_.rule_distance_R,
          config_.sampling_radius_multiplier,
          config_.num_threads);
    } else {
      for (const auto& poly : polygons) {
        double r = calculate_sampling_radius(poly,
                                             config_.sampling_radius_multiplier);
        auto violations = check_width_violations(poly,
                                                config_.rule_distance_R, r);

        results.width_violations.insert(results.width_violations.end(),
                                       violations.begin(), violations.end());
      }
    }
  }
};

}  // namespace easymrc
