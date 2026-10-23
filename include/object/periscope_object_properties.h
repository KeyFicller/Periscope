#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "type_hash/periscope_type_hash.h"
#include "type_traits/periscope_type_list.h"
#include <memory>
#include <string>
#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------

class base_object;

// enum_type is type alias for enum values
using enum_type = unsigned int;

// has_get_handle is to check if the type has get_handle method
template<typename t>
struct has_get_handle
{
  private:
    template<typename v>
    static auto test(int) -> decltype(std::declval<v>().get_handle(), std::true_type{});

    template<typename>
    static std::false_type test(...);

  public:
    static constexpr bool value = decltype(test<t>(0))::value;
};

// helper trait to extract handle from value
template<typename t>
static auto
extract_handle(const t& _value)
{
    if constexpr (has_get_handle<t>::value) {
        return _value.get_handle();
    } else if constexpr (std::is_same_v<t, std::shared_ptr<base_handle>>) {
        return _value;
    } else {
        return _value;
    }
};

// property_of_t maps a nested property enum to its owning property via ADL:
// a property declares hidden friend `friend prop periscope_property_of(enum_name) { return {}; }`
template<typename e>
concept property_enum = std::is_enum_v<e> && requires(e _value) { periscope_property_of(_value); };

template<typename e>
using property_of_t = decltype(periscope_property_of(std::declval<e>()));

// property_literal tags a value with its owning property so set("..."_op_name) /
// set("9:0"_np_time_tick_start) deduce the property without an explicit template arg
template<typename prop>
struct property_literal
{
    using property = prop;
    typename prop::type Value;
};

template<typename t>
struct is_property_literal : std::false_type
{};

template<typename prop>
struct is_property_literal<property_literal<prop>> : std::true_type
{};

template<typename t>
concept property_literal_value = is_property_literal<std::remove_cvref_t<t>>::value;

// ADL hook so property_of_t also works for property_literal (returns the property type)
template<typename prop>
prop
periscope_property_of(const property_literal<prop>&)
{
    return {};
}

namespace detail {
// Owning property key for a deduced set()/new_object argument (UDL or nested enum).
// Uniqueness is by property type (OP_name / NP_shape / …), not by value.
template<typename arg>
struct deduced_property_key;

template<typename prop>
struct deduced_property_key<property_literal<prop>>
{
    using type = prop;
};

template<typename e>
    requires property_enum<e>
struct deduced_property_key<e>
{
    using type = property_of_t<e>;
};

template<typename arg>
using deduced_property_key_t = typename deduced_property_key<std::remove_cvref_t<arg>>::type;

// Typed uniqueness gate: when Unique is false, the failing Prop appears in the
// instantiation stack (require_unique_owning_property<OP_name, false>).
template<typename prop, bool unique>
struct require_unique_owning_property
{
    static_assert(unique,
                  "periscope: duplicate owning property in deduced pack — each property "
                  "may appear at most once (see Prop in this instantiation; e.g. not two "
                  "OP_name / NP_shape values in one new_object / >> chain)");
    static constexpr bool value = unique;
};

// True when every property in the pack appears once (first duplicate is named as Prop).
template<typename... props>
struct owning_properties_unique;

template<>
struct owning_properties_unique<> : std::true_type
{};

template<typename first, typename... rest>
struct owning_properties_unique<first, rest...>
  : std::bool_constant<require_unique_owning_property<first, !type_list_contains_v<type_list<rest...>, first>>::value &&
                       owning_properties_unique<rest...>::value>
{};

template<typename... args>
inline constexpr bool deduced_args_unique_v =
  owning_properties_unique<deduced_property_key_t<args>...>::value;
} // namespace detail

// base_property is base class for all properties
template<typename value_type, typename owner>
struct base_property
{
    using type = value_type;
    type Value{};

    using owner_type = owner;
    using child_properties = type_list<>;
    using parent_properties = type_list<>;
};

// OP_name is property for object name
struct OP_name : base_property<std::string, base_object>
{
    static std::string to_string(const OP_name& _property, graph_type _graph_type = io().GraphType)
    {
        (void)_graph_type;
        return _property.Value;
    }
};

// "Start"_op_name -> property_literal<OP_name> (OP_ = object_property)
inline property_literal<OP_name>
operator""_op_name(const char* _str, std::size_t _len)
{
    return property_literal<OP_name>{ std::string(_str, _len) };
}

// OP_printable is property for whether object is printable
struct OP_printable : base_property<bool, base_object>
{};

// OP_type is property for object type hash
struct OP_type : base_property<type_hash_result, base_object>
{};
}
