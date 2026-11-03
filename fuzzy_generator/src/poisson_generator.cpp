#include "poisson_generator.h"
#include <map>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <thread>


namespace {

    int poisson_sample(double lambda, std::mt19937_64& rng) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double sum_logs = 0.0;
        int k = 0;
        const int MAX_ITER = 100000;

        while (sum_logs <= lambda && k < MAX_ITER) {
            double u = dist(rng);
            if (u <= 0.0) u = std::numeric_limits<double>::min();
            sum_logs += -std::log(u);
            ++k;
        }
        return (k < MAX_ITER) ? (k - 1) : k;
    }

} // anonymous namespace

namespace fuzzy_generator {

    FuzzyValue generate_poisson_value(
        double lambda,
        std::size_t M,
        FuzzyValue::NormalizeMode mode,
        double prune_threshold,
        std::uint64_t seed)
    {
        std::mt19937_64 rng(seed);
        return generate_poisson_value(lambda, M, mode, prune_threshold, rng);
    }

    FuzzyValue generate_poisson_value(
        double lambda,
        std::size_t M,
        FuzzyValue::NormalizeMode mode,
        double prune_threshold,
        std::mt19937_64& rng)
    {
        if (!(lambda > 0.0) || !std::isfinite(lambda)) {
            throw std::invalid_argument("generate_poisson_value: lambda must be finite and > 0");
        }
        if (M == 0) {
            throw std::invalid_argument("generate_poisson_value: M must be > 0");
        }
        if (M > 10'000'000) {
            throw std::invalid_argument("generate_poisson_value: M too large (max 10M)");
        }
        if (prune_threshold < 0.0 || prune_threshold > 1.0) {
            throw std::invalid_argument("generate_poisson_value: prune_threshold must be in [0, 1]");
        }

        std::vector<int> samples(M);
        for (std::size_t i = 0; i < M; ++i) {
            samples[i] = poisson_sample(lambda, rng);
        }
        std::sort(samples.begin(), samples.end());

        std::vector<double> xs;
        std::vector<double> mus;
        xs.reserve(50);
        mus.reserve(50);

        std::size_t i = 0;
        while (i < M) {
            int k = samples[i];
            std::size_t j = i;
            while (j < M && samples[j] == k) ++j;
            xs.push_back(static_cast<double>(k));
            mus.push_back(static_cast<double>(j - i) / static_cast<double>(M));
            i = j;
        }

        FuzzyValue result(xs, mus);
        result.normalize(mode);
        result.prune(prune_threshold);

        return result;
    }

    std::vector<FuzzyValue> generate_poisson_set_parallel(
        std::size_t n, double lambda_min, double lambda_max,
        std::size_t M, FuzzyValue::NormalizeMode mode,
        double prune_threshold, std::uint64_t seed)
    {

        unsigned T = std::thread::hardware_concurrency();
        if (T == 0) T = 4;
        if (T > n) T = static_cast<unsigned>(n);

        std::vector<std::vector<FuzzyValue>> parts(T);
        std::vector<std::thread> threads;
        threads.reserve(T);

        for (unsigned t = 0; t < T; ++t) {
            std::size_t begin = n * t / T;
            std::size_t end = n * (t + 1) / T;

            threads.emplace_back([&, t, begin, end]() {
                std::mt19937_64 rng(seed + t);
                std::uniform_real_distribution<double> lambda_dist(lambda_min, lambda_max);
                parts[t].reserve(end - begin);

                for (std::size_t i = begin; i < end; ++i) {
                    double lambda = lambda_dist(rng);
                    parts[t].push_back(generate_poisson_value(lambda, M, mode, prune_threshold, rng));
                }
                });
        }

        for (auto& th : threads) th.join();

        std::vector<FuzzyValue> result;
        result.reserve(n);
        for (auto& part : parts) {
            for (auto& v : part) {
                result.push_back(std::move(v));
            }
        }
        return result;
    }

    std::vector<FuzzyValue> generate_poisson_set(
        std::size_t n,
        double lambda_min,
        double lambda_max,
        std::size_t M,
        FuzzyValue::NormalizeMode mode,
        double prune_threshold,
        std::uint64_t seed)
    {
        if (n == 0) {
            throw std::invalid_argument("generate_poisson_set: n must be > 0");
        }
        if (!(lambda_min > 0.0) || !std::isfinite(lambda_min)) {
            throw std::invalid_argument("generate_poisson_set: lambda_min must be finite and > 0");
        }
        if (!(lambda_max >= lambda_min) || !std::isfinite(lambda_max)) {
            throw std::invalid_argument("generate_poisson_set: lambda_max must be >= lambda_min");
        }

        unsigned hw = std::thread::hardware_concurrency();
        if (hw > 1 && n * M >= 100'000) {
            return generate_poisson_set_parallel(n, lambda_min, lambda_max, M, mode, prune_threshold, seed);
        }

        std::mt19937_64 rng(seed);
        std::uniform_real_distribution<double> lambda_dist(lambda_min, lambda_max);

        std::vector<FuzzyValue> result;
        result.reserve(n);

        for (std::size_t i = 0; i < n; ++i) {
            double lambda = lambda_dist(rng);
            result.push_back(generate_poisson_value(lambda, M, mode, prune_threshold, rng));
        }

        return result;
    }

} // namespace fuzzy_generator
