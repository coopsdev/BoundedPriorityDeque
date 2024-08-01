//
// Created by Cooper Larson on 7/30/24.
//

#ifndef SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
#define SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H

#include "BoundingPair.h"
#include <vector>
#include <utility>
#include <memory>
#include <mutex>
#include <functional>
#include <algorithm>

#ifdef ENABLE_DEBUG
    #include <stdexcept>
#endif


template<typename K, typename V>
class BoundedPriorityDequeBase {
protected:
    std::vector<BoundingPair<K, V>> _container;
    size_t _k;

    void bubbleUp() {
        unsigned int index = _container.size() - 1;
        while (index > 0 && compareElements(_container[index].first, _container[index - 1].first)) {
            std::swap(_container[index], _container[index - 1]);
            --index;
        }
    }

    void bubbleDown() {
        size_t index = 0;
        while (index < _container.size() - 1 && !compareElements(_container[index].first, _container[index + 1].first)) {
            std::swap(_container[index], _container[index + 1]);
            ++index;
        }
    }

    virtual bool compareElements(K a, K b) const = 0;

    std::pair<bool, size_t> binarySearch(const BoundingPair<K, V>& pair) {
        auto val = pair.first;
        if (_container.empty()) return { false, 0 };
        size_t hi = _container.size() - 1, lo = 0;
        while (lo <= hi) {
            auto mid = lo + (hi - lo) / 2;
            if (_container[mid].first == val) return { true, mid };
            else if (!compareElements(_container[mid].first, val)) lo = mid + 1;
            else hi = mid - 1;
        }
        return { false, lo };
    }

public:
    explicit BoundedPriorityDequeBase(unsigned int capacity = 0) : _container(), _k(capacity) { _container.reserve(_k + 1); }

    BoundingPair<K, V> top() const {
#ifdef ENABLE_DEBUG
        if (_container.empty()) throw std::runtime_error("Attempted to access top element of empty BoundedHeap");
#endif
        return _container.front();
    }

    BoundingPair<K, V> bottom() const {
#ifdef ENABLE_DEBUG
        if (_container.empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedHeap");
#endif
        return _container.back();
    }

    [[nodiscard]] double bottomK() const {
#ifdef ENABLE_DEBUG
        if (_container.empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedHeap");
#endif
        return _container.back().first;
    }

    BoundingPair<K, V> pop() {
#ifdef ENABLE_DEBUG
        if (_container.empty()) throw std::runtime_error("Attempted to pop from empty BoundedHeap");
#endif
        auto topElement = _container.front();
        if (_container.size() > 1) _container.front() = std::move(_container.back());
        _container.pop_back();
        if (!_container.empty()) bubbleDown();
        return topElement;
    }

    BoundingPair<K, V> popBottom() {
#ifdef ENABLE_DEBUG
        if (_container.empty()) throw std::runtime_error("Attempted to pop from empty BoundedHeap");
#endif
        auto bottomElement = _container.back();
        _container.pop_back();
        return bottomElement;
    }

    void push(const BoundingPair<K, V>& element) {
        if (_container.size() < _k) _container.push_back(element);
        else {
            if (!compareElements(element.first, _container.back().first)) return;

            const auto& [foundMatch, index] = binarySearch(element);
            if (index != _container.size() - 1) std::swap(_container[index], _container.back());
        }

        bubbleUp();
        if (_container.size() > _k) _container.pop_back();
    }

    void emplace(double n, const V& point) { push({n, point}); }
    void clear() { _container.clear(); }
    [[nodiscard]] unsigned int size() const { return _container.size(); }
    [[nodiscard]] unsigned int capacity() const { return _k; }
    [[nodiscard]] bool empty() const { return _container.empty(); }
    [[nodiscard]] bool full() const { return _container.size() == _k; }

    void setCapacity(unsigned int k) {
        _k = k;
        _container.reserve(k);
    }
};


#endif //SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H
