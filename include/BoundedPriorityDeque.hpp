//
// Created by Cooper Larson on 7/30/24.
//

#ifndef SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
#define SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H

#include <vector>
#include <utility>
#include <type_traits>

#ifdef ENABLE_DEBUG
#include <stdexcept>
#endif

// Helper type traits to check if a type has a comparison operator
template <typename, typename T>
struct has_comparison_operator {
    static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");
};

template <typename C, typename Ret, typename... Args>
struct has_comparison_operator<C, Ret(Args...)> {
private:
    template <typename T>
    static constexpr auto check(T*) ->
    typename std::is_same<decltype(std::declval<T>().operator()(std::declval<Args>()...)), Ret>::type;

    template <typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

template <typename T>
constexpr bool has_comparison_operator_v = has_comparison_operator<T, bool(const T&, const T&)>::value;

// BoundingPair class template
template<typename K, typename V, typename Enable = void>
class BoundingPair : public std::pair<K, V> {
    using pair = std::pair<K, V>;
public:
    using pair::pair;
    using pair::first;
    using pair::second;

    // Comparison operator
    bool operator<(const BoundingPair& other) const {
        return first < other.first;
    }
};

// Specialization for comparator objects
template<typename K, typename V>
class BoundingPair<K, V, std::enable_if_t<has_comparison_operator_v<K>>> : public std::pair<K, V> {
    using pair = std::pair<K, V>;
public:
    using pair::pair;
    using pair::first;
    using pair::second;

    // Comparison operator using the comparator
    bool operator<(const BoundingPair& other) const {
        return K()(first, other.first);
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
            if (compare(_buffer[mid].first, target.first)) start = (mid + 1) % _buffer.size();
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

    [[nodiscard]] K& bottomK() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _buffer[_tail].first;
    }

    void push(const BoundingPair<K, V>& element) {
        if (_size == _k) {
            if (compare(element.first, _buffer[_tail].first)) popBottom();
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
            if (_size == _k && compare(bottom().first, rhs._buffer[index].first)) return;
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
