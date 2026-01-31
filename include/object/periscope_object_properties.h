#pragma once

#include "type_hash/periscope_type_hash.h"
#include "type_traits/periscope_type_list.h"
#include <string>

namespace periscope {
// ------------------------ Main template -----------------------

class base_object;

using enum_type = unsigned int;

template<typename value_type, typename owner>
struct base_property
{
    using type = value_type;
    type Value{};

    using owner_type = owner;
    using child_properties = type_list<>;
    using parent_properties = type_list<>;
};

struct OP_name : base_property<std::string, base_object>
{};

struct OP_printable : base_property<bool, base_object>
{};

struct OP_type : base_property<type_hash_result, base_object>
{};
}