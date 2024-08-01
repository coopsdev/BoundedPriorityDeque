//
// Created by Cooper Larson on 7/13/24.
//

#ifndef SERVER_BOUNDED_MIN_PRIORITY_DEQUE_H
#define SERVER_BOUNDED_MIN_PRIORITY_DEQUE_H

#include "BoundedPriorityDequeBase.h"

template<typename K, typename V>
class BoundedMinPriorityDeque : public BoundedPriorityDequeBase<K, V> {
    using Node = BoundedPriorityDequeBase<K, V>::Node;

protected:
    [[nodiscard]] bool compareElements(K a, K b) const override { return a < b; }

public:
    explicit BoundedMinPriorityDeque(unsigned int capacity = 0) :
        BoundedPriorityDequeBase<K, V>(
                [](const Node* a, const Node* b) { return a->_data.first < b->_data.first; }, capacity) {}

};


#endif //SERVER_BOUNDED_MIN_PRIORITY_DEQUE_H

