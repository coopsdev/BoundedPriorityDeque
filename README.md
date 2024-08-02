# BoundedPriorityDeque.hpp

A highly efficient, single-header C++ library implementing a bounded priority deque with a focus on performance and versatility through templating. This library utilizes a circular buffer, chosen for its robust performance characteristics.

## Release Status: Early Development

This project is currently in active development and is subject to significant changes. However, a stable release is anticipated in the near future.

## Motivation

The bounded priority deque addresses a notable gap in the standard C++ library and fills a niche not currently catered to by lightweight libraries available on major C++ package managers such as Conan. This project emerged from complex requirements in multi-threaded ball tree constructions and Traveling Salesman Problem (TSP) solution optimizations.

### Evolution of the Core Data Structure

The design of this deque has undergone numerous iterations, each aimed at enhancing performance and efficiency:

- Sorted vector with binary search
- Vector-based min heap
- Unordered set
- Ordered set
- Priority queue
- Deque
- Ordered set of indexed linked-list nodes
- **Current Implementation:** Circular buffer-based vector
  - This minimizes destructor calls, addressing a previously identified performance bottleneck.
  - It enables O(1) time complexity for pop operations from the front of the deque, a significant improvement over previous methods that approached O(n) complexity.

## Key Features

- **Single-Header Library:** Streamlined and efficient, suitable for integration into various C++ projects without the overhead of additional compilation units.
- **Optimized for TSP Solutions:** Crucial to various facets of the TSP optimization process, including BallTree construction, nearest neighbor queries, and multithreaded 2-opt and 3-opt tasks.
- **Developer Control:** Intentionally designed without "safety rails" in release mode to maximize performance:
  - Throws exceptions and provides debug information in debug mode.
  - Expects the programmer to manage thread safety, accommodating highly efficient multithreaded operations.
- **Efficient Merging Capability:** The `operator+=()` for merging deques aims to facilitate efficient integration of results from multiple threads without the need for mutex locks, given appropriate task architecture.
- **Performance Insights:** Preliminary testing suggests performance improves as the size of the parent deque decreases and more data is culled.

## Future Directions

Further information and updates on this library will be provided as development progresses. Interested users and contributors are encouraged to keep an eye on repository updates for the latest enhancements and stability improvements.
