#pragma once

#include "periscope_type_list.h"
#include <iterator>
#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------
// memeber_class is the class of a member function or method
template<typename member>
struct member_class
{
    static_assert(std::is_member_pointer_v<member>, "member is not a member function or method");

    template<typename klass, typename type>
    static constexpr klass pick_up(type klass::*);

    template<typename klass, typename ret, typename... args>
    static constexpr klass pick_up(ret (klass::*)(args...));

    template<typename klass, typename ret, typename... args>
    static constexpr klass pick_up(ret (klass::*)(args...) const);

  public:
    using type = std::remove_pointer_t<decltype(pick_up(std::declval<member>()))>;
};

// member_class_t is the class of a member function or method
template<typename member>
using member_class_t = typename member_class<member>::type;

// nth_argument is the nth argument of a function or method
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

// nth_argument_t is the nth argument of a function or method
template<std::size_t index, auto candidate>
using nth_argument_t = typename nth_argument<index, candidate>::type;

// return_argument is the return type of a function or method
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

// return_argument_t is the return type of a function or method
template<auto candidate>
using return_argument_t = typename return_argument<candidate>::type;

// is_applicable is whether the function or method is applicable to the argument
template<typename, typename>
struct is_applicable : std::false_type
{};

// is_applicable_v is whether the function or method is applicable to the argument
template<typename func, typename tuple_like>
inline constexpr bool is_applicable_v = is_applicable<func, tuple_like>::value;

// is_applicable_r is whether the function or method with return type is applicable to the argument
template<typename, typename, typename>
struct is_applicable_r : std::false_type
{};

// is_applicable_r_v is whether the function or method with return type is applicable to the argument
template<typename func, typename tuple_like, typename return_type>
inline constexpr bool is_applicable_r_v = is_applicable_r<func, tuple_like, return_type>::value;

// is_complete is whether the type is a complete type
template<typename type, typename = void>
struct is_complete : std::false_type
{};

// is_complete_v is whether the type is a complete type
template<typename type>
inline constexpr bool is_complete_v = is_complete<type>::value;

// is_iterator is whether the type is an iterator
template<typename type, typename = void>
struct is_iterator : std::false_type
{};

// is_iterator_v is whether the type is an iterator
template<typename type>
inline constexpr bool is_iterator_v = is_iterator<type>::value;

// is_ebco_eligible is whether the type is eligible for EBCO
template<typename type>
struct is_ebco_eligible : std::bool_constant<std::is_empty_v<type> && !std::is_final_v<type>>
{};

// is_ebco_eligible_v is whether the type is eligible for EBCO
template<typename type>
inline constexpr bool is_ebco_eligible_v = is_ebco_eligible<type>::value;

// is_transparent is whether the type is a transparent type
template<typename type, typename = void>
struct is_transparent : std::false_type
{};

// is_transparent_v is whether the type is a transparent type
template<typename type>
inline constexpr bool is_transparent_v = is_transparent<type>::value;

namespace internal {
template<typename, typename = void>
struct has_tuple_size_value : std::false_type
{};

template<typename, typename = void>
struct has_value_type : std::false_type
{};

template<typename>
[[nodiscard]] constexpr bool
dispatch_is_equality_comparable();

template<typename type, std::size_t... indices>
[[nodiscard]] constexpr bool
unpack_maybe_equality_comparable(std::index_sequence<indices...>)
{
    return (dispatch_is_equality_comparable<std::tuple_element_t<indices, type>>() && ...);
}

template<typename>
[[nodiscard]] constexpr bool
maybe_equality_comparable(char)
{
    return false;
}

template<typename type>
[[nodiscard]] constexpr auto
maybe_equality_comparable(int) -> decltype(std::declval<type>() == std::declval<type>())
{
    return true;
}

template<typename type>
[[nodiscard]] constexpr bool
dispatch_is_equality_comparable()
{
    if constexpr (std::is_array_v<type>) {
        return false;
    } else if constexpr (is_complete_v<std::tuple_size<std::remove_cv_t<type>>>) {
        if constexpr (has_tuple_size_value<type>::value) {
            return maybe_equality_comparable<type>(0) &&
                   unpack_maybe_equality_comparable<type>(std::make_index_sequence<std::tuple_size_v<type>>());
        } else {
            return maybe_equality_comparable<type>(0);
        }
    } else if constexpr (has_value_type<type>::value) {
        if constexpr (is_iterator_v<type> || std::is_same_v<typename type::value_type, type> ||
                      dispatch_is_equality_comparable<typename type::value_type>()) {
            return maybe_equality_comparable<type>(0);
        } else {
            return false;
        }
    } else {
        return maybe_equality_comparable<type>(0);
    }
}
}
// is_equality_comparable is whether the type is a equality comparable type
template<typename type>
struct is_equality_comparable : std::bool_constant<dispatch_is_equality_comparable<type>()>
{};

// is_equality_comparable_v is whether the type is a equality comparable type
template<typename type>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<type>::value;

// constness_as is fork the constness of the type to another type
template<typename from, typename target>
struct constness_as
{
    using type = std::remove_const_t<from>;
};

template<typename from, typename target>
struct constness_as<from, const target>
{
    using type = target const;
};

// constness_as_t is the type of constness_as
template<typename from, typename target>
using constness_as_t = typename constness_as<from, target>::type;

// ---------------------- Specialization(this) ------------------
// check tuple like type can be arguments for a function or method
template<typename func, template<typename...> class tuple_like, typename... args>
struct is_applicable<func, tuple_like<args...>> : std::is_invocable<func, args...>
{};

// check const tuple like type can be arguments for a function or method
template<typename func, template<typename...> class tuple_like, typename... args>
struct is_applicable<func, tuple_like<args...> const> : std::is_invocable<func, args...>
{};

// check tuple like type can be arguments for a function or method with return type
template<typename ret, typename func, template<typename...> class tuple_like, typename... args>
struct is_applicable_r<ret, func, tuple_like<args...>> : std::is_invocable_r<ret, func, args...>
{};

// check const tuple like type can be arguments for a function or method with return type
template<typename ret, typename func, template<typename...> class tuple_like, typename... args>
struct is_applicable_r<ret, func, tuple_like<args...> const> : std::is_invocable_r<ret, func, args...>
{};

// check if the type is a complete type
template<typename type>
struct is_complete<type, std::void_t<decltype(sizeof(type))>> : std::true_type
{};

namespace internal {
template<typename, typename = void>
struct has_iterator_category : std::false_type
{};

template<typename type>
struct has_iterator_category<type, std::void_t<typename std::iterator_traits<type>::iterator_category>> : std::true_type
{};
}

// check if the type is an iterator
template<typename type>
struct is_iterator<type, std::enable_if_t<!std::is_void_v<std::remove_cvref_t<type>>>>
  : internal::has_iterator_category<type>
{};

// check if the type is a transparent type
template<typename type>
struct is_transparent<type, std::void_t<typename type::is_transparent>> : std::true_type
{};

namespace internal {
template<typename type>
struct has_tuple_size_value<type, std::void_t<decltype(std::tuple_size_v<const type>)>> : std::true_type
{};

template<typename type>
struct has_value_type<type, std::void_t<typename type::value_type>> : std::true_type
{};
}
// const
template<typename type>
struct is_equality_comparable<const type> : is_equality_comparable<type>
{};

}