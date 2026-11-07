#include "cluster.h"
#include "poisson_generator.h"
#include <iostream>
#include <iomanip>

std::vector<FuzzyValue> make_test_data() {
    std::vector<FuzzyValue> values;
    std::mt19937_64 rng(42);

    // lambda: [4.5, 5.5] (center ~ 5)
    std::uniform_real_distribution<double> dist1(4.5, 5.5);
    for (int i = 0; i < 15; ++i) {
        values.push_back(fuzzy_generator::generate_poisson_value(
            dist1(rng), 1000, FuzzyValue::NormalizeMode::Max, 0.1, rng()));
    }

    // lambda: [14.5, 15.5] (center ~ 15)
    std::uniform_real_distribution<double> dist2(14.5, 15.5);
    for (int i = 0; i < 15; ++i) {
        values.push_back(fuzzy_generator::generate_poisson_value(
            dist2(rng), 1000, FuzzyValue::NormalizeMode::Max, 0.1, rng()));
    }

    // lambda: [24.5, 25.5] (center ~ 25)
    std::uniform_real_distribution<double> dist3(24.5, 25.5);
    for (int i = 0; i < 15; ++i) {
        values.push_back(fuzzy_generator::generate_poisson_value(
            dist3(rng), 1000, FuzzyValue::NormalizeMode::Max, 0.1, rng()));
    }

    return values;
}

int main() {

    auto test_data = make_test_data();
    fuzzy_cluster::FcmResult res = fuzzy_cluster::fuzzy_c_means(test_data, 3);

    std::cout << "Clustering result (c=3):\n";
    for (std::size_t k = 0; k < res.centers.size(); ++k) {
        std::cout << "Cluster " << k << ": center = " << res.centers[k] << "\n";
    }
    std::cout << "\nMembership matrix U (rows = values, cols = clusters):\n";

    for (std::size_t i = 0; i < test_data.size(); ++i) {
        std::cout << "Value " << i << " (centroid=" << test_data[i].centroid() << "): ";

        for (std::size_t k = 0; k < res.centers.size(); ++k) {
            std::cout << "U[" << k << "]=" << std::fixed << std::setprecision(3)
                << res.memberships[i][k] << " ";
        }
        std::cout << "\n";
    }

	return 0;
}