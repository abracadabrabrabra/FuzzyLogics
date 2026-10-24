#include "fuzzy_io.h"
#include <cstddef>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

namespace fuzzy_io {

	std::string to_string(const FuzzyValue& v) {
		std::stringstream oss;
		std::size_t v_size = v.size();
		const auto& xs = v.xs();
		const auto& mus = v.mus();
		oss << std::setprecision(std::numeric_limits<double>::max_digits10);
		oss << "Fuzzy value: size = " << v_size << ", centroid = " << v.centroid() << "\n";
		for (size_t i = 0; i < v_size; ++i)
			oss << "(x = " << xs[i] << ", mu = " << mus[i] << ")\n";
		oss << "End of fuzzy value\n";
		return oss.str();
	}

	std::string to_csv(const FuzzyValue& v) {
		std::ostringstream oss;
		oss << std::setprecision(std::numeric_limits<double>::max_digits10);
		const auto& xs = v.xs();
		const auto& mus = v.mus();
		oss << "x,mu\n";
		for (std::size_t i = 0; i < xs.size(); ++i) {
			oss << xs[i] << "," << mus[i] << "\n";
		}
		return oss.str();
	}

	void save_to_stream(const FuzzyValue& v, std::ostream& out) {
		out << std::setprecision(std::numeric_limits<double>::max_digits10);
		const auto& xs = v.xs();
		const auto& mus = v.mus();
		out << xs.size() << "\n";
		for (std::size_t i = 0; i < xs.size(); ++i) {
			out << xs[i] << " " << mus[i] << "\n";
		}
	}

	FuzzyValue load_from_stream(std::istream& in) {
		std::size_t n = 0;
		if (!(in >> n)) {
			throw std::invalid_argument("load_from_stream: cannot read size");
		}
		std::vector<double> xs, mus;
		xs.reserve(n);
		mus.reserve(n);
		for (std::size_t i = 0; i < n; ++i) {
			double x, mu;
			if (!(in >> x >> mu)) {
				throw std::invalid_argument("load_from_stream: cannot read point " + std::to_string(i));
			}
			xs.push_back(x);
			mus.push_back(mu);
		}
		FuzzyValue v(xs, mus);
		if (!v.is_valid()) {
			throw std::invalid_argument("load_from_stream: loaded value is invalid");
		}
		return v;
	}

	void log_to_file(const std::string& msg, std::ostream& out) {
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		out << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
			<< msg << "\n";
	}

	void save_to_file(const FuzzyValue& v, const std::string& path) {
		std::ofstream out(path);
		if (!out) {
			throw std::runtime_error("save_to_file: cannot open " + path);
		}
		save_to_stream(v, out);
	}

	FuzzyValue load_from_file(const std::string& path) {
		std::ifstream in(path);
		if (!in) {
			throw std::runtime_error("load_from_file: cannot open " + path);
		}
		return load_from_stream(in);
	}

	FuzzyValue read_from_console(std::istream& in, std::ostream& out) {
		out << "Enter number of points: ";
		std::size_t n = 0;
		if (!(in >> n)) {
			throw std::invalid_argument("read_from_console: cannot read size");
		}
		std::vector<double> xs, mus;
		xs.reserve(n);
		mus.reserve(n);
		for (std::size_t i = 0; i < n; ++i) {
			double x, mu;
			out << "Point " << (i + 1) << " (x mu): ";
			if (!(in >> x >> mu)) {
				throw std::invalid_argument("read_from_console: cannot read point " + std::to_string(i));
			}
			xs.push_back(x);
			mus.push_back(mu);
		}
		return FuzzyValue(xs, mus);
	}

}//namespace fuzzy_io