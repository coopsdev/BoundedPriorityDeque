//
// Created by Cooper Larson on 7/31/24.
//

#ifndef BOUNDED_HEAP_BOUNDINGPAIR_H
#define BOUNDED_HEAP_BOUNDINGPAIR_H

#include <utility>
#include <type_traits>

// Helper type traits to check if a type has a comparison operator
template <typename, typename T>
struct has_comparison_operator {
    static_assert(
            std::integral_constant<T, false>::value,
            "Second template parameter needs to be of function type.");
};

template <typename C, typename Ret, typename... Args>
struct has_comparison_operator<C, Ret(Args...)> {
private:
    template <typename T>
    static constexpr auto check(T*) ->
    typename std::is_same<decltype(std::declval<T>().operator()(std::declval<Args>()...)), Ret>::type;

    template <typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

template <typename T>
constexpr bool has_comparison_operator_v = has_comparison_operator<T, bool(const T&, const T&)>::value;

// BoundingPair class template
template<typename K, typename V, typename Enable = void>
class BoundingPair : public std::pair<K, V> {
    using pair = std::pair<K, V>;
public:
    using pair::pair;
    using pair::first;
    using pair::second;

    // Comparison operator
    bool operator<(const BoundingPair& other) const {
        return first < other.first;
    }
};

// Specialization for comparator objects
template<typename K, typename V>
class BoundingPair<K, V, std::enable_if_t<has_comparison_operator_v<K>>> : public std::pair<K, V> {
    using pair = std::pair<K, V>;
public:
    using pair::pair;
    using pair::first;
    using pair::second;

    // Comparison operator using the comparator
    bool operator<(const BoundingPair& other) const {
        return K()(first, other.first);
    }
};

#endif //BOUNDED_HEAP_BOUNDINGPAIR_H
