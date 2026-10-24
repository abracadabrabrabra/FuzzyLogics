#include "poisson_generator.h"
#include "fuzzy_value.h"
#include <cassert>
#include <chrono>
#include <iostream>

using namespace fuzzy_generator;

int main() {

    std::cout << "Try generate fuzzy value with invalid params\n";
    try {
        FuzzyValue u = generate_poisson_value(-10.0, 1000);
    }
    catch (const std::exception& e) {
        std::cout << "Catched: " << e.what() << std::endl;
    }

    FuzzyValue v = generate_poisson_value(5.0, 1000);

    assert(!v.empty());
    assert(v.is_valid());
    assert(v.centroid() > 0.0);

    std::cout << "Generated value: size = " << v.size()
        << ", centroid = " << v.centroid() << "\n";

    auto start = std::chrono::steady_clock::now();
    auto set = generate_poisson_set(50, 1.0, 10.0, 1000);
    auto end = std::chrono::steady_clock::now();
    assert(set.size() == 50);


    for (const auto& fv : set) {
        assert(fv.is_valid());
    }

    std::chrono::duration<double> elapsed = end - start; // 0.04 second
    std::cout << "Generated set of " << set.size() << " values, time = " << elapsed.count() << " seconds\n";

    auto v1 = generate_poisson_value(5.0, 1000, FuzzyValue::NormalizeMode::Max, 0.1, 42);
    auto v2 = generate_poisson_value(5.0, 1000, FuzzyValue::NormalizeMode::Max, 0.1, 42);
    assert(v1.size() == v2.size());
    for (std::size_t i = 0; i < v1.size(); ++i) {
        assert(std::fabs(v1.xs()[i] - v2.xs()[i]) < 1e-12);
        assert(std::fabs(v1.mus()[i] - v2.mus()[i]) < 1e-12);
    }
    std::cout << "Reproducibility test passed\n";


    double lambda_test = 5.0;
    auto v_lambda = generate_poisson_value(lambda_test, 10000, FuzzyValue::NormalizeMode::Max, 0.1, 42);
    double rel_error = std::fabs(v_lambda.centroid() - lambda_test) / lambda_test;
    assert(rel_error < 0.1);
    std::cout << "Centroid ~ lambda: centroid = " << v_lambda.centroid()
        << ", lambda = " << lambda_test << ", rel_error = " << rel_error << "\n";

    std::cout << "All fuzzy_generator tests passed\n";

    return 0;
}