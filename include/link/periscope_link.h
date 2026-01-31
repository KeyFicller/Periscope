#pragma once

#include "link/periscope_link_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"

namespace periscope {

class link : public object<link>
{
  public:
    link() { set<OP_printable>(true); }

  public:
    std::string to_string_impl() const
    {
        std::string str = "";
        str += m_handle->print() + " ";
        str += get<LP_source>().Value->print() + " -> ";
        str += get<LP_target>().Value->print();
        return str;
    }
};
}