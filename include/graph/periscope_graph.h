#pragma once

#include "graph/periscope_graph_properties.h"
#include "link/periscope_link.h"
#include "node/periscope_node.h"
#include "object/periscope_handle.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include "type_hash/periscope_type_hash.h"
#include <map>
#include <memory>
#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------

template<typename underlying_type>
class graph : public object<graph<underlying_type>>
{
  public:
    graph()
    {
        // default type is flowchart
        this->template set<GP_type<underlying_type>>(GP_type<underlying_type>::k_flowchart);
        this->template set<OP_printable>(true);
    }

  public:
    std::string to_string_impl() const
    {
        std::string str = "";
        // Header info
        switch (this->template get<GP_type<underlying_type>>().Value) {
            case GP_type<underlying_type>::k_flowchart:
                str += "flowchart";
                str += "\n";
                break;
            default:
                throw std::runtime_error("Unsupported graph type");
        }

        // Draw nodes
        for_each_object<node>([&str](node& node) {
            str += node.to_string();
            str += "\n";
        });

        // Draw links
        for_each_object<link>([&str](link& link) {
            str += link.to_string();
            str += "\n";
        });

        return str;
    }

    template<typename t>
    void for_each_object(const std::function<void(t&)>& callback) const
    {
        for (auto& [_handle, ptr] : m_elements) {
            if (ptr->template get<OP_type>().Value == static_hash<t>()) {
                callback(*static_cast<t*>(ptr.get()));
            }
        }
    }

  public:
    template<typename t>
    t& new_object()
    {
        static_assert(std::is_base_of_v<base_object, t>, "T must be a derived class of base_object");
        auto _handle = m_handle_manager.allocate();
        auto ptr = std::make_unique<t>();
        t& ref = *ptr;
        m_elements[_handle] = std::move(ptr);
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.template set<OP_type>(static_hash<t>());
        return ref;
    }

    template<typename t>
    t& new_object_at(const handle<underlying_type>& _handle)
    {
        if (m_handle_manager.is_allocated(_handle))
            throw std::runtime_error("Handle already allocated");
        auto ptr = std::make_unique<t>();
        t& ref = *ptr;
        m_elements[_handle] = std::move(ptr);
        m_handle_manager.allocate_at(_handle);
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.template set<OP_type>(static_hash<t>());
        return ref;
    }

    void delete_object(const handle<underlying_type>& _handle)
    {
        if (!m_handle_manager.is_allocated(_handle))
            throw std::runtime_error("Handle not allocated");
        m_elements.erase(_handle);
        m_handle_manager.deallocate(_handle);
    }

  private:
    handle_manager<underlying_type> m_handle_manager;
    std::map<handle<underlying_type>, std::unique_ptr<base_object>> m_elements;
};

}