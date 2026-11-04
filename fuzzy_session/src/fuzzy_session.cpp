#include "fuzzy_session.h"
#include "fuzzy_io.h"
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <cstring>

namespace {
    template <typename T>
    bool write_pod(std::uint8_t*& ptr, std::uint8_t* end, const T& value) {
        if (ptr + sizeof(T) > end)  return false;
        std::memcpy(ptr, &value, sizeof(T));
        ptr += sizeof(T);
        return true;
    }

    template <typename T>
    bool read_pod(const std::uint8_t*& ptr, const std::uint8_t* end, T& value) {
        if (ptr + sizeof(T) > end)  return false;
        std::memcpy(&value, ptr, sizeof(T));
        ptr += sizeof(T);
        return true;
    }
} // anonymous namespace

namespace fuzzy_session {

    std::size_t Session::add(const FuzzyValue& v) {
        if (values_.size() >= kMaxSessionValues) {
            throw std::length_error("Session::add: maximum number of values reached (" + std::to_string(kMaxSessionValues) + ")");
        }
        values_.push_back(v);
        return values_.size() - 1;
    }
    std::size_t Session::add(FuzzyValue&& v) {
        if (values_.size() >= kMaxSessionValues) {
            throw std::length_error("Session::add: maximum number of values reached (" + std::to_string(kMaxSessionValues) + ")");
        }
        values_.push_back(std::move(v));
        return values_.size() - 1;
    }

    std::pair<std::size_t, std::size_t> Session::add(const std::vector<FuzzyValue>& vs) {
        if (vs.empty()) return { values_.size(), values_.size() };
        if ((values_.size() + vs.size()) > kMaxSessionValues) {
            throw std::length_error("Session::add: maximum number of values reached (" + std::to_string(kMaxSessionValues) + ")");
        }
        std::size_t first = values_.size();
        for (const auto& v : vs) {
            values_.push_back(v);
        }
        return { first, values_.size() - 1 };
    }

    std::pair<std::size_t, std::size_t> Session::add(std::vector<FuzzyValue>&& vs) {
        if (vs.empty()) return { values_.size(), values_.size() };
        if ((values_.size() + vs.size()) > kMaxSessionValues) {
            throw std::length_error("Session::add: maximum number of values reached (" + std::to_string(kMaxSessionValues) + ")");
        }
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


    std::size_t Session::serialized_size() const {
        std::size_t total = sizeof(std::uint32_t) * (values_.size() + 1); // cnt of fuzzy values + sizes
        for (const auto& v : values_) {
            total += v.size() * sizeof(double) * 2;   // xs + mus
        }
        return total;
    }

    std::size_t Session::serialize_to_buffer(std::uint8_t* buffer, std::size_t buffer_size) const {
        std::uint8_t* ptr = buffer;
        std::uint8_t* end = buffer + buffer_size;

        std::uint32_t n = static_cast<std::uint32_t>(values_.size());
        if (!write_pod(ptr, end, n)) throw std::runtime_error("serialize: buffer too small (count)");

        for (const auto& v : values_) {
            std::uint32_t size = static_cast<std::uint32_t>(v.size());
            if (!write_pod(ptr, end, size)) throw std::runtime_error("serialize: buffer too small (size)");

            for (double x : v.xs()) {
                if (!write_pod(ptr, end, x)) throw std::runtime_error("serialize: buffer too small (xs)");
            }
            for (double mu : v.mus()) {
                if (!write_pod(ptr, end, mu)) throw std::runtime_error("serialize: buffer too small (mus)");
            }
        }
        return static_cast<std::size_t>(ptr - buffer);
    }

    std::size_t Session::deserialize_from_buffer(const std::uint8_t* buffer, std::size_t buffer_size) {
        const std::uint8_t* ptr = buffer;
        const std::uint8_t* end = buffer + buffer_size;

        std::uint32_t n = 0;
        if (!read_pod(ptr, end, n)) throw std::runtime_error("deserialize: buffer too small (count)");

        std::vector<FuzzyValue> new_values;
        new_values.reserve(n);

        for (std::uint32_t k = 0; k < n; ++k) {
            std::uint32_t size = 0;
            if (!read_pod(ptr, end, size)) throw std::runtime_error("deserialize: buffer too small (size)");

            std::vector<double> xs(size), mus(size);
            for (std::uint32_t i = 0; i < size; ++i) {
                if (!read_pod(ptr, end, xs[i])) throw std::runtime_error("deserialize: buffer too small (xs)");
            }
            for (std::uint32_t i = 0; i < size; ++i) {
                if (!read_pod(ptr, end, mus[i])) throw std::runtime_error("deserialize: buffer too small (mus)");
            }
            new_values.emplace_back(xs, mus);
        }
        values_ = std::move(new_values);
        return static_cast<std::size_t>(ptr - buffer);
    }

} // namespace fuzzy_session