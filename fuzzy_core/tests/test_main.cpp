#define RUN_TEST(fn) do { std::cout << #fn << "... "; fn(); std::cout << "OK\n"; } while(0)

#include "fuzzy_value.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

static bool almost_equal(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

//Creation, state

static void test_empty_state() {
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

static void test_add_point() {
    FuzzyValue v;
    v.add_point(1.0, 0.2);
    v.add_point(2.0, 0.5);
    v.add_point(3.0, 0.3);
    assert(v.size() == 3);
    assert(v.is_valid());
}

static void test_clear() {
    FuzzyValue v({ 1.0, 2.0 }, { 0.5, 0.5 });
    v.clear();
    assert(v.empty());
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

//Is_valid

static void test_validation_errors() {
    FuzzyValue v1;
    v1.assign({ 1.0, 2.0, 3.0 }, { 0.5, 0.5 });
    assert(!v1.is_valid());

    FuzzyValue v2;
    v2.assign({ 1.0, 2.0 }, { 1.5, -0.1 });
    assert(!v2.is_valid());

    FuzzyValue v3;
    v3.assign({ 1.0 }, { std::nan("") });
    assert(!v3.is_valid());

    FuzzyValue v4;
    v4.assign({ 3.0, 1.0, 2.0 }, { 0.3, 0.3, 0.4 });
    assert(!v4.is_valid());
}

//Pruning, normalization

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

// Arithmetical operators

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

static void test_operator_minus() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.5, 0.5 });
    FuzzyValue b({ 10.0 }, { 1.0 });
    FuzzyValue c = a - b;
    assert(c.size() == 2);
    assert(almost_equal(c.xs()[0], -9.0));
    assert(almost_equal(c.mus()[0], 0.5));
    assert(almost_equal(c.xs()[1], -8.0));
    assert(almost_equal(c.mus()[1], 0.5));
    assert(c.is_valid());
}

static void test_operator_multiply() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.5, 0.5 });
    FuzzyValue b({ 3.0 }, { 1.0 });
    FuzzyValue c = a * b;
    assert(c.size() == 2);
    assert(almost_equal(c.xs()[0], 3.0));
    assert(almost_equal(c.mus()[0], 0.5));
    assert(almost_equal(c.xs()[1], 6.0));
    assert(almost_equal(c.mus()[1], 0.5));
    assert(c.is_valid());
}

static void test_operator_divide() {
    FuzzyValue a({ 2.0, 4.0 }, { 0.5, 0.5 });
    FuzzyValue b({ 2.0 }, { 1.0 });
    FuzzyValue c = a / b;
    assert(c.size() == 2);
    assert(almost_equal(c.xs()[0], 1.0));
    assert(almost_equal(c.mus()[0], 0.5));
    assert(almost_equal(c.xs()[1], 2.0));
    assert(almost_equal(c.mus()[1], 0.5));
    assert(c.is_valid());
}

static void test_operator_divide_by_zero() {
    FuzzyValue a({ 1.0, 2.0 }, { 0.5, 0.5 });
    FuzzyValue b({ 0.0, 1.0 }, { 1.0, 1.0 });
    FuzzyValue c = a / b;

    assert(c.size() == 2);
    assert(almost_equal(c.xs()[0], 1.0));
    assert(almost_equal(c.xs()[1], 2.0));
    assert(c.is_valid());
}

// Comparsion

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

// Launch

static void run_creation_tests() {
    std::cout << "--- Creation & state ---\n";
    RUN_TEST(test_empty_state);
    RUN_TEST(test_constructor_from_vectors);
    RUN_TEST(test_add_point);
    RUN_TEST(test_clear);
    RUN_TEST(test_copy_move);
}

static void run_validation_tests() {
    std::cout << "--- Validation ---\n";
    RUN_TEST(test_validation_errors);
}

static void run_normalization_tests() {
    std::cout << "--- Normalization & pruning ---\n";
    RUN_TEST(test_normalize_sum);
    RUN_TEST(test_normalize_max);
    RUN_TEST(test_prune);
}

static void run_arithmetic_tests() {
    std::cout << "--- Arithmetic ---\n";
    RUN_TEST(test_operator_plus);
    RUN_TEST(test_operator_plus_overlapping);
    RUN_TEST(test_operator_plus_empty);
    RUN_TEST(test_operator_minus);
    RUN_TEST(test_operator_multiply);
    RUN_TEST(test_operator_divide);
    RUN_TEST(test_operator_divide_by_zero);
}

static void run_comparison_tests() {
    std::cout << "--- Comparison & centroid ---\n";
    RUN_TEST(test_comparison);
    RUN_TEST(test_comparison_equal_centroids);
    RUN_TEST(test_centroid);
    RUN_TEST(test_centroid_empty);
}

int main() {
    run_creation_tests();
    run_validation_tests();
    run_normalization_tests();
    run_arithmetic_tests();
    run_comparison_tests();

    std::cout << "\nAll fuzzy_value tests successfully passed\n";
    return 0;
}