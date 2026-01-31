#pragma once

#include "object/periscope_object_properties.h"

namespace periscope {
// ------------------------ Main template -----------------------

class node;

// NP_shape is property for node shape
struct NP_shape : base_property<enum_type, node>
{
    enum shape : enum_type
    {
        k_circle,
        k_rectangle,
    };
};
}