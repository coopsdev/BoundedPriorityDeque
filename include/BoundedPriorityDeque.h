//
// Created by Cooper Larson on 7/31/24.
//

#ifndef BOUNDED_HEAP_BOUNDED_PRIORITY_DEQUE_H
#define BOUNDED_HEAP_BOUNDED_PRIORITY_DEQUE_H

#include "BoundedPriorityDequeBase.h"

template<typename K, typename V, typename Comparator = std::less<K>>
class BoundedPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    Comparator comparator;

    using BoundedPriorityDequeBase<K, V>::_container;
    using BoundedPriorityDequeBase<K, V>::_k;

    [[nodiscard]] bool compareElements(K a, K b) const override { return comparator(a, b); }

public:
    explicit BoundedPriorityDeque(unsigned int capacity = 0, Comparator comp = Comparator()) : BoundedPriorityDequeBase<K, V>(capacity), comparator(comp) {}

    void operator+=(const BoundedPriorityDeque<K, V>& rhs) {
        if (rhs._container.empty()) return;
        for (const BoundingPair element : rhs._container) {
            if (this->size() == this->capacity() && !compareElements(element.first, this->bottomK())) break;
            BoundedPriorityDequeBase<K, V>::push(element);
        }
    }
};

#endif //BOUNDED_HEAP_BOUNDED_PRIORITY_DEQUE_H
