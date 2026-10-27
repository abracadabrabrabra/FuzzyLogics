#include "fuzzy_session.h"
#include "fuzzy_io.h"
#include <stdexcept>
#include <sstream>
#include <fstream>

namespace fuzzy_session {

    std::size_t Session::add(const FuzzyValue& v) {
        values_.push_back(v);
        return values_.size() - 1;
    }
    std::size_t Session::add(FuzzyValue&& v) {
        values_.push_back(std::move(v));
        return values_.size() - 1;
    }

    std::pair<std::size_t, std::size_t> Session::add(const std::vector<FuzzyValue>& vs) {
        if (vs.empty()) return { values_.size(), values_.size() };
        std::size_t first = values_.size();
        for (const auto& v : vs) {
            values_.push_back(v);
        }
        return { first, values_.size() - 1 };
    }

    std::pair<std::size_t, std::size_t> Session::add(std::vector<FuzzyValue>&& vs) {
        if (vs.empty()) return { values_.size(), values_.size() };
        std::size_t first = values_.size();
        for (auto& v : vs) {
            values_.push_back(std::move(v));
        }
        return { first, values_.size() - 1 };
    }

    bool Session::remove(std::size_t index) {
        if (index >= values_.size()) return false;
        values_.erase(values_.begin() + index);
        return true;
    }

    void Session::clear() noexcept { values_.clear(); }
    std::size_t Session::size() const noexcept { return values_.size(); }
    bool Session::empty() const noexcept { return values_.empty(); }
    bool Session::contains(std::size_t index) const noexcept { return index < values_.size(); }

    const FuzzyValue& Session::get(std::size_t index) const {
        if (index >= values_.size()) throw std::out_of_range("Session::get: index out of range");
        return values_[index];
    }

    FuzzyValue& Session::get(std::size_t index) {
        if (index >= values_.size()) throw std::out_of_range("Session::get: index out of range");
        return values_[index];
    }

    std::size_t Session::add_values(std::size_t i, std::size_t j) {
        return add(get(i) + get(j));
    }
    std::size_t Session::sub_values(std::size_t i, std::size_t j) {
        return add(get(i) - get(j));
    }
    std::size_t Session::mul_values(std::size_t i, std::size_t j) {
        return add(get(i) * get(j));
    }
    std::size_t Session::div_values(std::size_t i, std::size_t j) {
        return add(get(i) / get(j));
    }

    CompareResult Session::compare(std::size_t i, std::size_t j, double eps) const {
        double ci = centroid(i);
        double cj = centroid(j);
        if (std::fabs(ci - cj) < eps) return CompareResult::Equal;
        return (ci < cj) ? CompareResult::Less : CompareResult::Greater;
    }

    double Session::centroid(std::size_t i) const {
        return get(i).centroid();
    }

    void Session::save_to_file(const std::string& path) const {
        std::ofstream out(path);
        if (!out) throw std::runtime_error("Session::save_to_file: cannot open " + path);
        out << values_.size() << "\n";
        for (const auto& v : values_) {
            fuzzy_io::save_to_stream(v, out);
        }
    }

    void Session::load_from_file(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("Session::load_from_file: cannot open " + path);
        std::size_t n = 0;
        if (!(in >> n)) throw std::invalid_argument("Session::load_from_file: cannot read count");
        for (std::size_t k = 0; k < n; ++k) {
            add(fuzzy_io::load_from_stream(in));
        }
    }

    void Session::log_summary(std::ostream& out) const {
        out << "Session summary: " << values_.size() << " values\n";
        for (std::size_t i = 0; i < values_.size(); ++i) {
            out << "[" << i << "] size=" << values_[i].size()
                << " centroid=" << values_[i].centroid() << "\n";
        }
    }

} // namespace fuzzy_session