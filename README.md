# EARLY RELEASE | ACTIVELY IN DEVELOPMENT
### A performant bounded priority deque, currently in active dev. 
* The goal is a priority deque (missing from the std c++ lib) that is bounded and highly performant.
* Inspired for use in multithreaded ball tree construction and query contexts for TSP solutions with maximum performance.
* "Safety rails" are removed by design when compiled for release
  * will throw exceptions and print debug info in debug mode
  * otherwise there are no protections, for maximum performance in tight multithreaded loops.
  * thread safety is programmers responsibility, I will ***not*** be responsible for introudcing deadlocks in your program under multithreaded contexts
    * the operator+=() merge/extend method is very efficient for merging deques from multiple local threaded futures into a single deque. The smaller the parent deque and the more data culled, the faster the performance, under my own testing at least...
### More info coming soon...
