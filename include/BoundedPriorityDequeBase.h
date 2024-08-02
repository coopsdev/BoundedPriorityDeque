//
// Created by Cooper Larson on 7/30/24.
//

#ifndef SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
#define SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H

#include "BoundingPair.h"
#include <vector>
#include <stdexcept>

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
    explicit BoundedPriorityDequeBase(unsigned int capacity) : _buffer(capacity), _k(capacity) {}

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
                if (_head > 0) {
                    std::move(_buffer.begin() + _head, _buffer.begin() + pos + 1, _buffer.begin() + pos + 2);
                } else if (_tail < _k - 1) {
                    std::move_backward(_buffer.begin() + pos, _buffer.begin() + _tail + 1, _buffer.begin() + _tail + 2);
                }
            } else {
                if (pos > 0) {
                    std::move_backward(_buffer.begin() + pos, _buffer.begin() + _tail + 1, _buffer.begin() + _tail + 2);
                } else {
                    std::move(_buffer.begin() + _head, _buffer.begin() + pos + 1, _buffer.begin() + _head - 1);
                }
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
        _size--;
        return result;
    }

    BoundingPair<K, V> popBottom() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        BoundingPair<K, V> result = _buffer[_tail];
        _tail = prevIndex(_tail);
        _size--;
        return result;
    }

    void emplace(K key, const V& value) {
        BoundingPair<K, V> element(key, value);
        push(element);
    }

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

#endif // SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
