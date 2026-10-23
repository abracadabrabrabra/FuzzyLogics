#pragma once

#include <vector>
#include <cstddef>
#include <utility>
#include <algorithm>
#include <cmath>
#include <map>

class FuzzyValue {
private:
	std::vector<double> xs_;
	std::vector<double> mus_;

public:
	enum class NormalizeMode {
		Sum,   
		Max
	};

public:
	FuzzyValue() = default;
	FuzzyValue(const std::vector<double>& xs, const std::vector<double>& mus);

	void assign(const std::vector<double>& xs, const std::vector<double>& mus);
	void add_point(double x, double mu);

	std::size_t size() const noexcept;
	bool empty() const noexcept;
	const std::vector<double>& xs() const noexcept;
	const std::vector<double>& mus() const noexcept;

	void normalize(NormalizeMode mode = NormalizeMode::Sum);
	void clear() noexcept;
	void prune(double threshold = 0.1);
	bool is_valid() const noexcept;

	double centroid() const noexcept;
};

bool operator<(const FuzzyValue& a, const FuzzyValue& b);
bool operator>(const FuzzyValue& a, const FuzzyValue& b);
bool operator==(const FuzzyValue& a, const FuzzyValue& b);
bool operator!=(const FuzzyValue& a, const FuzzyValue& b);

FuzzyValue operator+(const FuzzyValue& a, const FuzzyValue& b);