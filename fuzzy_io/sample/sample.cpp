#include "fuzzy_io.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdexcept>

using namespace fuzzy_io;

static bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

static bool values_equal(const FuzzyValue& a, const FuzzyValue& b, double eps = 1e-9) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!almost_equal(a.xs()[i], b.xs()[i], eps)) return false;
        if (!almost_equal(a.mus()[i], b.mus()[i], eps)) return false;
    }
    return true;
}

static void test_to_string() {
    FuzzyValue v({ 1.0, 2.0, 3.0 }, { 0.2, 0.5, 0.3 });
    std::string s = fuzzy_io::to_string(v);
    assert(!s.empty());
    assert(s.find("size = 3") != std::string::npos);
    assert(s.find("centroid") != std::string::npos);
    assert(s.find("x = 1") != std::string::npos);
    assert(s.find("mu = 0.5") != std::string::npos);
    assert(s.find("End of fuzzy value") != std::string::npos);
}

static void test_to_string_empty() {
    FuzzyValue v;
    std::string s = fuzzy_io::to_string(v);
    assert(s.find("size = 0") != std::string::npos);
}


static void test_to_csv() {
    FuzzyValue v({ 1.0, 2.0, 3.0 }, { 0.2, 0.5, 0.3 });
    std::string s = fuzzy_io::to_csv(v);

    assert(!s.empty());
    assert(s.find("x,mu") != std::string::npos);

    std::istringstream in(s);
    std::string header;
    std::getline(in, header);
    assert(header == "x,mu");

    std::vector<double> xs, mus;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto comma = line.find(',');
        assert(comma != std::string::npos);
        double x = std::stod(line.substr(0, comma));
        double mu = std::stod(line.substr(comma + 1));
        xs.push_back(x);
        mus.push_back(mu);
    }

    assert(xs.size() == 3);
    assert(almost_equal(xs[0], 1.0));
    assert(almost_equal(xs[1], 2.0));
    assert(almost_equal(xs[2], 3.0));
    assert(almost_equal(mus[0], 0.2));
    assert(almost_equal(mus[1], 0.5));
    assert(almost_equal(mus[2], 0.3));
}

static void test_to_csv_empty() {
    FuzzyValue v;
    std::string s = fuzzy_io::to_csv(v);
    assert(s.find("x,mu") != std::string::npos);
}

static void test_stream_roundtrip() {
    FuzzyValue original({ 1.0, 2.0, 3.0 }, { 0.2, 0.5, 0.3 });

    std::ostringstream out;
    fuzzy_io::save_to_stream(original, out);

    std::istringstream in(out.str());
    FuzzyValue loaded = fuzzy_io::load_from_stream(in);

    assert(values_equal(original, loaded));
}

static void test_stream_roundtrip_high_precision() {
    FuzzyValue original({ 0.123456789012345, 1.987654321098765 },
        { 0.333333333333333, 0.666666666666666 });

    std::ostringstream out;
    fuzzy_io::save_to_stream(original, out);
    std::istringstream in(out.str());
    FuzzyValue loaded = fuzzy_io::load_from_stream(in);

    assert(values_equal(original, loaded, 1e-12));
}

static void test_load_from_stream_invalid_size() {
    std::istringstream in("abc\n");
    try {
        fuzzy_io::load_from_stream(in);
        assert(false && "should have thrown");
    }
    catch (const std::invalid_argument&) {
        // OK
    }
}

static void test_load_from_stream_invalid_point() {
    std::istringstream in("3\n1.0 0.2\n2.0 0.5\n");
    try {
        fuzzy_io::load_from_stream(in);
        assert(false && "should have thrown");
    }
    catch (const std::invalid_argument&) {
        // OK
    }
}

static void test_load_from_stream_invalid_value() {
    std::istringstream in("2\n1.0 0.2\n2.0 1.5\n");
    try {
        fuzzy_io::load_from_stream(in);
        assert(false && "should have thrown");
    }
    catch (const std::invalid_argument&) {
        // OK
    }
}


