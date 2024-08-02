//
// Created by Cooper Larson on 7/13/24.
//

#ifndef SERVER_BOUNDED_MAX_PRIORITY_DEQUE_H
#define SERVER_BOUNDED_MAX_PRIORITY_DEQUE_H

#include "BoundedPriorityDequeBase.h"

template<typename K, typename V>
class BoundedMaxPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    [[nodiscard]] bool compare(K a, K b) const override { return a > b; }

public:
    explicit BoundedMaxPriorityDeque(unsigned int capacity = 0) : BoundedPriorityDequeBase<K, V>(capacity) {}

};


#endif //SERVER_BOUNDED_MAX_PRIORITY_DEQUE_H

