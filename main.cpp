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
        // cout<<"OPERATION : "<<operation<<endl<<endl;
        if (operation == 0) { // --- INSERT ---
            
            std::string value = "val" + std::to_string(i);
            // cout<<"\tinserting : { "<<key<<" "<<value<<" }"<<endl;
            myTable.insert(key, value);
            goldenMap[key] = value; // std::unordered_map also handles updates
            
        } else if (operation == 1) { // --- FIND ---
            // Even if goldenMap is empty, we test 'find' on potentially non-existent keys
            std::string my_val = myTable.find(key);
            
            // cout<<"\tfinding : { "<<key<<" "<<" }"<<endl;
            // Check if the golden map has the key
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
                // Optional: myTable.print();
                assert(false && "Correctness test failed on find!");
            }

        } else { // --- REMOVE ---
            // cout<<"\tdeleting: { "<<key<<" "<<" }"<<endl;
            myTable.remove(key);
            goldenMap.erase(key); // std::unordered_map erase on non-existent key is safe
        }
    }

    // Final verification: Iterate through all keys in goldenMap and ensure they exist and match in myTable
    for (const auto& pair : goldenMap) {
        std::string my_val = myTable.find(pair.first);
        if (my_val != pair.second) {
            std::cerr << "Correctness Test FAILED during final verification!" << std::endl;
            std::cerr << "Key: " << pair.first << std::endl;
            std::cerr << "My Table found: '" << my_val << "'" << std::endl;
            std::cerr << "Golden Map found: '" << pair.second << "'" << std::endl;
            // Optional: myTable.print();
            assert(false && "Correctness test failed on final verification!");
        }
    }

    std::cout << "Correctness Test PASSED: All " << NUM_OPERATIONS << " operations matched std::unordered_map." << std::endl;
    std::cout << "Final map size: " << goldenMap.size() << " keys." << std::endl;
    std::cout << std::endl;
}

// --- Test 2: Latency Test (Proving No Pauses) ---
void runLatencyTest() {
    std::cout << "--- Running Latency Test ---" << std::endl;
    HashTable myTable;

    const int NUM_INSERTIONS = 2000000; // 2 Million insertions to trigger many resizes
    std::vector<long long> timings_ns; // Store timings in nanoseconds
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

    std::cout << "--- Latency Test Results ---" << std::endl;
    std::cout << "Total items inserted: " << NUM_INSERTIONS << std::endl;
    std::cout << "Average insertion latency: " << avg_latency << " ns (" << avg_latency / 1000.0 << " us)" << std::endl;
    std::cout << "MAXIMUM insertion latency: " << max_latency << " ns (" << static_cast<double>(max_latency) / 1000000.0 << " ms)" << std::endl;

    // Threshold: if max latency is over, say, 10ms, it's likely not fully incremental.
    // Adjust this threshold based on your hardware and expectations.
    if (static_cast<double>(max_latency) / 1000000.0 > 10.0) { 
        std::cout << "TEST FAILED: Max latency is too high! Possible pause during resize." << std::endl;
    } else {
        std::cout << "TEST PASSED: No major pauses detected during resizing." << std::endl;
    }
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
    runLatencyTest();

    std::cout << "All stress tests completed." << std::endl;

    return 0;
}