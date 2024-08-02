# EARLY RELEASE | ACTIVELY IN DEVELOPMENT
## A performant bounded priority deque for specialized use-cases, highly volatile at the moment but will be stable in short future. 

### Highlighting the need:
* missing from the std C++ lib and as standalone lightweight library on major C++ package managers such as Conan (used for gtest in this project)
* Inspired for use in multithreaded ball tree construction and query contexts for TSP solutions with maximum performance.
* The core data structure on this has changed nearly 10 times since I started this a month ago (extracted from larger TSP project).
  * sorted vector w/binary search
  * vector min heap
  * unordered-set
  * ordered set
  * priority-queue
  * deque
  * ordered set of indexed linked-list nodes
  * *CURRENT: circular buffer based vector
    * minimizes destructor calls, previously profiled to be a huge weakness
    * allows O(1) pop from front of the priority deque, previously required a heapify or swap and bubble sort action that was often nearing O(n)

### This will be a header only, very lean, and highly-performant library.

## It is the crux of my C++ TSP solution at the moment serving roles in multiple facets of the optimization process (BallTree construction + NN queries, 2opt/3opt multithreaded task bins, etc):
* "Safety rails" are removed by design when compiled for release
  * will throw exceptions and print debug info in debug mode
  * otherwise there are no protections, for maximum performance in tight multithreaded loops.
  * thread safety is programmers responsibility
* the operator+=() merge/extend method is in dev but will be very efficient for merging deques from multiple local threaded futures into a single deque, preventing the necessity of using mutex with the right multi-threaded task architecture.
* The smaller the parent deque and the more data culled, the faster the performance, under my own testing at least...
### More info coming soon...
