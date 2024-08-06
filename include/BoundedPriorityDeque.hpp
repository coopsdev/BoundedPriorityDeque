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
 * Uses a custom comparator for ordering.
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
 * This class provides the basic functionalities of a bounded deque with methods for
 * pushing, popping, and accessing elements while maintaining a defined capacity.
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

    /**
     * @brief Provides fast access to the next index of a given insertion position.
     *
     * @param current The index queried for next index
     * @return The next index with circular wrap-around
     */
    [[nodiscard]] size_t nextIndex(size_t current) const { return (current + 1) % _k; }

    /**
     * @brief Provides fast access to the previous index of a given insertion position.
     *
     * @param current The index queried for previous index
     * @return The previous index with circular wrap-around
     */
    [[nodiscard]] size_t prevIndex(size_t current) const { return (current + _k - 1) % _k; }

    /**
     * @brief Efficiently locates the optimal insertion index.
     *
     * Performs binary insertion search and locates insertion index in O(log n) time,
     * adapted for a circular buffer with modulo arithmetic. No handling of duplicate values.
     *
     * @param target The element to be inserted.
     * @return The optimal insertion index for the targeted insertion element.
     */
    size_t binarySearch(const BoundingPair<K, V>& target) const {
        size_t start = 0;
        auto end = _size;
        while (start != end) {
            size_t mid = start + (end - start) / 2;
            if (compare(_buffer[(_head + mid) % _k].key, target.key)) start = mid + 1;
            else end = mid;
        }
        return (_head + start) % _k;
    }

    /**
     * @brief Inserts an element into the _buffer
     *
     * This is a utility method to allow shared insertion logic between the public push method
     * and the operator+=() code, the latter of which needed a way to terminate the loop without
     * running redundant capacity checks, so this function is essentially protection free insertion (think
     * overwriting elements in the circular buffer), popping from the bottom if capacity is to be exceeded
     * must be handled by the programmer in this case.
     *
     * @param element
     */
    void insert(const BoundingPair<K, V>& element) {
        if (_size == 0) {
            _buffer[0] = element;
            _head = 0;
            _tail = 0;
            _size = 1;
            return;
        }

        auto index = binarySearch(element);
        if (index == nextIndex(_tail)) {
            if (index == prevIndex(_head) && compare(element.key, topK())) _head = prevIndex(_head);
            else _tail = nextIndex(_tail);
        }
        else if (index == prevIndex(_head)) _head = prevIndex(_head);
        else if (_head <= _tail && _head > 0) {
            std::move(_buffer.begin() + _head, _buffer.begin() + index + 1, _buffer.begin() + _head - 1);
            --_head;
        } else {
            std::move_backward(_buffer.begin() + index, _buffer.begin() + _tail + 1, _buffer.begin() + _tail + 2);
            ++_tail;
        }

        _buffer[index] = element;
        ++_size;
    }

    /**
     * @brief Internal method with no return val
     */
    void _popTop() {
        _head = nextIndex(_head);
        if (--_size == 0) clear();
    }

    /**
     * @brief Internal method with no return val
     */
    void _popBottom() {
        _tail = prevIndex(_tail);
        if (--_size == 0) clear();
    }

