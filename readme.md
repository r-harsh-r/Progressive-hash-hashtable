# Simple Progressive Hash Table

This repo implements a chained hash table with progressive (distributed) rehashing to avoid long pause times when growing. The implementation is a header-only template type `HashTable<T,H>` (both key and value types are templated).

Highlights
- Templated API: `HashTable<T,H>` where `T` is the key type and `H` the value type. The header defines `Node<T,H>` and implements the table methods (insert/find/remove/resize/progressiveRehashing) inline.
- Progressive rehashing: when expanding the bucket array the table moves nodes in small batches during normal operations, avoiding large pauses.

Files of interest
- `hashtable.h` — header-only implementation: templated `HashTable<T,H>` with full implementation.
- `main.cpp` — stress test harness. It now contains:
  - A templated correctness fuzz test that compares `HashTable` against `std::unordered_map`.
  - Templated latency tests for both `HashTable` and `std::unordered_map`.
  - Tests run for several common type combinations (see "Stress tests" below).

Stress tests included
- int -> int
- long -> long
- std::string -> std::string
- int -> std::string
- std::string -> int

What the latency output shows
- For each latency run the program prints:

  Inserted: <N>, avg insert: <avg_ns> ns (<avg_ms> ms), max: <max_ns> ns (<max_ms> ms)

  The reported max latency is printed in both nanoseconds and milliseconds (the <max_ms> value is the additional metric you requested).

Build & run
You can compile the provided `main.cpp` (which includes `hashtable.h`) with a modern C++ compiler. Example (developer machine):

```bash
# single-file compile (main.cpp includes hashtable.h)
g++ -std=c++17 -O2 main.cpp -o stress_test_app

# or, if you prefer to compile with hashtable.cpp / other sources present:
g++ -std=c++17 -O2 main.cpp hashtable.cpp -o stress_test_app
```

Run the stress tests:

```bash
./stress_test_app
```

Configuration notes
- The number of operations used by the correctness and latency tests is controlled by constants near the top of `main.cpp`:
  - `CORRECTNESS_OPS` — number of operations (insert/find/remove) per correctness fuzz test.
  - `LATENCY_INS` — number of insertions per latency run.

- To test other key/value types or different sizes, add or change calls to the templated helpers in `main.cpp` (see the `runCorrectnessTest_T<>` and `runLatencyTest_MyTable_T<>` usage in `main()`).

