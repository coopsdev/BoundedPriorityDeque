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
#include <set>

#ifdef ENABLE_DEBUG
#include <stdexcept>
#endif

template<typename K, typename V>
class BoundedPriorityDequeBase {
protected:
    struct Node {
        BoundingPair<K, V> _data;
        Node* _prev;
        Node* _next;

        explicit Node(const BoundingPair<K, V>& pair) : _data(pair), _prev(nullptr), _next(nullptr) {}
    };

    Node* _head;
    Node* _tail;
    std::set<Node*, std::function<bool(const Node* a, const Node* b)>> _index;
    size_t _k, _size;

    virtual bool compareElements(K a, K b) const = 0;

public:
    explicit BoundedPriorityDequeBase(const std::function<bool(const Node* a, const Node* b)>& compare, unsigned int capacity = 0) :
            _head(nullptr), _tail(nullptr),
            _index(compare),
            _k(capacity), _size(0) {}

    BoundingPair<K, V> top() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access top element of empty BoundedPriorityDeque");
#endif
        return _head->_data;
    }

    BoundingPair<K, V> bottom() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _tail->_data;
    }

    K bottomK() const {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to access bottom element of empty BoundedPriorityDeque");
#endif
        return _tail->_data.first;
    }

    BoundingPair<K, V> pop() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        auto topElement = _head;

        if (_size > 1) {
            _head = topElement->_next;
            _head->_prev = _tail;
            _tail->_next = _head;
        } else {
            _head = nullptr;
            _tail = nullptr;
        }

        _index.erase(topElement);
        BoundingPair<K, V> data = topElement->_data;
        delete topElement;
        --_size;

        return data;
    }

    BoundingPair<K, V> popBottom() {
#ifdef ENABLE_DEBUG
        if (empty()) throw std::runtime_error("Attempted to pop from empty BoundedPriorityDeque");
#endif
        auto bottomElement = _tail;

        if (_size > 1) {
            _tail = bottomElement->_prev;
            _tail->_next = _head;
            _head->_prev = _tail;
        } else {
            _head = nullptr;
            _tail = nullptr;
        }

        _index.erase(bottomElement);
        BoundingPair<K, V> data = bottomElement->_data;
        delete bottomElement;
        --_size;

        return data;
    }

    void push(const BoundingPair<K, V>& element) {
        Node* newNode = new Node(element);

        auto insertAsNewHead = [&] {
            newNode->_next = _head;
            newNode->_prev = _tail;
            _head->_prev = newNode;
            _tail->_next = newNode;
            _head = newNode;
        };

        auto insertAsNewTail = [&] {
            newNode->_prev = _tail;
            newNode->_next = _head;
            _tail->_next = newNode;
            _head->_prev = newNode;
            _tail = newNode;
        };

        auto insertInMiddle = [&](Node* it) {
            newNode->_next = it;
            newNode->_prev = it->_prev;
            it->_prev->_next = newNode;
            it->_prev = newNode;
        };

        if (!_head) {
            _head = _tail = newNode;
            _head->_next = _head;
            _head->_prev = _head;
        } else {
            auto it = _index.lower_bound(newNode);
            if (it == _index.begin()) insertAsNewHead();
            else if (it == _index.end()) insertAsNewTail();
            else insertInMiddle(*it);
        }

        _index.insert(newNode);
        ++_size;
        if (_size > _k) popBottom();
    }

    void emplace(K key, const V& value) { push(BoundingPair<K, V>(key, value)); }

    void operator+=(const BoundedPriorityDequeBase<K, V>& rhs) {
        if (rhs.empty()) return;
        auto currNode = rhs._head;
        do {
            if (_size == _k && !compareElements(currNode->_data.first, bottomK())) break;
            push(currNode->_data);
            currNode = currNode->_next;
        } while (currNode != rhs._head);
    }

    void clear() { while (_size > 0) pop(); }
    [[nodiscard]] unsigned int size() const { return _size; }
    [[nodiscard]] unsigned int capacity() const { return _k; }
    [[nodiscard]] bool empty() const { return _size == 0; }
    [[nodiscard]] bool full() const { return _size == _k; }

    void setCapacity(unsigned int k) { _k = k; }
};

#endif //SERVER_BOUNDED_PRIORITY_DEQUE_BASE_H

