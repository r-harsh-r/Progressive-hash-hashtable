# Simple Hash Table with Distributed (Progressive) Rehashing

This repository implements a basic chained hash table with a progressive (distributed) rehashing strategy to avoid long pause times during growing.

Files and key symbols
- [hashtable.h](hashtable.h) — declaration of the main type: [`HashTable`](hashtable.h).
- [hashtable.cpp](hashtable.cpp) — implementation of methods such as [`HashTable::Hash`](hashtable.cpp), [`HashTable::getIdx`](hashtable.cpp), [`HashTable::insert`](hashtable.cpp), [`HashTable::find`](hashtable.cpp), [`HashTable::remove`](hashtable.cpp), [`HashTable::resize`](hashtable.cpp), [`HashTable::progressiveRehashing`](hashtable.cpp), and [`HashTable::print`](hashtable.cpp).
- [main.cpp](main.cpp) — test harness and benchmarks that exercise correctness and latency.

Overview of the data structure
- Buckets: an array of singly-linked lists (`Node*` per bucket).
- Hashing: uses `std::hash<std::string>` via [`HashTable::Hash`](hashtable.cpp).
- Indexing: bucket index = `Hash(key) % N` via [`HashTable::getIdx`](hashtable.cpp).
- Initial bucket count: `N = 1024` (see [`HashTable` constructor](hashtable.h) / [hashtable.cpp](hashtable.cpp)).
- Each node stores a `key`, `value`, and `nex` pointer (see [`Node` in hashtable.h](hashtable.h)).

Primary operations
- Insert — [`HashTable::insert`](hashtable.cpp)
  - If not resizing (`trans_idx == -1`): insert at head of `bin[idx]`.
  - If resizing: insert into the target `newBin` (so new entries go straight to the destination table).
  - Increments `count_keys`.
- Find — [`HashTable::find`](hashtable.cpp)
  - If not resizing: scan `bin[idx]` and return value or empty string if missing.
  - If resizing: search `bin[idx]` first, then `newBin[newIdx]`. If not found, a small rehash batch is advanced (`progressiveRehashing`) and an empty result is returned.
- Remove — [`HashTable::remove`](hashtable.cpp)
  - Searches the relevant bucket(s) (both `bin` and `newBin` when resizing), unlinks and deletes matching nodes, and decrements `count_keys`.
  - When called during a resize phase it participates in the progressive rehashing steps.
- Print / Debug — [`HashTable::print`](hashtable.cpp).

Progressive (distributed) rehashing
- When the table grows, a new bucket array `newBin` of size `2 * N` is allocated and `trans_idx` set to `0` by [`HashTable::resize`](hashtable.cpp).
- Instead of moving all nodes at once, [`HashTable::progressiveRehashing`](hashtable.cpp) moves up to `batchSize` nodes per call (default `batchSize = 128`).
- A single call to `progressiveRehashing` scans `bin[trans_idx]`, moves nodes into `newBin` (recomputing `Hash(key) % (2*N)`), and updates `trans_idx`. It may stop early when the batch quota is used.
- When `trans_idx >= N`, the migration is complete: `bin` is replaced with `newBin`, `N` is doubled, `newBin` is freed, and `trans_idx` is reset to `-1`.
- Client-facing operations (insert/find/remove) interleave normal work with calls to `progressiveRehashing`, so the cost of rehashing is distributed across many operations and long GC-like pauses are avoided.

Complexity notes
- Average-case: O(1) for insert/find/remove (assuming a reasonable hash and load).
- Worst-case: O(k) where k is the length of a bucket chain.
- Rehashing cost is amortized and distributed: moving all nodes costs O(n) overall, but each operation only pays for a small constant batch. Load factor is $α = n / N$; resizing policy controls when `resize()` is triggered.

Testing and benchmarking
- See [main.cpp](main.cpp) for:
  - A correctness fuzz test comparing against `std::unordered_map`.
  - A latency test that inserts millions of items while recording per-insert latency to validate the absence of long pauses.

References
- Implementation: [hashtable.h](hashtable.h) and [hashtable.cpp](hashtable.cpp).
- Example tests: [main.cpp](main.cpp).