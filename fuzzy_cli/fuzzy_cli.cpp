#include "fuzzy_session.h"
#include "poisson_generator.h"
#include "fuzzy_io.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace fuzzy_session;

static const char* compare_to_string(CompareResult r) {
    switch (r) {
    case CompareResult::Less:    return "<";
    case CompareResult::Greater: return ">";
    case CompareResult::Equal:   return "==";
    }
    return "?";
}

static void print_help() {
    std::cout <<
        "Commands:\n"
        "  help                              - show this help\n"
        "  exit                              - exit (session is NOT auto-saved)\n"
        "  list                              - list all values (index, size, centroid)\n"
        "  print <i>                         - print value i in full\n"
        "  read                              - read one value from console\n"
        "  generate <n> <lmin> <lmax> <M>    - generate n Poisson values\n"
        "  add <i> <j>                       - value i + value j -> new value\n"
        "  sub <i> <j>                       - value i - value j -> new value\n"
        "  mul <i> <j>                       - value i * value j -> new value\n"
        "  div <i> <j>                       - value i / value j -> new value\n"
        "  compare <i> <j> [eps]             - compare centroids (default eps=1e-9)\n"
        "  centroid <i>                      - print centroid of value i\n"
        "  save <path>                       - save session to file\n"
        "  load <path>                       - load session from file (appends)\n"
        "  log <path>                        - write session summary to log file\n"
        "  clear                             - clear session\n"
        "  remove <i>                        - remove value i\n";
}

static bool execute_command(const std::string& line, Session& session) {
    std::istringstream iss(line);
    std::string cmd;
    if (!(iss >> cmd)) return true;

    try {
        if (cmd == "help") {
            print_help();
        }
        else if (cmd == "exit") {
            return false;
        }
        else if (cmd == "list") {
            session.log_summary(std::cout);
        }
        else if (cmd == "print") {
            std::size_t i;
            if (!(iss >> i)) { std::cout << "Usage: print <i>\n"; return true; }
            std::cout << fuzzy_io::to_string(session.get(i));
        }
        else if (cmd == "read") {
            FuzzyValue v = fuzzy_io::read_from_console(std::cin, std::cout);
            std::size_t idx = session.add(std::move(v));
            std::cout << "Added as [" << idx << "]\n";
        }
        else if (cmd == "generate") {
            std::size_t n, M;
            double lmin, lmax;
            if (!(iss >> n >> lmin >> lmax >> M)) {
                std::cout << "Usage: generate <n> <lmin> <lmax> <M>\n";
                return true;
            }
            auto set = fuzzy_generator::generate_poisson_set(n, lmin, lmax, M);
            auto [first, last] = session.add(std::move(set));
            std::cout << "Generated " << n << " values: [" << first << ".." << last << "]\n";
        }
        else if (cmd == "add" || cmd == "sub" || cmd == "mul" || cmd == "div") {
            std::size_t i, j;
            if (!(iss >> i >> j)) {
                std::cout << "Usage: " << cmd << " <i> <j>\n";
                return true;
            }
            std::size_t idx = 0;
            if (cmd == "add") idx = session.add_values(i, j);
            else if (cmd == "sub") idx = session.sub_values(i, j);
            else if (cmd == "mul") idx = session.mul_values(i, j);
            else idx = session.div_values(i, j);
            std::cout << "Result added as [" << idx << "]\n";
        }
        else if (cmd == "compare") {
            std::size_t i, j;
            double eps = 1e-9;
            if (!(iss >> i >> j)) { std::cout << "Usage: compare <i> <j> [eps]\n"; return true; }
            iss >> eps;
            auto r = session.compare(i, j, eps);
            std::cout << " " << i << " " << compare_to_string(r) << " " << j << "\n";
        }
        else if (cmd == "centroid") {
            std::size_t i;
            if (!(iss >> i)) { std::cout << "Usage: centroid <i>\n"; return true; }
            std::cout << "centroid[" << i << "] = " << session.centroid(i) << "\n";
        }
        else if (cmd == "save") {
            std::string path;
            if (!(iss >> path)) { std::cout << "Usage: save <path>\n"; return true; }
            session.save_to_file(path);
            std::cout << "Saved to " << path << "\n";
        }
        else if (cmd == "load") {
            std::string path;
            if (!(iss >> path)) { std::cout << "Usage: load <path>\n"; return true; }
            session.load_from_file(path);
            std::cout << "Loaded from " << path << " (session size = " << session.size() << ")\n";
        }
        else if (cmd == "log") {
            std::string path;
            if (!(iss >> path)) { std::cout << "Usage: log <path>\n"; return true; }
            std::ofstream out(path, std::ios::app);
            if (!out) { std::cout << "Cannot open " << path << "\n"; return true; }
            fuzzy_io::log_to_file("session summary", out);
            session.log_summary(out);
            std::cout << "Logged to " << path << "\n";
        }
        else if (cmd == "clear") {
            session.clear();
            std::cout << "Session cleared\n";
        }
        else if (cmd == "remove") {
            std::size_t i;
            if (!(iss >> i)) { std::cout << "Usage: remove <i>\n"; return true; }
            if (session.remove(i)) std::cout << "Removed [" << i << "]\n";
            else std::cout << "Index out of range\n";
        }
        else {
            std::cout << "Unknown command: " << cmd << ". Type 'help' for list.\n";
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    return true;
}

int main() {
    Session session;
    std::string line;
    std::cout << "fuzzy_cli: type 'help' for commands, 'exit' to quit\n";
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break; // EOF
        if (!execute_command(line, session)) break;
    }
    std::cout << "Bye.\n";
    return 0;
}