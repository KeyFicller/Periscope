#pragma once

#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"

namespace periscope {
class node : public object<node>
{
  public:
    node() { set<OP_printable>(true); }

  public:
    std::string to_string_impl() const { return m_handle->print() + " " + object<node>::to_string_impl(); }
};
}