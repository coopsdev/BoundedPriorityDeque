//
// Created by Cooper Larson on 7/31/24.
//

#include <gtest/gtest.h>
#include <thread>
#include <random>
#include <vector>
#include <algorithm>
#include "include/BoundedPriorityDeque.hpp"

TEST(BoundingPairTest, Comparison) {
    BoundingPair<int, std::string> pair1(10, "ten");
    BoundingPair<int, std::string> pair2(20, "twenty");
    ASSERT_TRUE(pair1 < pair2);
}

TEST(DequeTest, OneItemCapacity) {
    BoundedMinPriorityDeque<double, std::string> deque(1);
    deque.emplace(4, "four");
    deque.emplace(8, "eight");
    deque.emplace(2, "two");
    deque.emplace(1, "one");

    ASSERT_EQ(deque.pop().value, "one");
}

TEST(MinDequeTest, BasicOperations) {
    BoundedMinPriorityDeque<int, std::string> deque(2);
    deque.push(BoundingPair<int, std::string>(10, "ten"));
    deque.push(BoundingPair<int, std::string>(20, "twenty"));
    deque.push(BoundingPair<int, std::string>(5, "five"));

    ASSERT_EQ(deque.top().value, "five");
    ASSERT_EQ(deque.pop().value, "five");
    ASSERT_EQ(deque.top().value, "ten");
    ASSERT_TRUE(deque.pop().value == "ten");
    ASSERT_TRUE(deque.empty());
}

TEST(MaxDequeTest, BasicOperations) {
    BoundedMaxPriorityDeque<int, std::string> deque(2);
    deque.push(BoundingPair<int, std::string>(10, "ten"));
    deque.push(BoundingPair<int, std::string>(20, "twenty"));
    deque.push(BoundingPair<int, std::string>(5, "five"));

    ASSERT_EQ(deque.top().value, "twenty");
    ASSERT_EQ(deque.pop().value, "twenty");
    ASSERT_EQ(deque.top().value, "ten");
    ASSERT_EQ(deque.pop().value, "ten");
    ASSERT_TRUE(deque.empty());
}

TEST(BoundedDequeTest, CustomComparator) {
    BoundedPriorityDeque<int, std::string, std::greater<>> deque(10, std::greater<>());
    deque.push(BoundingPair<int, std::string>(10, "ten"));
    deque.push(BoundingPair<int, std::string>(20, "twenty"));
    deque.push(BoundingPair<int, std::string>(5, "five"));

    ASSERT_EQ(deque.top().value, "twenty");
    ASSERT_EQ(deque.pop().value, "twenty");
    ASSERT_EQ(deque.top().value, "ten");
}

TEST(BoundedDequeTest, OverflowBehavior) {
    BoundedMinPriorityDeque<int, std::string> deque(3);
    deque.push(BoundingPair<int, std::string>(4, "four"));
    deque.push(BoundingPair<int, std::string>(1, "one"));
    deque.push(BoundingPair<int, std::string>(3, "three"));
    deque.push(BoundingPair<int, std::string>(2, "two"));  // Should push out "four"

    ASSERT_EQ(deque.top().value, "one");
    ASSERT_EQ(deque.size(), 3);
    deque.push(BoundingPair<int, std::string>(5, "five")); // Should not change the deque
    ASSERT_EQ(deque.size(), 3);
    ASSERT_EQ(deque.pop().value, "one");
    ASSERT_EQ(deque.pop().value, "two");
    ASSERT_EQ(deque.pop().value, "three");
    ASSERT_TRUE(deque.empty());
}

TEST(BoundedDequeTest, Emptydeque) {
    BoundedMaxPriorityDeque<int, std::string> deque(2);
    ASSERT_TRUE(deque.empty());
    try {
        deque.top();
        FAIL() << "Expected std::exception due to empty deque";
    } catch (const std::exception& e) {
        ASSERT_STREQ("Attempted to access top element of empty BoundedPriorityDeque", e.what());
    }
}

TEST(BoundedDequeTest, Merge) {
    BoundedMinPriorityDeque<int, std::string> a(5), b(5);
    a.emplace(2, "two");
    a.emplace(5, "five");
    a.emplace(7, "seven");
    a.emplace(12, "twelve");

    b.emplace(1, "one");
    b.emplace(3, "three");
    b.emplace(9, "nine");
    b.emplace(4, "four");

    a += b;
    ASSERT_EQ(a.pop().value, "one");
    ASSERT_EQ(a.pop().value, "two");
    ASSERT_EQ(a.pop().value, "three");
    ASSERT_EQ(a.pop().value, "four");
    ASSERT_EQ(a.pop().value, "five");
    ASSERT_TRUE(a.empty());
}

class ConcurrentDequeTest : public ::testing::Test {
protected:
    BoundedMinPriorityDeque<int, std::string> deque;
    std::mutex mutex;
    static constexpr int kMaxItems = 1000;
    static constexpr int kCapacity = 100;

    ConcurrentDequeTest() : deque(kCapacity) {}

    void fillDequeConcurrently(int num_threads, int items_per_thread) {
        auto pushToDeque = [this](int seed, int count) {
            std::mt19937 generator(seed);
            std::uniform_int_distribution<int> distribution(1, 10000);
            for (int i = 0; i < count; ++i) {
                int num = distribution(generator);
                std::lock_guard<std::mutex> lock(this->mutex);
                this->deque.push(BoundingPair<int, std::string>(num, "Value: " + std::to_string(num)));
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(pushToDeque, i, items_per_thread);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
};

//TEST_F(ConcurrentDequeTest, MultiThreadedPush) {
//    const int num_threads = 2;
//    const int num_operations_per_thread = kMaxItems / num_threads;
//
//    fillDequeConcurrently(num_threads, num_operations_per_thread);
//
//    // Verify that the capacity is not exceeded
//    ASSERT_LE(deque.size(), kCapacity);
//
//    // Verify the consistency and integrity of the deque
//    std::vector<int> values;
//    while (!deque.empty()) {
//        std::string topValue = deque.top().value;
//        values.push_back(std::stoi(topValue.substr(7))); // Extract the number from the string
//        deque.pop();
//    }
//
//    // Ensure all values are unique since they should be
//    std::sort(values.begin(), values.end());
//    auto last = std::unique(values.begin(), values.end());
//    ASSERT_TRUE(last == values.end());
//
//    // Ensure values are in increasing order since it's a min-deque
//    for (size_t i = 1; i < values.size(); ++i) {
//        ASSERT_LE(values[i - 1], values[i]);
//    }
//}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
