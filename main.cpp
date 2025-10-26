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

// --- Test 1: Correctness Fuzzing (Golden Model Test) ---
void runCorrectnessTest() {
    std::cout << "--- Running Correctness Fuzz Test ---" << std::endl;
    HashTable myTable;
    std::unordered_map<std::string, std::string> goldenMap;

    // Setup for random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> op_dist(0, 2); // 0: Insert, 1: Find, 2: Remove
    // Key pool size. Smaller pool means more collisions and more removals of existing keys.
    std::uniform_int_distribution<> key_dist(0, 50000); 

    const int NUM_OPERATIONS = 100000; // 1 Million operations for fuzzing

    for (int i = 0; i < NUM_OPERATIONS; ++i) {
        int operation = op_dist(gen);
        std::string key = "key" + std::to_string(key_dist(gen)); // Random key
        
        if (operation == 0) { // --- INSERT ---
            std::string value = "val" + std::to_string(i);
            myTable.insert(key, value);
            goldenMap[key] = value; // std::unordered_map also handles updates
            
        } else if (operation == 1) { // --- FIND ---
            std::string my_val = myTable.find(key);
            
            std::string golden_val = "";
            auto it = goldenMap.find(key);
            if (it != goldenMap.end()) {
                golden_val = it->second;
            }

            // Assertion: values must be identical
            if (my_val != golden_val) {
                std::cerr << "Correctness Test FAILED at operation " << i << " (FIND)!" << std::endl;
                std::cerr << "Key: " << key << std::endl;
                std::cerr << "My Table found: '" << my_val << "'" << std::endl;
                std::cerr << "Golden Map found: '" << golden_val << "'" << std::endl;
                assert(false && "Correctness test failed on find!");
            }

        } else { // --- REMOVE ---
            myTable.remove(key);
            goldenMap.erase(key); // std::unordered_map erase on non-existent key is safe
        }
    }

    // Final verification
    for (const auto& pair : goldenMap) {
        std::string my_val = myTable.find(pair.first);
        if (my_val != pair.second) {
            std::cerr << "Correctness Test FAILED during final verification!" << std::endl;
            std::cerr << "Key: " << pair.first << std::endl;
            std::cerr << "My Table found: '" << my_val << "'" << std::endl;
            std::cerr << "Golden Map found: '" << pair.second << "'" << std::endl;
            assert(false && "Correctness test failed on final verification!");
        }
    }

    std::cout << "Correctness Test PASSED: All " << NUM_OPERATIONS << " operations matched std::unordered_map." << std::endl;
    std::cout << "Final map size: " << goldenMap.size() << " keys." << std::endl;
    std::cout << std::endl;
}

// --- Test 2: Latency Test (YOUR HashTable) ---
void runLatencyTest_MyTable() { // Renamed this function
    std::cout << "--- Running Latency Test (Your HashTable) ---" << std::endl;
    HashTable myTable;

    const int NUM_INSERTIONS = 2000000; // 2 Million insertions
    std::vector<long long> timings_ns;
    timings_ns.reserve(NUM_INSERTIONS);

    std::cout << "Inserting " << NUM_INSERTIONS << " items and measuring each insertion latency..." << std::endl;

    for (int i = 0; i < NUM_INSERTIONS; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string val = "val" + std::to_string(i);

        auto start = std::chrono::high_resolution_clock::now();
        myTable.insert(key, val);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        timings_ns.push_back(duration.count());
    }

    // --- Analyze Results ---
    long long max_latency = 0;
    if (!timings_ns.empty()) {
        max_latency = *std::max_element(timings_ns.begin(), timings_ns.end());
    }
    
    long long total_time = std::accumulate(timings_ns.begin(), timings_ns.end(), 0LL);
    double avg_latency = 0.0;
    if (NUM_INSERTIONS > 0) {
        avg_latency = static_cast<double>(total_time) / NUM_INSERTIONS;
    }

    std::cout << "--- Latency Test Results (Your HashTable) ---" << std::endl;
    std::cout << "Total items inserted: " << NUM_INSERTIONS << std::endl;
    std::cout << "Average insertion latency: " << avg_latency << " ns (" << avg_latency / 1000.0 << " us)" << std::endl;
    std::cout << "MAXIMUM insertion latency: " << max_latency << " ns (" << static_cast<double>(max_latency) / 1000000.0 << " ms)" << std::endl;

    if (static_cast<double>(max_latency) / 1000000.0 > 100.0) { 
        std::cout << "TEST FAILED (Your Table): Max latency is too high! Possible pause during resize." << std::endl;
    } else {
        std::cout << "TEST PASSED (Your Table): No major pauses detected during resizing." << std::endl;
    }
    std::cout << std::endl;
}

// --- Test 3: Latency Test (std::unordered_map) ---
void runLatencyTest_STL() {
    std::cout << "--- Running Latency Test (std::unordered_map) ---" << std::endl;
    std::unordered_map<std::string, std::string> stlMap; // Use the STL map

    const int NUM_INSERTIONS = 2000000; // Same workload
    std::vector<long long> timings_ns;
    timings_ns.reserve(NUM_INSERTIONS);

    std::cout << "Inserting " << NUM_INSERTIONS << " items and measuring each insertion latency..." << std::endl;

    for (int i = 0; i < NUM_INSERTIONS; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string val = "val" + std::to_string(i);

        auto start = std::chrono::high_resolution_clock::now();
        stlMap[key] = val; // Use the STL insert operation
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        timings_ns.push_back(duration.count());
    }

    // --- Analyze Results ---
    long long max_latency = 0;
    if (!timings_ns.empty()) {
        max_latency = *std::max_element(timings_ns.begin(), timings_ns.end());
    }
    
    long long total_time = std::accumulate(timings_ns.begin(), timings_ns.end(), 0LL);
    double avg_latency = 0.0;
    if (NUM_INSERTIONS > 0) {
        avg_latency = static_cast<double>(total_time) / NUM_INSERTIONS;
    }

    std::cout << "--- Latency Test Results (std::unordered_map) ---" << std::endl;
    std::cout << "Total items inserted: " << NUM_INSERTIONS << std::endl;
    std::cout << "Average insertion latency: " << avg_latency << " ns (" << avg_latency / 1000.0 << " us)" << std::endl;
    std::cout << "MAXIMUM insertion latency: " << max_latency << " ns (" << static_cast<double>(max_latency) / 1000000.0 << " ms)" << std::endl;
    std::cout << std::endl;
}

int main() {
    // --- IMPORTANT: For memory leak detection, compile with -fsanitize=address ---
    // g++ -std=c++17 -g -fsanitize=address main.cpp hashtable.cpp -o stress_test_app
    // Then run: ./stress_test_app
    
    // Disable synchronization for faster I/O in tests
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    runCorrectnessTest();
    runLatencyTest_MyTable(); // Run your test
    runLatencyTest_STL();     // Run the STL test

    std::cout << "All stress tests completed." << std::endl;

    return 0;
}