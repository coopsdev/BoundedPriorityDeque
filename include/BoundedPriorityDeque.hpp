//
// Created by Cooper Larson on 7/30/24.
//

#ifndef SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
#define SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H

#include <vector>
#include <type_traits>
#include <cassert>

#ifdef ENABLE_DEBUG
#include <stdexcept>
#endif

// type trait to detect comparison operator
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

// BoundingPair class template
template<typename K, typename V, typename Enable = void>
class BoundingPair {
public:
    K key;
    V value;

    bool operator<(const BoundingPair& other) const {
        return this->key < other.key;
    }
};

// Specialization for comparator objects
template<typename K, typename V>
class BoundingPair<K, V, std::enable_if_t<has_comparison_operator_v<K>>> : public std::pair<K, V> {
public:
    K key;
    V value;

    bool operator<(const BoundingPair& other) const {
        K comparator;
        return comparator(this->key, other.key);
    }
};

template<typename K, typename V>
class BoundedPriorityDequeBase {
protected:
    std::vector<BoundingPair<K, V>> _buffer;
    size_t _k, _size = 0, _head = 0, _tail = 0;

    virtual bool compare(K a, K b) const = 0;

    [[nodiscard]] size_t nextIndex(size_t current) const { return (current + 1) % _k; }

    [[nodiscard]] size_t prevIndex(size_t current) const { return (current + _k - 1) % _k; }

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
    explicit BoundedPriorityDequeBase(size_t capacity) : _buffer(capacity), _k(capacity) {}

    BoundingPair<K, V> top() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access top element of empty BoundedPriorityDeque");
#endif
        return _buffer[_head];
    }

    BoundingPair<K, V> bottom() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _buffer[_tail];
    }

    [[nodiscard]] K bottomK() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _buffer[_tail].key;
    }

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

    BoundingPair<K, V> pop() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        BoundingPair<K, V> result = _buffer[_head];
        _head = nextIndex(_head);
        --_size;
        return result;
    }

    BoundingPair<K, V> popBottom() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        BoundingPair<K, V> result = _buffer[_tail];
        _tail = prevIndex(_tail);
        --_size;
        return result;
    }

    void emplace(K key, const V& value) { push({ key, value }); }

    void operator+=(const BoundedPriorityDequeBase<K, V>& rhs) {
        for (size_t i = 0; i < rhs._size; ++i) {
            size_t index = (rhs._head + i) % rhs._k;
            if (_size == _k && compare(bottom().key, rhs._buffer[index].key)) return;
            push(rhs._buffer[index]);
        }
    }

    void clear() {
        _head = 0;
        _tail = 0;
        _size = 0;
    }

    [[nodiscard]] size_t size() const { return _size; }
    [[nodiscard]] size_t capacity() const { return _k; }
    [[nodiscard]] bool empty() const { return _size == 0; }
    [[nodiscard]] bool full() const { return _size == _k; }

    void setCapacity(unsigned int k) {
        _k = k;
        _buffer.resize(k);
        clear();
    }
};

template<typename K, typename V>
class BoundedMinPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    [[nodiscard]] bool compare(K a, K b) const override { return a < b; }

public:
    explicit BoundedMinPriorityDeque(unsigned int capacity = 0) : BoundedPriorityDequeBase<K, V>(capacity) {}

};

template<typename K, typename V>
class BoundedMaxPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    [[nodiscard]] bool compare(K a, K b) const override { return a > b; }

public:
    explicit BoundedMaxPriorityDeque(unsigned int capacity = 0) : BoundedPriorityDequeBase<K, V>(capacity) {}
};


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
