#pragma once

#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"

namespace periscope {
class link;

struct LP_source : public base_property<std::shared_ptr<base_handle>, link>
{
    using owner_type = link;
};

struct LP_target : public base_property<std::shared_ptr<base_handle>, link>
{
    using owner_type = link;
};

}