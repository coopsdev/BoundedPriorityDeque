//
// Created by Cooper Larson on 7/31/24.
//

#include <gtest/gtest.h>
#include "include/BoundingPair.h"
#include "include/BoundedMinPriorityDeque.h"
#include "include/BoundedMaxPriorityDeque.h"
#include "include/BoundedPriorityDeque.h"

// Helper comparator for testing
struct IntComparator {
    bool operator()(int a, int b) const { return a < b; }
};

// Tests for BoundingPair
TEST(BoundingPairTest, Comparison) {
    BoundingPair<int, std::string> pair1(10, "ten");
    BoundingPair<int, std::string> pair2(20, "twenty");
    ASSERT_TRUE(pair1 < pair2);
}

// Tests for MinHeap
TEST(MinHeapTest, BasicOperations) {
    BoundedMinPriorityDeque<int, std::string> heap(2);
    heap.push(BoundingPair<int, std::string>(10, "ten"));
    heap.push(BoundingPair<int, std::string>(20, "twenty"));
    heap.push(BoundingPair<int, std::string>(5, "five"));

    ASSERT_EQ(heap.top().second, "five");
    ASSERT_EQ(heap.pop().second, "five");
    ASSERT_EQ(heap.top().second, "ten");
}

// Tests for MaxHeap
TEST(MaxHeapTest, BasicOperations) {
    BoundedMaxPriorityDeque<int, std::string> heap(2);
    heap.push(BoundingPair<int, std::string>(10, "ten"));
    heap.push(BoundingPair<int, std::string>(20, "twenty"));
    heap.push(BoundingPair<int, std::string>(5, "five"));

    ASSERT_EQ(heap.top().second, "twenty");
    ASSERT_EQ(heap.pop().second, "twenty");
    ASSERT_EQ(heap.top().second, "ten");
}

// Tests for BoundedHeap with custom comparator
TEST(BoundedHeapTest, CustomComparator) {
    BoundedPriorityDeque<int, std::string, std::greater<>> heap(10, std::greater<>());
    heap.push(BoundingPair<int, std::string>(10, "ten"));
    heap.push(BoundingPair<int, std::string>(20, "twenty"));
    heap.push(BoundingPair<int, std::string>(5, "five"));

    ASSERT_EQ(heap.top().second, "twenty");
    ASSERT_EQ(heap.pop().second, "twenty");
    ASSERT_EQ(heap.top().second, "ten");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
