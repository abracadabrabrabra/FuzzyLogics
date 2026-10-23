#define RUN_TEST(fn) do { std::cout << #fn << "... "; fn(); std::cout << "OK\n"; } while(0)

#include "fuzzy_value.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

static bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

static void test_default_constructor() {
    FuzzyValue v;
    assert(v.empty());
    assert(v.size() == 0);
    assert(v.xs().empty());
    assert(v.mus().empty());
    assert(v.is_valid());
}

static void test_constructor_from_vectors() {
    std::vector<double> xs = { 1.0, 2.0, 3.0 };
    std::vector<double> mus = { 0.2, 0.5, 0.3 };

    FuzzyValue v(xs, mus);
    assert(v.size() == 3);
    assert(!v.empty());
    assert(almost_equal(v.xs()[0], 1.0));
    assert(almost_equal(v.mus()[1], 0.5));
    assert(v.is_valid());
}

static void test_size_mismatch() {
    std::vector<double> xs = { 1.0, 2.0, 3.0 };
    std::vector<double> mus = { 0.5, 0.5 }; 
    FuzzyValue v;
    v.assign(xs, mus);
    assert(!v.is_valid());
}

static void test_mu_out_of_range() {
    std::vector<double> xs = { 1.0, 2.0 };
    std::vector<double> mus = { 1.5, -0.1 };
    FuzzyValue v;
    v.assign(xs, mus);
    assert(!v.is_valid());
}

static void test_nan_mu() {
    std::vector<double> xs = { 1.0 };
    std::vector<double> mus = { std::nan("") };
    FuzzyValue v;
    v.assign(xs, mus);
    assert(!v.is_valid());
}

static void test_unordered_x() {
    std::vector<double> xs = { 3.0, 1.0, 2.0 };
    std::vector<double> mus = { 0.3, 0.3, 0.4 };
    FuzzyValue v;
    v.assign(xs, mus);
    assert(!v.is_valid());
}

static void test_add_point() {
    FuzzyValue v;
    v.add_point(1.0, 0.2);
    v.add_point(2.0, 0.5);
    v.add_point(3.0, 0.3);
    assert(v.size() == 3);
    assert(v.is_valid());
}

static void test_normalize_sum() {
    FuzzyValue v({ 1.0, 2.0, 3.0 }, { 1.0, 2.0, 1.0 });
    v.normalize(FuzzyValue::NormalizeMode::Sum);
    assert(v.is_valid());
    double sum = 0.0;
    for (double mu : v.mus()) sum += mu;
    assert(almost_equal(sum, 1.0));
}

static void test_normalize_max() {
    FuzzyValue v({ 1.0, 2.0, 3.0 }, { 1.0, 2.0, 1.0 });
    v.normalize(FuzzyValue::NormalizeMode::Max);
    assert(v.is_valid());
    double max_mu = 0.0;
    for (double mu : v.mus()) max_mu = std::max(max_mu, mu);
    assert(almost_equal(max_mu, 1.0)); 
}

static void test_prune() {
    FuzzyValue v({ 1.0, 2.0, 3.0, 4.0 }, { 0.05, 0.5, 0.3, 0.15 });
    v.prune(0.1);
    assert(v.size() == 3);
    assert(v.xs()[0] == 2.0);
    assert(v.is_valid());
}

static void test_clear() {
    FuzzyValue v({ 1.0, 2.0 }, { 0.5, 0.5 });
    v.clear();
    assert(v.empty());
    assert(v.size() == 0);
    assert(v.is_valid());
}

static void test_copy_move() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.5, 0.5 });
    FuzzyValue b = a;             
    assert(b.size() == 2);
    assert(almost_equal(b.mus()[0], 0.5));

    FuzzyValue c = std::move(a);  
    assert(c.size() == 2);
}

static void test_operator_plus() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.5, 0.5 });
    FuzzyValue b({ 10.0 }, { 1.0 });
    FuzzyValue c = a + b;
    assert(c.size() == 2);
    assert(almost_equal(c.xs()[0], 11.0));
    assert(almost_equal(c.mus()[0], 0.5));
    assert(almost_equal(c.xs()[1], 12.0));
    assert(almost_equal(c.mus()[1], 0.5));
    assert(c.is_valid());
}

static void test_operator_plus_overlapping() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.3, 0.8 });
    FuzzyValue b({ 1.0 }, { 0.6 });
    FuzzyValue c = a + b;
    assert(c.size() == 2);
    assert(almost_equal(c.xs()[0], 2.0));
    assert(almost_equal(c.mus()[0], 0.3));
    assert(almost_equal(c.xs()[1], 3.0));
    assert(almost_equal(c.mus()[1], 0.6));
}

static void test_operator_plus_empty() {
    FuzzyValue a({ 1.0 }, { 0.5 });
    FuzzyValue empty;
    FuzzyValue c = a + empty;
    assert(c.empty());
}

static void test_comparison() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.5, 0.5 });
    FuzzyValue b({ 10.0 }, { 1.0 });
    assert(a < b);
    assert(b > a);
    assert(a != b);
    assert(!(a == b));
    FuzzyValue a_copy = a;
    assert(a == a_copy);
}

static void test_comparison_equal_centroids() {
    FuzzyValue a({ 0.0, 2.0 }, { 1.0, 1.0 });
    FuzzyValue b({ 1.0 }, { 1.0 });
    assert(a == b);
    assert(!(a < b));
    assert(!(a > b));
}

static void test_centroid() {
    FuzzyValue v({ 1.0, 2.0, 3.0 }, { 0.2, 0.5, 0.3 });
    assert(almost_equal(v.centroid(), 2.1));
}

static void test_centroid_empty() {
    FuzzyValue v;
    assert(almost_equal(v.centroid(), 0.0));
}

int main() {
    RUN_TEST(test_default_constructor);
    RUN_TEST(test_constructor_from_vectors);
    RUN_TEST(test_size_mismatch);
    RUN_TEST(test_mu_out_of_range);
    RUN_TEST(test_nan_mu);
    RUN_TEST(test_unordered_x);
    RUN_TEST(test_add_point);
    RUN_TEST(test_normalize_sum);
    RUN_TEST(test_normalize_max);
    RUN_TEST(test_prune);
    RUN_TEST(test_clear);
    RUN_TEST(test_copy_move);
    RUN_TEST(test_operator_plus);
    RUN_TEST(test_operator_plus_overlapping);
    RUN_TEST(test_operator_plus_empty);
    RUN_TEST(test_comparison);
    RUN_TEST(test_comparison_equal_centroids);
    RUN_TEST(test_centroid);
    RUN_TEST(test_centroid_empty);
    

    std::cout << "All fuzzy_value tests successfully passed\n";
    return 0;
}