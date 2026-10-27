#include <iostream>
#include <cassert>
#include <sstream>
#include "fuzzy_session.h"
using namespace fuzzy_session;

static bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

static void test_add_and_get() {
    Session s;
    assert(s.empty());
    assert(s.size() == 0);

    FuzzyValue v({ 1.0, 2.0 }, { 0.5, 0.5 });
    std::size_t idx = s.add(v);
    assert(idx == 0);
    assert(s.size() == 1);
    assert(s.contains(0));
    assert(!s.contains(1));
    assert(s.get(0).size() == 2);

    Session s1;
    std::size_t idx1 = s1.add(FuzzyValue({ 1.0, 2.0 }, { 0.5, 0.5 }));
    assert(idx1 == 0);
    assert(s1.size() == 1);

    FuzzyValue v1({ 3.0 }, { 1.0 });
    std::size_t idx2 = s1.add(v);
    assert(idx2 == 1);
    assert(!v.empty());
}

static void test_add_vector() {
    Session s;
    std::vector<FuzzyValue> vs = {
        FuzzyValue({1.0}, {1.0}),
        FuzzyValue({2.0}, {1.0}),
        FuzzyValue({3.0}, {1.0})
    };
    auto indices = s.add(std::move(vs));
    assert(indices.first == 0);
    assert(indices.second == 2);

    Session s2;
    auto [f2, l2] = s2.add(std::vector<FuzzyValue>{});
    assert(f2 == 0);
    assert(l2 == 0);
    assert(s2.size() == 0);
}

static void test_get_out_of_range() {
    Session s;
    s.add(FuzzyValue({ 1.0 }, { 1.0 }));
    try {
        s.get(5);
        assert(false && "should have thrown");
    }
    catch (const std::out_of_range&) {
        // OK
    }
}

static void test_remove_and_clear() {
    Session s;
    s.add(FuzzyValue({ 1.0 }, { 1.0 }));
    s.add(FuzzyValue({ 2.0 }, { 1.0 }));
    s.add(FuzzyValue({ 3.0 }, { 1.0 }));

    assert(s.remove(1));
    assert(s.size() == 2);
    assert(s.get(0).xs()[0] == 1.0);
    assert(s.get(1).xs()[0] == 3.0);

    assert(!s.remove(10));

    s.clear();
    assert(s.empty());
}

static void test_arithmetic() {
    Session s;
    s.add(FuzzyValue({ 1.0, 2.0 }, { 0.5, 0.5 }));
    s.add(FuzzyValue({ 10.0 }, { 1.0 }));

    std::size_t idx = s.add_values(0, 1);
    assert(idx == 2);
    assert(s.size() == 3);
    assert(s.get(2).size() == 2);
    assert(almost_equal(s.get(2).xs()[0], 11.0));
    assert(almost_equal(s.get(2).xs()[1], 12.0));
}

static void test_compare() {
    Session s;
    s.add(FuzzyValue({ 1.0 }, { 1.0 }));
    s.add(FuzzyValue({ 10.0 }, { 1.0 }));

    assert(s.compare(0, 1) == CompareResult::Less);
    assert(s.compare(1, 0) == CompareResult::Greater);
    assert(s.compare(0, 0) == CompareResult::Equal);
}

static void test_save_load_roundtrip() {
    const std::string path = "test_session.txt";
    std::remove(path.c_str());

    Session s1;
    s1.add(FuzzyValue({ 1.0, 2.0 }, { 0.5, 0.5 }));
    s1.add(FuzzyValue({ 3.0 }, { 1.0 }));
    s1.save_to_file(path);

    Session s2;
    s2.load_from_file(path);
    assert(s2.size() == 2);
    assert(s2.get(0).size() == 2);
    assert(s2.get(1).size() == 1);

    std::remove(path.c_str());
}

static void test_log_summary() {
    Session s;
    s.add(FuzzyValue({ 1.0 }, { 1.0 }));
    std::ostringstream oss;
    s.log_summary(oss);
    std::string out = oss.str();
    assert(out.find("Session summary: 1 values") != std::string::npos);
    assert(out.find("[0]") != std::string::npos);
}

int main() {

    test_add_and_get();
    test_add_vector();
    test_get_out_of_range();
    test_remove_and_clear();
    test_arithmetic();
    test_compare();
    test_save_load_roundtrip();
    test_log_summary();

    std::cout << "All sessions tests successfully passed" << std::endl;

	return  0;
}