#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "directed_graph.hpp"
#include "graph.hpp"
#include "list_sequence.hpp"
#include "shortest_paths.hpp"

using Clock = std::chrono::steady_clock;

std::string ReadLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

char AskChar(const std::string& prompt, char default_value) {
    std::cout << prompt;
    std::string line = ReadLine();
    if (line.empty()) {
        return default_value;
    }
    return line.front();
}

template <typename T>
T AskValue(const std::string& prompt, T default_value) {
    std::cout << prompt;
    std::string line = ReadLine();
    if (line.empty()) {
        return default_value;
    }
    std::istringstream iss(line);
    T v;
    if (iss >> v) {
        return v;
    }
    return default_value;
}

uint64_t EdgeKey(size_t u, size_t v, size_t n, bool directed) {
    if (!directed && u > v) {
        std::swap(u, v);
    }
    return u * n + v;
}

SequencePtr<Edge> GenerateRandomEdges(size_t n, size_t m, bool directed, int min_w, int max_w, std::mt19937& rng) {
    const size_t max_edges = directed ? n * (n - 1) : n * (n - 1) / 2;
    if (m > max_edges) {
        throw std::invalid_argument("Too many edges requested for given vertex count");
    }
    std::uniform_int_distribution<size_t> vert_dist(0, n - 1);
    std::uniform_int_distribution<int> weight_dist(min_w, max_w);

    std::unordered_set<uint64_t> used;
    auto edges = std::make_shared<ListSequence<Edge>>();
    while (edges->GetLength() < m) {
        size_t u = vert_dist(rng);
        size_t v = vert_dist(rng);
        if (u == v) {
            continue;
        }
        uint64_t key = EdgeKey(u, v, n, directed);
        if (used.contains(key)) {
            continue;
        }
        used.insert(key);
        edges->Append({u, v, weight_dist(rng)});
    }
    return edges;
}

SequencePtr<Edge> ReadEdges(size_t n, size_t m) {
    auto edges = std::make_shared<ListSequence<Edge>>();
    std::cout << "Введите " << m << " ребер в формате: u v w (0-индексация)\n";
    for (size_t i = 0; i < m; ++i) {
        size_t u, v;
        int64_t w;
        std::cin >> u >> v >> w;
        if (u >= n || v >= n) {
            throw std::out_of_range("Вершина вне диапазона");
        }
        edges->Append({u, v, w});
    }
    return edges;
}

void PrintPath(const SequencePtr<size_t>& path) {
    if (path == nullptr) {
        std::cout << "пути нет\n";
        return;
    }
    bool first = true;
    for (auto it = path->GetIterator(); it->HasNext(); it->Next()) {
        if (!first) {
            std::cout << " -> ";
        }
        first = false;
        std::cout << it->GetCurrentItem();
    }
    std::cout << "\n";
}

