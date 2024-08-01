//
// Created by Cooper Larson on 7/31/24.
//

#ifndef BOUNDED_HEAP_BOUNDED_PRIORITY_DEQUE_H
#define BOUNDED_HEAP_BOUNDED_PRIORITY_DEQUE_H

#include "BoundedPriorityDequeBase.h"

template<typename K, typename V, typename Comparator = std::less<K>>
class BoundedPriorityDeque : public BoundedPriorityDequeBase<K, V> {
    using Node = BoundedPriorityDequeBase<K, V>::Node;

protected:
    Comparator comparator;

    [[nodiscard]] bool compareElements(K a, K b) const override { return comparator(a, b); }

public:
    explicit BoundedPriorityDeque(unsigned int capacity = 0, Comparator comp = Comparator()) :
        BoundedPriorityDequeBase<K, V>(
                [](const Node* a, const Node* b) { return Comparator()(a->_data.first, b->_data.first); },
                    capacity), comparator(comp) {}

};

#endif //BOUNDED_HEAP_BOUNDED_PRIORITY_DEQUE_H
