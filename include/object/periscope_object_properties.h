#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "type_hash/periscope_type_hash.h"
#include "type_traits/periscope_type_list.h"
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
        return _property.Value;
    }
};

// OP_printable is property for whether object is printable
struct OP_printable : base_property<bool, base_object>
{};

// OP_type is property for object type hash
struct OP_type : base_property<type_hash_result, base_object>
{};
}