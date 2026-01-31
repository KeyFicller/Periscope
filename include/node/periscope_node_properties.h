#pragma once

#include "object/periscope_object_properties.h"

namespace periscope {

class node;

struct NP_shape : base_property<enum_type, node>
{
    enum shape : enum_type
    {
        k_circle,
        k_rectangle,
    };
};
}