#pragma once

#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------
template<typename... types>
struct type_list
{
    using type = type_list;
    static constexpr auto size = sizeof...(types);
};

template<std::size_t, typename>
struct type_list_element;

template<std::size_t index, typename list>
using type_list_element_t = typename type_list_element<index, list>::type;

template<std::size_t index, auto candidate>
class nth_argument
{
    template<typename ret, typename... args>
    static constexpr type_list<args...> pick_up(ret (*)(args...));

    template<typename ret, typename klass, typename... args>
    static constexpr type_list<args...> pick_up(ret (klass::*)(args...));

    template<typename ret, typename klass, typename... args>
    static constexpr type_list<args...> pick_up(ret (klass::*)(args...) const);

    template<typename type, typename klass>
    static constexpr type_list<type> pick_up(type klass::*);

  public:
    using type = type_list_element<index, decltype(pick_up(candidate))>;
};

template<std::size_t index, auto candidate>
using nth_argument_t = typename nth_argument<index, candidate>::type;

template<auto candidate>
class return_argument
{
    template<typename ret, typename... args>
    static constexpr ret pick_up(ret (*)(args...));

    template<typename ret, typename klass, typename... args>
    static constexpr ret pick_up(ret (klass::*)(args...));

    template<typename ret, typename klass, typename... args>
    static constexpr ret pick_up(ret (klass::*)(args...) const);

  public:
    using type = decltype(pick_up(candidate));
};

template<auto candidate>
using return_argument_t = typename return_argument<candidate>::type;

template<typename list, typename elem>
struct type_list_contains
{
    static constexpr bool value = false;
};

template<typename list, typename elem>
constexpr bool type_list_contains_v = type_list_contains<list, elem>::value;

template<typename list, typename elem>
struct type_list_contains_derived
{
    static constexpr bool value = false;
};

template<typename list, typename elem>
constexpr bool type_list_contains_derived_v = type_list_contains_derived<list, elem>::value;

// ---------------------- Specialization(this) ------------------
template<std::size_t index, typename first, typename... others>
struct type_list_element<index, type_list<first, others...>> : type_list_element<index - 1u, type_list<others...>>
{};

template<typename first, typename... others>
struct type_list_element<0u, type_list<first, others...>>
{
    using type = first;
};

template<typename... types, typename elem>
struct type_list_contains<type_list<types...>, elem>
{
    static constexpr bool value = (std::is_same_v<types, elem> || ...);
};

template<typename... types, typename elem>
struct type_list_contains_derived<type_list<types...>, elem>
{
    static constexpr bool value = (std::is_base_of_v<types, elem> || ...);
};

// --------------------- Specialization(other) ------------------

}