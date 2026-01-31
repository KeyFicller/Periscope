#pragma once

#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"

namespace periscope {
// ------------------------ Main template -----------------------

class link;

// LP_source is property for link source handle
struct LP_source : public base_property<std::shared_ptr<base_handle>, link>
{
    using owner_type = link;
};

// LP_target is property for link target handle
struct LP_target : public base_property<std::shared_ptr<base_handle>, link>
{
    using owner_type = link;
};

}