template <typename Algo>
int64_t MeasureUs(Algo&& make_algo, size_t target) {
    auto start = Clock::now();
    auto algo = make_algo();
    auto dist = algo.GetDistance(target);
    auto path = algo.GetShortestPath(target);
    (void)dist;
    (void)path;
    auto end = Clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <typename Algo>
void RunAndReport(const std::string& name, Algo&& make_algo, size_t target) {
    auto start = Clock::now();
    auto algo = make_algo();
    auto dist = algo.GetDistance(target);
    auto path = algo.GetShortestPath(target);
    auto end = Clock::now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << name << ": " << micros << " мкс\n";
    std::cout << "  distance to " << target << " = " << dist << "\n";
    std::cout << "  path: ";
    PrintPath(path);
}

struct BenchResult {
    size_t n;
    size_t m;
    bool directed;
    std::string algo;
    int64_t time_us;
};

size_t ClampEdges(size_t n, size_t edges_per_vertex, bool directed) {
    size_t max_edges = directed ? n * (n - 1) : n * (n - 1) / 2;
    size_t requested = n * edges_per_vertex;
    return std::min(max_edges, requested);
}

std::vector<size_t> ReadSizes() {
    std::cout << "Размеры графов по умолчанию: 500 800 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000. "
                 "Использовать их? (Enter=да): ";
    std::string line = ReadLine();
    if (line.empty() || line[0] == 'y' || line[0] == 'Y') {
        return {500, 800, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
    }
    std::cout << "Введите размеры через пробел и завершите 0: ";
    std::vector<size_t> res;
    while (true) {
        size_t v;
        std::cin >> v;
        if (v == 0) {
            break;
        }
        res.push_back(v);
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (res.empty()) {
        res = {500, 800, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
    }
    return res;
}

void WriteCsv(const std::string& path, const std::vector<BenchResult>& rows) {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + path);
    }
    out << "n,m,directed,algo,time_us\n";
    for (const auto& r : rows) {
        out << r.n << "," << r.m << "," << (r.directed ? 1 : 0) << "," << r.algo << "," << r.time_us << "\n";
    }
}

void RunBenchmark() {
    bool directed = AskChar("Ориентированный граф? (Y/n, Enter=Y): ", 'y') == 'y';

    size_t edges_per_vertex = AskValue<size_t>("Среднее число ребер на вершину (Enter=4): ", 4);

    auto sizes = ReadSizes();

    std::cout << "Файл для CSV (по умолчанию bench.csv): ";
    std::string csv_path = ReadLine();
    if (csv_path.empty()) {
        csv_path = "bench.csv";
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    std::vector<BenchResult> results;

    for (size_t n : sizes) {
        if (n == 0) {
            continue;
        }
        size_t m = ClampEdges(n, edges_per_vertex, directed);
        auto edges = GenerateRandomEdges(n, m, directed, 1, 10, rng);
        IGraphPtr graph;
        if (directed) {
            graph = std::make_shared<DirectedGraph>(n, edges);
        } else {
            graph = std::make_shared<Graph>(n, edges);
        }
        size_t from = 0;
        size_t to = (n > 1) ? n - 1 : 0;
        try {
            int64_t t = MeasureUs([&] { return Dijkstra(graph, from); }, to);
            results.push_back({n, m, directed, "Dijkstra", t});
        } catch (const std::exception& e) {
            std::cout << "Dijkstra пропущен для n=" << n << ": " << e.what() << "\n";
        }
        try {
            int64_t t = MeasureUs([&] { return FordBellman(graph, from); }, to);
            results.push_back({n, m, directed, "Bellman-Ford", t});
        } catch (const std::exception& e) {
            std::cout << "Bellman-Ford пропущен для n=" << n << ": " << e.what() << "\n";
        }
    }

    try {
        WriteCsv(csv_path, results);
        std::cout << "CSV сохранен в " << csv_path << "\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка записи CSV: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "=== Graph shortest paths ===\n";
    char mode = AskChar("Выберите режим: (i)nteractive / (b)enchmark (Enter=i): ", 'i');

    if (mode == 'b' || mode == 'B') {
        RunBenchmark();
        return 0;
    }

    bool directed = AskChar("Ориентированный граф? (y/n, Enter=n): ", 'n') == 'y';

    mode = AskChar("Режим: (g)enerate случайный или (m)anual ввод (Enter=g): ", 'g');

    size_t n = AskValue<size_t>("Число вершин (Enter=5): ", 5);
    size_t m = AskValue<size_t>("Число ребер (Enter=5): ", 5);

    SequencePtr<Edge> edges;
    if (mode == 'g' || mode == 'G') {
        int min_w = AskValue<int>("Минимальный вес (Enter=1): ", 1);
        int max_w = AskValue<int>("Максимальный вес (Enter=10): ", 10);
        std::random_device rd;
        std::mt19937 rng(rd());
        try {
            edges = GenerateRandomEdges(n, m, directed, min_w, max_w, rng);
        } catch (const std::exception& e) {
            std::cerr << "Ошибка генерации: " << e.what() << "\n";
            return 1;
        }
    } else {
        try {
            edges = ReadEdges(n, m);
        } catch (const std::exception& e) {
            std::cerr << "Ошибка ввода: " << e.what() << "\n";
            return 1;
        }
    }

    IGraphPtr graph;
    if (directed) {
        graph = std::make_shared<DirectedGraph>(n, edges);
    } else {
        graph = std::make_shared<Graph>(n, edges);
    }

    size_t from = AskValue<size_t>("Стартовая вершина (Enter=0): ", 0);
    size_t to = AskValue<size_t>("Целевая вершина (Enter=n-1): ", n ? n - 1 : 0);

    try {
        RunAndReport(
            "Dijkstra", [&] { return Dijkstra(graph, from); }, to);
    } catch (const std::exception& e) {
        std::cout << "Dijkstra не выполнен: " << e.what() << "\n";
    }

    try {
        RunAndReport(
            "Bellman-Ford", [&] { return FordBellman(graph, from); }, to);
    } catch (const std::exception& e) {
        std::cout << "Bellman-Ford не выполнен: " << e.what() << "\n";
    }

    return 0;
}
