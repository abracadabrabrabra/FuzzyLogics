#pragma once

#include "fuzzy_value.h"
#include <vector>
#include <random>
#include <cstddef>
#include <cstdint>

namespace fuzzy_cluster {

	struct FcmResult {
		std::vector<double> centers;
		std::vector<std::vector<double>> memberships;
	};

    FcmResult fuzzy_c_means(
        const std::vector<FuzzyValue>& values,
        std::size_t num_clusters,
        double m = 2.0,
        std::size_t max_iter = 100,
        double epsilon = 1e-5,
        std::uint64_t seed = std::random_device{}()
    );
} //namespace fuzzy_cluster