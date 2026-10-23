#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "type_hash/periscope_type_hash.h"
#include "type_traits/periscope_type_list.h"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

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
// set("9:0"_np_time_tick_start) deduce the property without an explicit template arg.
// For vector-of-handle props (NP_subgraph_node / MNP_basis), a pack of get_handle()
// owners (node&, note&, …) builds the vector — no .get_handle() at the call site.
template<typename prop>
struct property_literal
{
    using property = prop;
    typename prop::type Value{};

    property_literal() = default;
    property_literal(const typename prop::type& _value)
      : Value(_value)
    {
    }
    property_literal(typename prop::type&& _value)
      : Value(std::move(_value))
    {
    }

    // Handle-vector props: property_literal<NP_subgraph_node>{ a, b } packs a.get_handle()…
    template<typename first, typename... rest>
        requires requires { typename prop::type::value_type; } &&
                 std::is_same_v<typename prop::type::value_type, std::shared_ptr<base_handle>> &&
                 has_get_handle<std::remove_cvref_t<first>>::value &&
                 (... && has_get_handle<std::remove_cvref_t<rest>>::value)
    property_literal(first&& _first, rest&&... _rest)
      : Value{ extract_handle(std::forward<first>(_first)), extract_handle(std::forward<rest>(_rest))... }
    {
    }
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

// property_reduce: how duplicate owning keys in a deduced pack / >> chain fold.
// replace (default, scalars): at most one appearance (pack-level uniqueness).
// append (vector-valued props): multiple appearances concatenate into one slot.
// bitwise_or: reserved for flag masks (not folded yet).
enum class property_reduce
{
    replace,
    append,
    bitwise_or,
};

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

// Sequence that can be concatenated (std::vector and friends).
template<typename t>
concept appendable_sequence = requires(t& seq, const t& other) {
    typename t::value_type;
    seq.insert(seq.end(), other.begin(), other.end());
};

// Only replace-strategy keys participate in pack uniqueness; append keys may repeat.
template<typename prop>
using replace_key_list = std::conditional_t<prop::reduce == property_reduce::replace, type_list<prop>, type_list<>>;

// Leading type_list<> so empty packs still form a valid concat (new_object<link>()).
template<typename... props>
using replace_keys_t = type_list_concat_t<type_list<>, replace_key_list<props>...>;

// Pack-level uniqueness among replace keys only (sizeof pack_replace == unique).
// Shared by new_object / >>. Append (vector) keys are excluded — folded at apply time.
template<typename... props>
inline constexpr bool owning_properties_unique_v =
  replace_keys_t<props...>::size == type_list_unique_t<replace_keys_t<props...>>::size;

template<typename... args>
inline constexpr bool deduced_args_unique_v = owning_properties_unique_v<deduced_property_key_t<args>...>;
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
    // Default: scalar uniqueness. Vector props override to append.
    static constexpr property_reduce reduce = property_reduce::replace;
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