public:
    /**
     * @brief Primary constructor with default bounding capacity of zero.
     *
     * @param capacity The initially set bounding capacity of the data structure.
     */
    explicit BoundedPriorityDequeBase(size_t capacity = 0) : _buffer(capacity), _k(capacity) {}

    /**
     * @brief Get the highest-priority element.
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
     * Gets the next element to be pruned from the tail of the data structure.
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
     * @brief Get the key from the highest-priority element.
     *
     * Introduced to help with merging checks in operator+=, but useful for checking the top of the dequeues
     * key if an optimization threshold is trying to be achieved.
     *
     * @return the lowest-priority elements key.
     */
    [[nodiscard]] K topK() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _buffer[_head].key;
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
     * @brief constructs a BoundingPair<K, V> element and inserts it.
     *
     * @param key The bounding key value
     * @param value The data held by the bounding pair.
     */
    void emplace(K key, const V& value) {
        if (_size == _k) {
            if (compare(key, _buffer[_tail].key)) _popBottom();
            else return;
        }
        insert({ key, value });
    }

    /**
     * @brief Inserts an element into the vector.
     *
     * If dequeue is at capacity and the insertion is valid, pops the bottom element.
     * If straightforward insertion not possible, uses binary search to find the insertion index,
     * then shifts the elements above or below the insertion index to make room for the item.
     *
     * @param element The element to be inserted.
     */
    void push(const BoundingPair<K, V>& element) {
        if (_size == _k) {
            if (compare(element.key, _buffer[_tail].key)) _popBottom();
            else return;
        }
        insert(element);
    }

    /**
     * @brief random accessor relative to the top of the deque.
     *
     * This accessor has a lot of uses. It primarily exists to facilitate efficient merges
     * in operator+=() without removing const protections, but also could be useful for random access
     * to the i-th element offset from the top, ie the 4th highest-priority item in O(1) time.
     *
     * @param offsetTop The unsigned long offset from the top element.
     * @return The BoundingPair<K, V> element offset from the top of deque.
     */
    BoundingPair<K, V> operator[](size_t offsetTop) const {
        return _buffer[_head <= _tail ? _head + offsetTop : (_head + offsetTop) % _k];
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
        auto index = _head;
        _popTop();
        return _buffer[index];
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
        auto index = _tail;
        _popBottom();
        return _buffer[index];
    }

    /**
     * @brief Merges another BoundedPriorityDeque instance into the calling instance.
     *
     * Compares the incoming dequeues next value to the calling dequeues lowest-priority elements key,
     * terminates search early when the other buffers top element is lower-priority than 'this' bottom element.
     * Is non-destructive to the incoming dequeues data.
     *
     * @param rhs The BoundedPriorityDeque being merged into 'this' dequeue.
     */
    void operator+=(const BoundedPriorityDequeBase<K, V>& rhs) {
        for (size_t i = 0; i < rhs.size(); ++i) {
            if (_size == _k) {
                if (compare(rhs[i].key, bottomK())) _popBottom();
                else return;
            }
            insert(rhs[i]);
        }
    }

    /**
     * @brief clears and resets the data structure.
     *
     * Resets the size and index pointers to there original values. No destructor calls for performance.
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
     * @brief Sets the capacity and restores clean state.
     *
     * Updates the capacity, then restores the _head of the _buffer back to index 0
     * leaving the buffer in a contiguous fresh state. This preserves the integrity
     * of the higher-priority data when shrinking the buffer.
     *
     * @param k The new capacity.
     */
    void resize(size_t k) {
        if (k == 0) return;

        std::vector<BoundingPair<K, V>> newBuffer(k);

        if (_head <= _tail) {
            size_t elementsToCopy = std::min(_size, k);
            std::move(_buffer.begin() + _head, _buffer.begin() + _head + elementsToCopy, newBuffer.begin());
            _size = elementsToCopy;
        } else {
            size_t topSize = _k - _head;
            size_t bottomSize = _tail + 1;
            size_t elementsToCopyTop = std::min(topSize, k);
            size_t elementsToCopyBottom = std::min(bottomSize, k - elementsToCopyTop);

            std::move(_buffer.begin() + _head, _buffer.begin() + _head + elementsToCopyTop, newBuffer.begin());
            std::move(_buffer.begin(), _buffer.begin() + elementsToCopyBottom, newBuffer.begin() + elementsToCopyTop);

            _size = elementsToCopyTop + elementsToCopyBottom;
        }

        _buffer.swap(newBuffer);
        _k = k;
        _head = 0;
        _tail = _size - 1;
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
 * This class provides a lightweight custom-comparator implement of the base class.
 * This derived class allows for non-standard or non-arithmetic key types via a custom-comparator template argument.
 * The custom-comparator should be in the form of a function<bool(K a, K b)> comparator which returns
 * true if 'a' has a higher-priority than 'b'.
 *
 * @tparam K Template-comparator compatible key Type.
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
