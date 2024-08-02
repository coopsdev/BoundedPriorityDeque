//
// Created by Cooper Larson on 7/30/24.
//

#ifndef SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
#define SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H

#include <vector>

#ifdef ENABLE_DEBUG
#include <stdexcept>
#endif

/**
 * @file BoundedPriorityDeque.hpp
 * @brief Provides a framework for a bounded priority deque with customizable comparison mechanisms.
 *
 * Defines several template classes for implementing a bounded priority deque that supports
 * both min and max oriented operations with customizable comparison.
 * It includes mechanisms to handle capacity and size management,
 * as well as utility functions for element access and manipulation.
 */

/**
 * @class has_comparison_operator
 * @brief Type trait to detect the presence of a comparison operator in types.
 *
 * This type trait checks if a given type T has a comparison operator defined.
 * It is primarily used to enable specialization based on the availability of this operator.
 *
 * @tparam T The type to check for a comparison operator.
 */
template <typename T>
class has_comparison_operator {
    template <typename U, typename = decltype(std::declval<U>()(std::declval<const T&>(), std::declval<const T&>()))>
    static std::true_type check(U*);

    static std::false_type check(...);

public:
    static constexpr bool value = decltype(check(std::declval<T*>()))::value;
};

template <typename T>
inline constexpr bool has_comparison_operator_v = has_comparison_operator<T>::value;

/**
 * @class BoundingPair
 * @brief A template class to store key-value pairs with comparison operations.
 *
 * @tparam K The key type.
 * @tparam V The value type.
 * @tparam Enable A type parameter to enable template specialization based on traits.
 */
template<typename K, typename V, typename Enable = void>
class BoundingPair {
public:
    K key; ///< The key of the pair.
    V value; ///< The value of the pair.

    bool operator<(const BoundingPair& other) const {
        return this->key < other.key;
    }
};

/**
 * @class BoundingPair
 * @brief Specialization of BoundingPair for types with a custom comparison operator.
 *
 * Inherits from std::pair and uses a custom comparator for ordering.
 *
 * @tparam K The key type.
 * @tparam V The value type.
 */
template<typename K, typename V>
class BoundingPair<K, V, std::enable_if_t<has_comparison_operator_v<K>>> : public std::pair<K, V> {
public:
    bool operator<(const BoundingPair& other) const {
        K comparator;
        return comparator(this->key, other.key);
    }
};

/**
 * @class BoundedPriorityDequeBase
 * @brief Base class for implementing a bounded priority deque.
 *
 * This class provides the basic functionalities of a bounded deque with methods for pushing, popping, and accessing elements while maintaining a defined capacity.
 *
 * @tparam K Type of the key.
 * @tparam V Type of the value.
 */
template<typename K, typename V>
class BoundedPriorityDequeBase {
protected:
    std::vector<BoundingPair<K, V>> _buffer;
    size_t _k, _size = 0, _head = 0, _tail = 0;

    /**
     * Compares two keys.
     *
     * @param a The first key.
     * @param b The second key.
     * @return True if a is considered less than b in a min-oriented deque, or more in a max-oriented deque.
     */
    virtual bool compare(K a, K b) const = 0;

    [[nodiscard]] size_t nextIndex(size_t current) const { return (current + 1) % _k; }
    [[nodiscard]] size_t prevIndex(size_t current) const { return (current + _k - 1) % _k; }

    /**
     * @brief Efficiently locates the optimal insertion index in O(log n) time complexity.
     *
     * Performs binary search & locates insertion index in O(log n) time, adapted for a circular buffer with modulo arithmetic.
     *
     * @param target The element to be inserted.
     * @return
     */
    size_t binarySearch(const BoundingPair<K, V>& target) const {
        auto start = _head, end = _tail + 1;
        while (start != end) {
            size_t mid = (start + (end - start) / 2) % _buffer.size();
            if (compare(_buffer[mid].key, target.key)) start = (mid + 1) % _buffer.size();
            else end = mid;
        }
        return start;
    }

public:
    /**
     * The only constructor with default capacity == 0.
     *
     * @param capacity The initially set bounding capacity of the data structure (see setCapacity(k) for more...).
     */
    explicit BoundedPriorityDequeBase(size_t capacity = 0) : _buffer(capacity), _k(capacity) {}

