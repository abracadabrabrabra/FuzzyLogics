#include "cluster.h"

#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <stdexcept>

namespace {

    double distance(double centroid, double center) {
        return std::fabs(centroid - center);
    }

    //update U: U[i][k] = 1 / sum_j ( (d_ik / d_ij)^(2/(m-1)) )
    void update_memberships(
        const std::vector<double>& centroids,
        const std::vector<double>& centers,
        std::vector<std::vector<double>>& U,
        double m)
    {
        std::size_t n = centroids.size();
        std::size_t c = centers.size();
        double exponent = 2.0 / (m - 1.0);

        for (std::size_t i = 0; i < n; ++i) {
            double sum = 0.0;
            std::vector<double> d(c);

            for (std::size_t k = 0; k < c; ++k) {
                d[k] = distance(centroids[i], centers[k]);
                if (d[k] < 1e-12) d[k] = 1e-12;
                sum += std::pow(1.0 / d[k], exponent);
            }

            for (std::size_t k = 0; k < c; ++k) {
                U[i][k] = std::pow(1.0 / d[k], exponent) / sum;
            }
        }
    }

    //update centers: center_k = sum_i ( U[i][k]^m * centroid_i ) / sum_i ( U[i][k]^m )
    void update_centers(
        const std::vector<double>& centroids,
        const std::vector<std::vector<double>>& U,
        std::vector<double>& centers,
        double m)
    {
        std::size_t n = centroids.size();
        std::size_t c = centers.size();

        for (std::size_t k = 0; k < c; ++k) {
            double numerator = 0.0;
            double denominator = 0.0;

            for (std::size_t i = 0; i < n; ++i) {
                double weight = std::pow(U[i][k], m);
                numerator += weight * centroids[i];
                denominator += weight;
            }

            if (denominator > 1e-12) {
                centers[k] = numerator / denominator;
            }
            else {
                // cluster empty
            }
        }
    }

    double max_delta(
        const std::vector<std::vector<double>>& U_old,
        const std::vector<std::vector<double>>& U_new)
    {
        double max_diff = 0.0;
        for (std::size_t i = 0; i < U_old.size(); ++i) {
            for (std::size_t k = 0; k < U_old[i].size(); ++k) {
                max_diff = std::max(max_diff, std::fabs(U_old[i][k] - U_new[i][k]));
            }
        }
        return max_diff;
    }
}//anonymous namespace

namespace fuzzy_cluster {

    FcmResult fuzzy_c_means(
        const std::vector<FuzzyValue>& values,
        std::size_t num_clusters,
        double m,
        std::size_t max_iter,
        double epsilon,
        std::uint64_t seed)
    {
        if (values.empty()) {
            throw std::invalid_argument("fuzzy_c_means: values must not be empty");
        }
        if (num_clusters == 0 || num_clusters > values.size()) {
            throw std::invalid_argument("fuzzy_c_means: invalid number of clusters");
        }
        if (m <= 1.0) {
            throw std::invalid_argument("fuzzy_c_means: fuzziness m must be > 1.0");
        }

        std::size_t n = values.size();
        std::size_t c = num_clusters;

        std::vector<double> centroids(n);
        for (std::size_t i = 0; i < n; ++i) {
            centroids[i] = values[i].centroid();
        }

        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> dist(0.01, 1.0); // [0.01, 1.0] 

        std::vector<std::vector<double>> U(n, std::vector<double>(c));
        for (std::size_t i = 0; i < n; ++i) {
            double sum = 0.0;
            for (std::size_t k = 0; k < c; ++k) {
                U[i][k] = dist(rng);
                sum += U[i][k];
            }
            for (std::size_t k = 0; k < c; ++k) {
                U[i][k] /= sum;
            }
        }

        std::vector<double> centers(c, 0.0);
        update_centers(centroids, U, centers, m);

        std::vector<std::vector<double>> U_old = U;
        std::size_t iter = 0;
        double delta = std::numeric_limits<double>::max();

        while (iter < max_iter && delta > epsilon) {
            U_old = U;

            update_memberships(centroids, centers, U, m);

            update_centers(centroids, U, centers, m);

            delta = max_delta(U_old, U);
            ++iter;
        }

        FcmResult result;
        result.centers = std::move(centers);
        result.memberships = std::move(U);
        return result;
    }
}//namespace fuzzy_cluster