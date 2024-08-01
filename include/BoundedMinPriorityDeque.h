//
// Created by Cooper Larson on 7/13/24.
//

#ifndef SERVER_BOUNDED_MIN_PRIORITY_DEQUE_H
#define SERVER_BOUNDED_MIN_PRIORITY_DEQUE_H

#include "BoundedPriorityDequeBase.h"

template<typename K, typename V>
class BoundedMinPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    [[nodiscard]] bool compareElements(K a, K b) const override { return a < b; }

public:
    explicit BoundedMinPriorityDeque(unsigned int capacity = 0) : BoundedPriorityDequeBase<K, V>(capacity) {}

    void operator+=(const BoundedMinPriorityDeque<K, V>& rhs) {
        if (rhs._container.empty()) return;
        for (const BoundingPair element : rhs._container) {
            if (this->size() == this->capacity() && element.first >= this->bottomK()) break;
            BoundedPriorityDequeBase<K, V>::push(element);
        }
    }
};


#endif //SERVER_BOUNDED_MIN_PRIORITY_DEQUE_H

