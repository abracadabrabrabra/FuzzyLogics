#pragma once

#include "fuzzy_value.h"
#include <vector>
#include <cstddef>
#include <string>
#include <cstdint>

namespace fuzzy_session {

    enum class CompareResult {
        Less,
        Greater,
        Equal //by eps
    };

    class Session {
    public:
        Session() = default;

        std::size_t add(const FuzzyValue& v);
        std::size_t add(FuzzyValue&& v);
        std::pair<std::size_t, std::size_t> add(const std::vector<FuzzyValue>& vs);
        std::pair<std::size_t, std::size_t> add(std::vector<FuzzyValue>&& vs);
        bool remove(std::size_t index);
        void clear() noexcept;
        std::size_t size() const noexcept;
        bool empty() const noexcept;
        bool contains(std::size_t index) const noexcept;

        const FuzzyValue& get(std::size_t index) const;
        FuzzyValue& get(std::size_t index);

        std::size_t add_values(std::size_t i, std::size_t j);
        std::size_t sub_values(std::size_t i, std::size_t j);
        std::size_t mul_values(std::size_t i, std::size_t j);
        std::size_t div_values(std::size_t i, std::size_t j);

        CompareResult compare(std::size_t i, std::size_t j, double eps = 1e-9) const;
        double centroid(std::size_t i) const;

        void save_to_file(const std::string& path) const;
        void load_from_file(const std::string& path);
        void log_summary(std::ostream& out) const;

        std::size_t serialized_size() const;
        std::size_t serialize_to_buffer(std::uint8_t* buffer, std::size_t buffer_size) const;
        std::size_t deserialize_from_buffer(const std::uint8_t* buffer, std::size_t buffer_size);

    private:
        std::vector<FuzzyValue> values_;
    };

} // namespace fuzzy_session