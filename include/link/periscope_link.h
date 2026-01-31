#pragma once

#include "link/periscope_link_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include <format>

namespace periscope {
// ------------------------ Main template -----------------------

// link is graph link object connecting nodes
class link : public object<link>
{
  public:
    link() { set<OP_printable>(true); }

  public:
    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type graph_type) const
    {
        switch (graph_type) {
            case graph_type::k_flowchart: {
                return std::format("{} {} {}",
                                   get<LP_source>().Value->print(graph_type),
                                   "-->",
                                   get<LP_target>().Value->print(graph_type));
            }
            case graph_type::k_sequence: {
                return std::format("{} {} {} : {}",
                                   get<LP_source>().Value->print(graph_type),
                                   "->>",
                                   get<LP_target>().Value->print(graph_type),
                                   "test");
            }
        }
    }
};
}