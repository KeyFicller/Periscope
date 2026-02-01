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
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        std::string arrow_str = _V_str<LP_style>();
        switch (_graph_type) {
            case graph_type::k_flowchart: {
                if (has<OP_name>()) {
                    std::string pre = arrow_str.substr(0, 2);
                    arrow_str = pre + "\"" + _V_str<OP_name>() + "\"" + arrow_str.substr((int)arrow_str.size() - 3, 3);
                }
                return std::format(
                  "{} {} {}", get<LP_source>().Value->print(), arrow_str, get<LP_target>().Value->print());
            }
            case graph_type::k_sequence: {
                arrow_str += _V_str<LP_activate>();
                return std::format("{} {} {} : {}",
                                   get<LP_source>().Value->print(),
                                   arrow_str,
                                   get<LP_target>().Value->print(),
                                   _V_str<OP_name>());
            }
            default:
                throw std::runtime_error("Unsupported graph type for link");
        }
    }
};
}