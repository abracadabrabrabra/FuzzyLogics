#pragma once

#include "fuzzy_value.h"
#include <vector>
#include <random>
#include <cstdint>
#include <cstddef>

namespace fuzzy_generator {

	FuzzyValue generate_poisson_value(
		double lambda,
		std::size_t M,
		FuzzyValue::NormalizeMode mode = FuzzyValue::NormalizeMode::Max,
		double prune_threshold = 0.1,
		std::uint64_t seed = std::random_device{}()
	);

	FuzzyValue generate_poisson_value(
		double lambda,
		std::size_t M,
		FuzzyValue::NormalizeMode mode,
		double prune_threshold,
		std::mt19937_64& rng
	);

	std::vector<FuzzyValue> generate_poisson_set(
		std::size_t n,
		double lambda_min,
		double lambda_max,
		std::size_t M,
		FuzzyValue::NormalizeMode mode = FuzzyValue::NormalizeMode::Max,
		double prune_threshold = 0.1,
		std::uint64_t seed = std::random_device{}()
	);

} //namespace fuzzy_generator