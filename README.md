# EARLY RELEASE | ACTIVELY IN DEVELOPMENT
### A performant bounded priority deque, currently in active dev. 
* The goal is a priority deque (missing from the std c++ lib) that is bounded and highly performant.
* Inspired for use in multithreaded ball tree construction and query contexts for TSP solutions with maximum performance.
* "Safety rails" are removed by design when compiled for release
  * will throw exceptions and print debug info in debug mode
  * otherwise there are no protections, for maximum performance in tight multithreaded loops.
  * thread safety is programmers responsibility, I will ***not*** be responsible for introudcing deadlocks in your program under multithreaded contexts
    * the operator+=() merge/extend method is very efficient for merging deques from multiple local threaded futures into a single deque. The smaller the parent deque and the more data culled, the faster the performance, under my own testing at least...
* recently restructured from heap to binary insert sorted vector, should allow for O(n log n) inserts to the middle of the tree due to current use of bubble up afterwards, O(log n) search/query, and O(1) insertion/removal/query. These are rough calculations by my tired brain, and ultimately the goal is for O(log n) performance across the data structure.
 * Heap is computationally expensive to search for the bottom object. Its better to initialize once and replace the allocations directly in a contiguous allocation of memory such as a vector on the heap.
  * Some may say, but hey a "minMax" heap can offer efficient retrieval of both, but thats not the case. Its double the memory consumption, and still requires a linear lookup of the index value in the alternate heapset to remove.
### More info coming soon...
