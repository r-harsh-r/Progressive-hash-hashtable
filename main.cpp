#include "hashtable.h" // Your HashTable implementation
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map> // For the "golden model"
#include <cassert>       // For assert()
#include <random>        // For random number generation
#include <chrono>        // For performance timing
#include <numeric>       // For std::accumulate
#include <algorithm>     // For std::max_element
#include <type_traits>

// Helper to convert an integer (0..N) to a key of type K
template <typename K>
K make_key_from_index(int i) {
    if constexpr (std::is_integral<K>::value) {
        return static_cast<K>(i);
    } else {
        return std::string("key") + std::to_string(i);
    }
}

// Helper to create a value of type V from an index
template <typename V>
V make_value_from_index(int i) {
    if constexpr (std::is_integral<V>::value) {
        return static_cast<V>(i);
    } else {
        return std::string("val") + std::to_string(i);
    }
}

// Templated correctness fuzz test using unordered_map as golden model
template <typename K, typename V>
void runCorrectnessTest_T(const std::string &label, int NUM_OPERATIONS = 50000, int KEY_POOL = 50000) {
    std::cout << "--- Correctness Fuzz Test for " << label << " ---" << std::endl;
    HashTable<K, V> myTable;
    std::unordered_map<K, V> goldenMap;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> op_dist(0, 2); // 0: Insert, 1: Find, 2: Remove
    std::uniform_int_distribution<> key_dist(0, KEY_POOL);

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        int operation = op_dist(gen);
        K key = make_key_from_index<K>(key_dist(gen));

        if (operation == 0) { // INSERT
            V value = make_value_from_index<V>(i);
            myTable.insert(key, value);
            goldenMap[key] = value;
        } else if (operation == 1) { // FIND
            V my_val = myTable.find(key);
            V golden_val = V{};
            auto it = goldenMap.find(key);
            if (it != goldenMap.end()) golden_val = it->second;

            if (my_val != golden_val) {
                std::cerr << "Correctness FAILED for " << label << " at op " << i << " (FIND)" << std::endl;
                std::cerr << "Key (index form): " << key_dist(gen) << "\n";
                assert(false && "Correctness mismatch");
            }
        } else { // REMOVE
            myTable.remove(key);
            goldenMap.erase(key);
        }
    }

    // Final verification
    for (const auto &p : goldenMap) {
        V my_val = myTable.find(p.first);
        if (my_val != p.second) {
            std::cerr << "Correctness final verification FAILED for " << label << std::endl;
            assert(false && "Final verification mismatch");
        }
    }

    std::cout << "Correctness PASSED for " << label << " (ops=" << NUM_OPERATIONS << ", final_size=" << goldenMap.size() << ")" << std::endl;
    std::cout << std::endl;
}

// Templated latency test for HashTable
template <typename K, typename V>
void runLatencyTest_MyTable_T(const std::string &label, int NUM_INSERTIONS = 200000) {
    std::cout << "--- Latency Test (Your HashTable) for " << label << " ---" << std::endl;
    HashTable<K, V> myTable;

    std::vector<long long> timings_ns;
    timings_ns.reserve(NUM_INSERTIONS);

    for (int i = 0; i < NUM_INSERTIONS; ++i) {
        K key = make_key_from_index<K>(i);
        V val = make_value_from_index<V>(i);

        auto start = std::chrono::high_resolution_clock::now();
        myTable.insert(key, val);
        auto end = std::chrono::high_resolution_clock::now();

        timings_ns.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    }

    long long max_latency = timings_ns.empty() ? 0 : *std::max_element(timings_ns.begin(), timings_ns.end());
    long long total_time = std::accumulate(timings_ns.begin(), timings_ns.end(), 0LL);
    double avg_latency = NUM_INSERTIONS ? static_cast<double>(total_time) / NUM_INSERTIONS : 0.0;

    double avg_latency_ms = avg_latency / 1e6;
    double max_latency_ms = static_cast<double>(max_latency) / 1e6;

    std::cout << "Inserted: " << NUM_INSERTIONS
              << ", avg insert: " << avg_latency << " ns (" << avg_latency_ms << " ms)"
              << ", max: " << max_latency << " ns (" << max_latency_ms << " ms)" << std::endl;
    std::cout << std::endl;
}

// Templated latency test for std::unordered_map
template <typename K, typename V>
void runLatencyTest_STL_T(const std::string &label, int NUM_INSERTIONS = 200000) {
    std::cout << "--- Latency Test (std::unordered_map) for " << label << " ---" << std::endl;
    std::unordered_map<K, V> stlMap;

    std::vector<long long> timings_ns;
    timings_ns.reserve(NUM_INSERTIONS);

    for (int i = 0; i < NUM_INSERTIONS; ++i) {
        K key = make_key_from_index<K>(i);
        V val = make_value_from_index<V>(i);

        auto start = std::chrono::high_resolution_clock::now();
        stlMap[key] = val;
        auto end = std::chrono::high_resolution_clock::now();

        timings_ns.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    }

    long long max_latency = timings_ns.empty() ? 0 : *std::max_element(timings_ns.begin(), timings_ns.end());
    long long total_time = std::accumulate(timings_ns.begin(), timings_ns.end(), 0LL);
    double avg_latency = NUM_INSERTIONS ? static_cast<double>(total_time) / NUM_INSERTIONS : 0.0;

    double avg_latency_ms = avg_latency / 1e6;
    double max_latency_ms = static_cast<double>(max_latency) / 1e6;

    std::cout << "Inserted: " << NUM_INSERTIONS
              << ", avg insert: " << avg_latency << " ns (" << avg_latency_ms << " ms)"
              << ", max: " << max_latency << " ns (" << max_latency_ms << " ms)" << std::endl;
    std::cout << std::endl;
}

int main() {
    // Disable sync for faster I/O
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // We'll run correctness + latency tests for a few common/basic types.
    // Keep counts moderate so the run completes quickly on a developer machine.
    const int CORRECTNESS_OPS = 5000000;
    const int LATENCY_INS = 10000000;

    // 1) int -> int
    runCorrectnessTest_T<int, int>("int,int", CORRECTNESS_OPS, 20000);
    runLatencyTest_MyTable_T<int, int>("int,int", LATENCY_INS);
    runLatencyTest_STL_T<int, int>("int,int", LATENCY_INS);

    // 2) long -> long
    runCorrectnessTest_T<long, long>("long,long", CORRECTNESS_OPS, 20000);
    runLatencyTest_MyTable_T<long, long>("long,long", LATENCY_INS);
    runLatencyTest_STL_T<long, long>("long,long", LATENCY_INS);

    // 3) string -> string
    runCorrectnessTest_T<std::string, std::string>("string,string", CORRECTNESS_OPS, 20000);
    runLatencyTest_MyTable_T<std::string, std::string>("string,string", LATENCY_INS);
    runLatencyTest_STL_T<std::string, std::string>("string,string", LATENCY_INS);

    // 4) int -> string (mixed)
    runCorrectnessTest_T<int, std::string>("int,string", CORRECTNESS_OPS, 20000);
    runLatencyTest_MyTable_T<int, std::string>("int,string", LATENCY_INS);
    runLatencyTest_STL_T<int, std::string>("int,string", LATENCY_INS);

    // 5) string -> int (mixed)
    runCorrectnessTest_T<std::string, int>("string,int", CORRECTNESS_OPS, 20000);
    runLatencyTest_MyTable_T<std::string, int>("string,int", LATENCY_INS);
    runLatencyTest_STL_T<std::string, int>("string,int", LATENCY_INS);

    std::cout << "All stress tests completed." << std::endl;
    return 0;
}