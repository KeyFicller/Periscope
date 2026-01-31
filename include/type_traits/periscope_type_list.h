#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------
// type_list is a list of types
template<typename... types>
struct type_list
{
    using type = type_list;
    static constexpr auto size = sizeof...(types);
};

// type_list_element is nth element of type_list
template<std::size_t index, typename list>
struct type_list_element;

// type_list_element_t is nth element of type_list
template<std::size_t index, typename list>
using type_list_element_t = typename type_list_element<index, list>::type;

// type_list_index is index of type in type_list
template<typename type, typename list>
struct type_list_index;

// type_list_index_v is index of type in type_list
template<typename type, typename list>
inline constexpr std::size_t type_list_index_v = type_list_index<type, list>::value;

// operator+ is concatenation of type_lists
template<typename... types, typename... others>
constexpr type_list<types..., others...>
operator+(type_list<types...>, type_list<others...>)
{
    return {};
}

// type_list_concat is concatenation of type_lists
template<typename...>
struct type_list_concat;

// type_list_concat_t is concatenation of type_lists
template<typename... lists>
using type_list_concat_t = typename type_list_concat<lists...>::type;

namespace unique {
template<typename...>
struct type_list_unique;
}

// type_list_unique is unique of type_lists
template<typename list>
struct type_list_unique
{
    using type = typename unique::type_list_unique<list>::type;
};

// type_list_unique_t is unique of type_lists
template<typename list>
using type_list_unique_t = typename type_list_unique<list>::type;

// type_list_contains is whether the type is in the list
template<typename list, typename type>
struct type_list_contains;

// type_list_contains_v is whether the type is in the list
template<typename list, typename type>
inline constexpr bool type_list_contains_v = type_list_contains<list, type>::value;

// type_list_contains_derived is whether the type is derived from the type in the list
template<typename list, typename type>
struct type_list_contains_derived;

// type_list_contains_derived_v is whether the type is derived from the type in the list
template<typename list, typename type>
inline constexpr bool type_list_contains_derived_v = type_list_contains_derived<list, type>::value;

// type_list_diff is difference of type_lists
template<typename...>
struct type_list_diff;

// type_list_diff_t is difference of type_lists
template<typename... lists>
using type_list_diff_t = typename type_list_diff<lists...>::type;

// type_list_transform is transform of type_lists
template<typename list, template<typename> class transformer>
struct type_list_transform;

// type_list_transform_t is transform of type_lists
template<typename list, template<typename> class transformer>
using type_list_transform_t = typename type_list_transform<list, transformer>::type;

// ---------------------- Specialization(this) ------------------
// Recursive removal of the first type from the list
template<std::size_t index, typename first, typename... others>
struct type_list_element<index, type_list<first, others...>> : type_list_element<index - 1u, type_list<others...>>
{};

// Base case: the first type
template<typename first, typename... others>
struct type_list_element<0u, type_list<first, others...>>
{
    using type = first;
};

// Recursive search for the index of the type in the list
template<typename type, typename first, typename... others>
struct type_list_index<type, type_list<first, others...>>
{
    using value_type = std::size_t;
    static constexpr value_type value = 1u + type_list_index<type, type_list<others...>>::value;
};

// Base case: the type is the first type
template<typename type, typename... others>
struct type_list_index<type, type_list<type, others...>>
{
    static_assert(type_list_index<type, type_list<others...>>::value == sizeof...(others),
                  "Type appears multiple times in the list");
    using value_type = std::size_t;
    static constexpr value_type value = 0u;
};

// Base case: helper to judge if the type appears multiple times in the list
template<typename type>
struct type_list_index<type, type_list<>>
{
    using value_type = std::size_t;
    static constexpr value_type value = 0u;
};

// Base case: concatenation of empty lists
template<>
struct type_list_concat<type_list<>>
{
    using type = type_list<>;
};

// Base case: concatenation of one list
template<typename... types>
struct type_list_concat<type_list<types...>>
{
    using type = type_list<types...>;
};

// Recursive concatenation of lists
template<typename... types, typename... others, typename... lists>
struct type_list_concat<type_list<types...>, type_list<others...>, lists...>
{
    using type = type_list_concat<type_list<types..., others...>, lists...>::type;
};

namespace unique {
// Recursive append the unique first type of the list to the result types
template<typename first, typename... others, typename... types>
struct type_list_unique<type_list<first, others...>, types...>
  : std::conditional_t<(std::is_same_v<first, others> || ...),
                       type_list_unique<type_list<others...>, types...>,
                       type_list_unique<type_list<others...>, types..., first>>
{};

// Base case: all the types in type_list are handles
template<typename... types>
struct type_list_unique<type_list<>, types...>
{
    using type = type_list<types...>;
};
}

// Search for the type in the list
template<typename... types, typename type>
struct type_list_contains<type_list<types...>, type> : std::bool_constant<(std::is_same_v<types, type> || ...)>
{};

// Search for the type in the list is derived from the type in the list
template<typename... types, typename type>
struct type_list_contains_derived<type_list<types...>, type>
  : std::bool_constant<(std::is_base_of_v<types, type> || ...)>
{};

// Search for all types in the first list but not in the second list
template<typename... types, typename... others>
struct type_list_diff<type_list<types...>, type_list<others...>>
{
    using type = type_list_concat_t<
      std::conditional_t<type_list_contains_v<type_list<others...>, types>, type_list<>, type_list<types>>...>;
};

// Transform the type_list by the transformer
template<typename... types, template<typename> class transformer>
struct type_list_transform<type_list<types...>, transformer>
{
    using type = type_list<typename transformer<types>::type...>;
};
}

template<typename... types>
struct std::tuple_size<periscope::type_list<types...>>
  : std::integral_constant<std::size_t, periscope::type_list<types...>::size>
{};

template<std::size_t index, typename... types>
struct std::tuple_element<index, periscope::type_list<types...>>
  : periscope::type_list_element<index, periscope::type_list<types...>>
{};