#pragma once

#include "type_hash/periscope_type_hash.h"
#include "type_traits/periscope_type_list.h"
#include <string>

namespace periscope {
// ------------------------ Main template -----------------------

class base_object;

// enum_type is type alias for enum values
using enum_type = unsigned int;

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
{};

// OP_printable is property for whether object is printable
struct OP_printable : base_property<bool, base_object>
{};

// OP_type is property for object type hash
struct OP_type : base_property<type_hash_result, base_object>
{};
}