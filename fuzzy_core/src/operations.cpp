#include "fuzzy_value.h"
#include <optional>

double FuzzyValue::centroid() const noexcept {
	if (mus_.empty()) return 0.0;

	double weighted_sum = 0.0;
	double mu_sum = 0.0;
	for (std::size_t i = 0; i < xs_.size(); ++i) {
		weighted_sum += xs_[i] * mus_[i];
		mu_sum += mus_[i];
	}
	if (mu_sum < 1e-9) return 0.0;
	return weighted_sum / mu_sum;
}

bool operator<(const FuzzyValue& a, const FuzzyValue& b) { return a.centroid() < b.centroid(); }
bool operator>(const FuzzyValue& a, const FuzzyValue& b) { return a.centroid() > b.centroid(); }
bool operator==(const FuzzyValue& a, const FuzzyValue& b) { return std::fabs(a.centroid() - b.centroid()) < 1e-9; }
bool operator!=(const FuzzyValue& a, const FuzzyValue& b) { return !(a == b); }

namespace detail {
    template <typename BinaryOp>
    FuzzyValue combine(const FuzzyValue& a, const FuzzyValue& b, BinaryOp op) {
        if (a.empty() || b.empty()) return FuzzyValue();

        std::map<double, double> result;
        for (std::size_t i = 0; i < a.size(); ++i) {
            for (std::size_t j = 0; j < b.size(); ++j) {
                std::optional<double> z_opt = op(a.xs()[i], b.xs()[j]);
                if (!z_opt.has_value()) continue;
                double z = *z_opt;
                double mu = std::min(a.mus()[i], b.mus()[j]);
                auto it = result.find(z);
                if (it == result.end()) {
                    result[z] = mu;
                }
                else {
                    it->second = std::max(it->second, mu);
                }
            }
        }

        std::vector<double> xs;
        std::vector<double> mus;
        xs.reserve(result.size());
        mus.reserve(result.size());
        for (const auto& pair : result) {
            xs.push_back(pair.first);
            mus.push_back(pair.second);
        }

        FuzzyValue res;
        res.assign(xs, mus);
        return res;
    }
}

FuzzyValue operator+(const FuzzyValue& a, const FuzzyValue& b) {
	return detail::combine(a, b, [](double x, double y) {return x + y; });
}
FuzzyValue operator-(const FuzzyValue& a, const FuzzyValue& b) {
	return detail::combine(a, b, [](double x, double y) {return x - y; });
}
FuzzyValue operator*(const FuzzyValue& a, const FuzzyValue& b) {
	return detail::combine(a, b, [](double x, double y) {return x * y; });
}
FuzzyValue operator/(const FuzzyValue& a, const FuzzyValue& b) {
	return detail::combine(a, b, [](double x, double y) -> std::optional<double> {
		if (std::fabs(y) < 1e-9) return std::nullopt; 
		return x / y;
		});
}