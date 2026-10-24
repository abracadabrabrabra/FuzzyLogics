#pragma once

#include "fuzzy_value.h"
#include <string>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>

namespace fuzzy_io {

    std::string to_string(const FuzzyValue& v);

    std::string to_csv(const FuzzyValue& v);

    void save_to_stream(const FuzzyValue& v, std::ostream& out);
    FuzzyValue load_from_stream(std::istream& in);

    void save_to_file(const FuzzyValue& v, const std::string& path);
    FuzzyValue load_from_file(const std::string& path);

    void log_to_file(const std::string& msg, std::ostream& out);

    FuzzyValue read_from_console(std::istream& in = std::cin, std::ostream& out = std::cout);

} // namespace fuzzy_io