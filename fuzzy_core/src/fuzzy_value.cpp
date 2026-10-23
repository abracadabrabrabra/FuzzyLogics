#include "fuzzy_value.h"

FuzzyValue::FuzzyValue(const std::vector<double>& xs, const std::vector<double>& mus): xs_(xs), mus_(mus) {}

void FuzzyValue::assign(const std::vector<double>& xs, const std::vector<double>& mus) {
	xs_ = xs;
	mus_ = mus;
}

void FuzzyValue::add_point(double x, double mu) {
	xs_.push_back(x);
	mus_.push_back(mu);
}

std::size_t FuzzyValue::size() const noexcept { return xs_.size(); }

bool FuzzyValue::empty() const noexcept { return xs_.empty(); }

const std::vector<double>& FuzzyValue::xs() const noexcept { return xs_; }

const std::vector<double>& FuzzyValue::mus() const noexcept { return mus_; }

void FuzzyValue::normalize(NormalizeMode mode) {
	if (mus_.empty())	return;
	double divisor = 0.0;
	if (mode == NormalizeMode::Sum) {
		for (double mu : mus_)	divisor += mu;
	}
	else {
		divisor = *std::max_element(mus_.begin(), mus_.end());
	}
	if (divisor < 1e-9)	return;
	for (double& mu : mus_)	mu /= divisor;
}

void FuzzyValue::clear() noexcept {
	xs_.clear();
	mus_.clear();
}

void FuzzyValue::prune(double threshold) {
	std::size_t write_idx = 0;
	for (std::size_t read_idx = 0; read_idx < xs_.size(); ++read_idx) {
		if (mus_[read_idx] > threshold) {
			xs_[write_idx] = xs_[read_idx];
			mus_[write_idx] = mus_[read_idx];
			++write_idx;
		}
	}
	xs_.resize(write_idx);
	mus_.resize(write_idx);
}

bool FuzzyValue::is_valid() const noexcept {
	if (xs_.size() != mus_.size()) return false;

	for (std::size_t i = 0; i < xs_.size(); ++i) {
		const double x = xs_[i];
		const double mu = mus_[i];

		if (std::isnan(x) || std::isinf(x)) return false;

		if (std::isnan(mu) || std::isinf(mu)) return false;
		if (mu < 0.0 || mu > 1.0) return false;

		if (i > 0 && x <= xs_[i - 1]) return false;
	}

	return true;
}