//
// Created by Cooper Larson on 7/31/24.
//

#ifndef BOUNDED_HEAP_BOUNDEDPRIORITYDEQUE_H
#define BOUNDED_HEAP_BOUNDEDPRIORITYDEQUE_H

#include "BoundedPriorityDequeBase.h"
#include <functional> // for std::less

template<typename K, typename V, typename Comparator = std::less<K>>
class BoundedPriorityDeque : public BoundedPriorityDequeBase<K, V> {
protected:
    Comparator comparator;

    using BoundedPriorityDequeBase<K, V>::_container;
    using BoundedPriorityDequeBase<K, V>::_k;

    [[nodiscard]] bool compareElements(K a, K b) const override { return comparator(a, b); }

public:
    explicit BoundedPriorityDeque(unsigned int capacity = 0, Comparator comp = Comparator()) : BoundedPriorityDequeBase<K, V>(capacity), comparator(comp) {}
};

#endif //BOUNDED_HEAP_BOUNDEDPRIORITYDEQUE_H