static void test_file_roundtrip() {
    const std::string path = "test_fuzzy_value.txt";
    std::remove(path.c_str());

    FuzzyValue original({ 1.0, 2.0, 3.0 }, { 0.2, 0.5, 0.3 });
    fuzzy_io::save_to_file(original, path);
    FuzzyValue loaded = fuzzy_io::load_from_file(path);
    assert(values_equal(original, loaded));

    std::remove(path.c_str());
}

static void test_load_from_file_missing() {
    try {
        fuzzy_io::load_from_file("non_existent_file.txt");
        assert(false && "should have thrown");
    }
    catch (const std::runtime_error&) {
        // OK
    }
}

static void test_log_to_ostream() {
    std::ostringstream oss;
    fuzzy_io::log_to_file("test message", oss);
    std::string s = oss.str();

    assert(!s.empty());
    assert(s.find("[") == 0);
    assert(s.find("test message") != std::string::npos);
    assert(s.find("]") != std::string::npos);
}

static void test_log_to_real_file() {
    const std::string path = "test_fuzzy_io.log";
    std::remove(path.c_str());

    {
        std::ofstream out(path, std::ios::app);
        assert(out.is_open());
        fuzzy_io::log_to_file("first message", out);
        fuzzy_io::log_to_file("second message", out);
    }

    std::ifstream in(path);
    assert(in.is_open());
    std::string line1, line2;
    std::getline(in, line1);
    std::getline(in, line2);
    assert(line1.find("first message") != std::string::npos);
    assert(line2.find("second message") != std::string::npos);

    std::remove(path.c_str());
}

static void test_read_from_console() {
    std::istringstream fake_input("3\n1.0 0.2\n2.0 0.5\n3.0 0.3\n");
    std::ostringstream fake_output;

    FuzzyValue v = fuzzy_io::read_from_console(fake_input, fake_output);

    assert(v.size() == 3);
    assert(v.is_valid());
    assert(almost_equal(v.xs()[0], 1.0));
    assert(almost_equal(v.xs()[1], 2.0));
    assert(almost_equal(v.xs()[2], 3.0));
    assert(almost_equal(v.mus()[0], 0.2));
    assert(almost_equal(v.mus()[1], 0.5));
    assert(almost_equal(v.mus()[2], 0.3));

    std::string out = fake_output.str();
    assert(out.find("Enter number of points") != std::string::npos);
    assert(out.find("Point 1") != std::string::npos);
}

static void test_read_from_console_empty() {
    std::istringstream fake_input("0\n");
    std::ostringstream fake_output;

    FuzzyValue v = fuzzy_io::read_from_console(fake_input, fake_output);
    assert(v.empty());
    assert(v.is_valid());
}

static void test_read_from_console_invalid_input() {
    std::istringstream fake_input("abc\n"); 
    std::ostringstream fake_output;

    try {
        fuzzy_io::read_from_console(fake_input, fake_output);
        assert(false && "should have thrown");
    }
    catch (const std::invalid_argument&) {
        // OK
    }
}

static void test_full_roundtrip() {
    FuzzyValue original({ 1.0, 2.0, 3.0 }, { 0.2, 0.5, 0.3 });

    std::ostringstream saved;
    fuzzy_io::save_to_stream(original, saved);

    std::istringstream in(saved.str());
    FuzzyValue loaded = fuzzy_io::load_from_stream(in);

    assert(values_equal(original, loaded));
    std::string s = fuzzy_io::to_string(loaded);
    assert(!s.empty());
}

int main() {
    test_to_string();
    test_to_string_empty();
    test_to_csv();
    test_to_csv_empty();
    test_stream_roundtrip();
    test_stream_roundtrip_high_precision();
    test_load_from_stream_invalid_size();
    test_load_from_stream_invalid_point();
    test_load_from_stream_invalid_value();
    test_file_roundtrip();
    test_load_from_file_missing();
    test_log_to_ostream();
    test_log_to_real_file();
    test_read_from_console();
    test_read_from_console_empty();
    test_read_from_console_invalid_input();
    test_full_roundtrip();

    std::cout << "All fuzzy_io tests successfully passed\n";
    return 0;
}