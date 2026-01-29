#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace periscope {
// ------------------------ Main template -----------------------
// value_list is a list of values
template<auto... values>
struct value_list
{
    using type = value_list;
    static constexpr auto size = sizeof...(values);
};

// value_list_element is nth element of value_list
template<std::size_t index, typename list>
struct value_list_element;

// value_list_element_t is nth element of value_list
template<std::size_t index, typename list>
inline constexpr auto value_list_element_v = value_list_element<index, list>::value;

// value_list_index is index of value in value_list
template<auto val, typename list>
struct value_list_index;

// value_list_index_v is index of value in value_list
template<auto val, typename list>
inline constexpr std::size_t value_list_index_v = value_list_index<val, list>::value;

// operator+ is concatenation of value_lists
template<auto... vals, auto... others>
constexpr value_list<vals..., others...>
operator+(value_list<vals...>, value_list<others...>)
{
    return {};
};

// value_list_concat is concatenation of value_lists
template<typename...>
struct value_list_concat;

// value_list_concat_t is concatenation of value_lists
template<typename... lists>
using value_list_concat_t = typename value_list_concat<lists...>::type;

// value_list_unique is unique of value_lists
template<typename>
struct value_list_unique;

// value_list_unique_t is unique of value_lists
template<typename list>
using value_list_unique_t = typename value_list_unique<list>::type;

// value_list_contains is whether the value is in the list
template<typename list, auto val>
struct value_list_contains;

// value_list_contains_v is whether the value is in the list
template<typename list, auto val>
inline constexpr bool value_list_contains_v = value_list_contains<list, val>::value;

// value_list_diff is difference of value_lists
template<typename...>
struct value_list_diff;

// value_list_diff_t is difference of value_lists
template<typename... lists>
using value_list_diff_t = typename value_list_diff<lists...>::type;

// value_list_transform is transform of value_lists
template<typename list, template<auto> class transformer>
struct value_list_transform;

// value_list_transform_t is transform of value_lists
template<typename list, template<auto> class transformer>
using value_list_transform_t = typename value_list_transform<list, transformer>::type;

// ---------------------- Specialization(this) ------------------
// Recursive removal of the first value from the list
template<std::size_t index, auto... vals>
struct value_list_element<index, value_list<vals...>> : value_list_element<index - 1u, value_list<vals...>>
{};

// Base case: get the first value
template<auto val, auto... vals>
struct value_list_element<0u, value_list<val, vals...>>
{
    using value_type = decltype(val);
    static constexpr value_type value = val;
};

// Recursive search for the index of the value in the list
template<auto val, auto first, auto... others>
struct value_list_index<val, value_list<first, others...>>
{
    using value_type = std::size_t;
    static constexpr value_type value = 1u + value_list_index<val, value_list<others...>>::value;
};

// Base case: the value is the first value
template<auto first, auto... others>
struct value_list_index<first, value_list<first, others...>>
{
    static_assert(value_list_index<first, value_list<others...>>::value == sizeof...(others),
                  "Value appears multiple times in the list");
    using value_type = std::size_t;
    static constexpr value_type value = 0u;
};

// Base case: helper to judge if the value appears multiple times in the list
template<auto val>
struct value_list_index<val, value_list<>>
{
    using value_type = std::size_t;
    static constexpr value_type value = 0u;
};

// Base case: concatenation of empty lists
template<>
struct value_list_concat<value_list<>>
{
    using type = value_list<>;
};

// Base case: concatenation of one list
template<auto... vals>
struct value_list_concat<value_list<vals...>>
{
    using type = value_list<vals...>;
};

// Recursive concatenation of lists
template<auto... vals, auto... others, typename... lists>
struct value_list_concat<value_list<vals...>, value_list<others...>, lists...>
{
    using type = value_list_concat<value_list<vals..., others...>, lists...>::type;
};

// Recursive unique object by concatenation
template<auto val, auto... others>
struct value_list_unique<value_list<val, others...>>
{
    using type =
      std::conditional_t<((val == others) || ...),
                         typename value_list_unique<value_list<others...>>::type,
                         value_list_concat_t<value_list<val>, typename value_list_unique<value_list<others...>>::type>>;
};

// Base case: emtpy list
template<>
struct value_list_unique<value_list<>>
{
    using type = value_list<>;
};

// Search for the value in the list
template<auto... vals, auto val>
struct value_list_contains<value_list<vals...>, val> : std::bool_constant<((vals == val) || ...)>
{};

// Search for all values in the first list but not in the second list
template<auto... vals, auto... others>
struct value_list_diff<value_list<vals...>, value_list<others...>>
{
    using type = value_list_concat_t<
      std::conditional_t<value_list_contains_v<value_list<others...>, vals>, value_list<>, value_list<vals>>...>;
};

// Transform the value_list by the transformer
template<auto... vals, template<auto> class transformer>
struct value_list_transform<value_list<vals...>, transformer>
{
    using type = value_list<transformer<vals>::value...>;
};
}

template<auto... vals>
struct std::tuple_size<periscope::value_list<vals...>>
  : std::integral_constant<std::size_t, periscope::value_list<vals...>::size>
{};

template<std::size_t index, auto... vals>
struct std::tuple_element<index, periscope::value_list<vals...>>
  : periscope::value_list_element<index, periscope::value_list<vals...>>
{};