    /**
     * @brief Get the highest-priority element.
     *
     * Gets the highest-priority (ie. lowest K / key value in BoundedMinPriorityDeque) element in the circular buffer.
     *
     * @return the BoundingPair at the head of the circular buffer.
     */
    BoundingPair<K, V> top() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access top element of empty BoundedPriorityDeque");
#endif
        return _buffer[_head];
    }

    /**
     * @brief Get the lowest-priority element.
     *
     * Gets the lowest-priority (next to be pruned) element at the tail of the circular buffer.
     *
     * @return the BoundingPair at the tail of the circular buffer.
     */
    BoundingPair<K, V> bottom() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _buffer[_tail];
    }

    /**
     * @brief Get the key from the lowest-priority element.
     *
     * Effective for comparing against the bottom element without the overhead of passing the full BoundingPair<K, V>.
     *
     * @return the lowest-priority elements key.
     */
    [[nodiscard]] K bottomK() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _buffer[_tail].key;
    }

    /**
     * @brief Inserts an element into the vector.
     *
     * Pops bottom element to make room if necessary.
     *
     * If straightforward insertion not possible, uses binary search to find the insertion index,
     * then shifts the elements above or below the insertion index to make room for the item.
     *
     * @param element The element to be inserted.
     */
    void push(const BoundingPair<K, V>& element) {
        if (_size == _k) {
            if (compare(element.key, _buffer[_tail].key)) popBottom();
            else return;
        } else if (_size == 0) {
            _buffer[0] = element;
            _head = 0;
            _tail = 0;
            _size = 1;
            return;
        }

        auto pos = binarySearch(element);
        if (pos != nextIndex(_tail)) {
            if (_head <= _tail) {
                if (_head > 0) std::move(_buffer.begin() + _head, _buffer.begin() + pos + 1, _buffer.begin() + pos + 2);
                else std::move_backward(_buffer.begin() + pos, _buffer.begin() + _tail + 1, _buffer.begin() + _tail + 2);
            } else {
                if (pos > 0) std::move_backward(_buffer.begin() + pos, _buffer.begin() + _tail + 1, _buffer.begin() + _tail + 2);
                else std::move(_buffer.begin() + _head, _buffer.begin() + pos + 1, _buffer.begin() + _head - 1);
            }
        }

        _buffer[pos] = element;
        _tail = nextIndex(_tail);
        _size++;
    }

    /**
     * @brief remove the highest-priority element.
     *
     * @return The removed highest-priority element.
     */
    BoundingPair<K, V> pop() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        BoundingPair<K, V> result = _buffer[_head];
        _head = nextIndex(_head);
        --_size;
        return result;
    }

    /**
     * @brief remove the lowest-priority element.
     *
     * @return The removed lowest-priority element.
     */
    BoundingPair<K, V> popBottom() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        BoundingPair<K, V> result = _buffer[_tail];
        _tail = prevIndex(_tail);
        --_size;
        return result;
    }

    /**
     * @brief constructs a BoundingPair<K, V> element and inserts it.
     *
     * @param key The bounding key value
     * @param value The data held by the bounding pair.
     */
    void emplace(K key, const V& value) { push({ key, value }); }

    /**
     * @brief Merges another BoundedPriorityDeque instance into the calling instance.
     *
     * Compares the incoming dequeues next value to 'this' dequeues lowest-priority elements key,
     * terminates search early when the other buffers top element is lower-priority than 'this' bottom element.
     *
     * @param rhs The BoundedPriorityDeque being merged into 'this' dequeue.
     */
    void operator+=(const BoundedPriorityDequeBase<K, V>& rhs) {
        for (size_t i = 0; i < rhs._size; ++i) {
            size_t index = (rhs._head + i) % rhs._k;
            if (_size == _k && compare(bottom().key, rhs._buffer[index].key)) return;
            push(rhs._buffer[index]);
        }
    }

    /**
     * @brief clears and resets the data structure.
     *
     * Resets the size and index pointers to there original values, no destructor calls for performance.
     */
    void clear() {
        _head = 0;
        _tail = 0;
        _size = 0;
    }

    /**
     *
     * @return The vectors logical size.
     */
    [[nodiscard]] size_t size() const { return _size; }

    /**
     *
     * @return The bounding capacity of the data structure.
     */
    [[nodiscard]] size_t capacity() const { return _k; }

    /**
     *
     * @return True if the dequeue is empty, else False.
     */
    [[nodiscard]] bool empty() const { return _size == 0; }

    /**
     *
     * @return True if the dequeue is full, else False.
     */
    [[nodiscard]] bool full() const { return _size == _k; }

    /**
     * @brief Updates the capacity.
     *
     * Sets new capacity
     * Resizes buffer with new allocations
     * then resets the buffer to default params.
     *
     * @param k The new capacity.
     */
    void setCapacity(unsigned int k) {
        _k = k;
        _buffer.resize(k);
        clear();
    }
};

/**
 * @class BoundedMinPriorityDeque
 * @brief Lightweight minimum priority dequeue
 *
 * This class provides a lightweight minimum priority implement of the base class with an arithmetic key value.
 *
 * @tparam K Arithmetic type of the key.
 * @tparam V Type of the value.
 */
template<typename K, typename V>
class BoundedMinPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    [[nodiscard]] bool compare(K a, K b) const override { return a < b; }

public:
    explicit BoundedMinPriorityDeque(unsigned int capacity = 0) : BoundedPriorityDequeBase<K, V>(capacity) {}
};

/**
 * @class BoundedMaxPriorityDeque
 * @brief Lightweight maximum priority dequeue
 *
 * This class provides a lightweight maximum priority implement of the base class with an arithmetic key value.
 *
 * @tparam K Arithmetic type of the key.
 * @tparam V Type of the value.
 */
template<typename K, typename V>
class BoundedMaxPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    [[nodiscard]] bool compare(K a, K b) const override { return a > b; }

public:
    explicit BoundedMaxPriorityDeque(unsigned int capacity = 0) : BoundedPriorityDequeBase<K, V>(capacity) {}
};

/**
 * @class BoundedPriorityDeque
 * @brief Lightweight custom-comparator priority dequeue
 *
 * This class provides a lightweight minimum priority implement of the base class.
 * This derived class allows for non-standard or non-arithmetic key types via a custom-comparator template argument.
 * The custom-comparator should be in the form of a function<bool(K a, K b)> comparator which returns
 * true if 'a' has a higher-priority than 'b'.
 *
 * @tparam K Comparator support type of the key.
 * @tparam V Type of the value.
 */
template<typename K, typename V, typename Comparator = std::less<K>>
class BoundedPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    Comparator comparator;

    [[nodiscard]] bool compare(K a, K b) const override { return comparator(a, b); }

public:
    explicit BoundedPriorityDeque(unsigned int capacity = 0, Comparator comp = Comparator()) :
            BoundedPriorityDequeBase<K, V>(capacity), comparator(comp) {}
};

#endif // SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
