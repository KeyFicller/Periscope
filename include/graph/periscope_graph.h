#pragma once

#include "graph/periscope_graph_fwd.h"
#include "graph/periscope_graph_properties.h"
#include "link/periscope_link.h"
#include "misc/periscope_note.h"
#include "node/periscope_node.h"
#include "object/periscope_handle.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include "tools/periscope_guard.h"
#include "type_hash/periscope_type_hash.h"
#include <format>
#include <map>
#include <memory>
#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------

// graph is container for nodes and links with graph type management
template<typename underlying_type>
class graph : public object<graph<underlying_type>>
{
  public:
    graph()
    {
        // default type is flowchart
        this->template set<GP_type>(graph_type::k_flowchart);
        this->template set<OP_printable>(true);
    }

    // to_string is to convert graph to string representation
    std::string to_string() const
    {
        if (!this->template has<GP_type<underlying_type>>())
            throw std::runtime_error("Graph type not set, please set it before calling to_string");
        io().GraphType = this->template get<GP_type<underlying_type>>().Value;
        return object<graph<underlying_type>>::to_string();
    }

    // Bring base class set into scope for non-template properties
    using object<graph<underlying_type>>::set;

    // set is to set property value with type checking
    template<template<typename> class prop>
    graph& set(const typename prop<underlying_type>::type& _value)
    {
        static_assert(std::is_base_of_v<typename prop<underlying_type>::owner_type, graph<underlying_type>>,
                      "Owner must be a derived class of prop::owner_type");
        this->template get_or_create<prop<underlying_type>>().Value = _value;
        return *this;
    }

  public:
    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        std::string str = "";

        graph_output_format output_format = graph_output_format::k_markdown;
        if (this->template has<GP_output_format<underlying_type>>())
            output_format = this->template get<GP_output_format<underlying_type>>().Value;

        switch (output_format) {
            case graph_output_format::k_markdown:
                str += "```mermaid\n";
                break;
            default:
                break;
        }

        guard _guard([&str, output_format]() {
            switch (output_format) {
                case graph_output_format::k_markdown:
                    str += "```\n";
                    break;
                default:
                    break;
            }
        });

        // Graph info
        str += this->template _V_str<GP_type<underlying_type>>();
        str += " ";
        str += this->template _V_str<GP_flowchart_direction<underlying_type>>();
        str += "\n";

        // Configuration info
        str += this->template _V_str<GP_sequence_show_number<underlying_type>>();
        str += "\n";

        // Draw nodes
        traverser<node>::traverse(*this, [&str](node& node) { str += std::format("{}\n", node.to_string()); });

        // Draw links and notes
        traverser<type_list<link, note>>::traverse(
          *this, [&str](base_object& object) { str += std::format("{}\n", object.to_string()); });

        return str;
    }

    template<typename t>
    struct traverser
    {
        static void traverse(const graph<underlying_type>& _graph, const std::function<void(t&)>& _callback)
        {
            for (auto& ptr : _graph.m_elements) {
                if (ptr->template get<OP_type>().Value == static_hash<t>()) {
                    _callback(*static_cast<t*>(ptr.get()));
                }
            }
        }
    };

    template<typename... ts>
    struct traverser<type_list<ts...>>
    {
        static void traverse(const graph<underlying_type>& _graph, const std::function<void(base_object&)>& _callback)
        {
            for (auto& ptr : _graph.m_elements) {
                if (((ptr->template get<OP_type>().Value == static_hash<ts>()) || ...)) {
                    _callback(*static_cast<base_object*>(ptr.get()));
                }
            }
        }
    };

  public:
    // new_object is to create a new object with auto-allocated handle
    template<typename t>
    t& new_object()
    {
        static_assert(std::is_base_of_v<base_object, t>, "T must be a derived class of base_object");
        auto _handle = m_handle_manager.allocate();
        auto ptr = std::make_unique<t>();
        t& ref = *ptr;
        m_elements.push_back(std::move(ptr));
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.template set<OP_type>(static_hash<t>());
        return ref;
    }

    // access is to access a object by handle
    template<typename t>
    t& access(const handle<underlying_type>& _handle)
    {
        for (auto& ptr : m_elements) {
            if (ptr->template get<OP_type>().Value == static_hash<t>()) {
                if (static_cast<handle<underlying_type>*>(ptr->get_handle().get())->id() == _handle.id()) {
                    return *static_cast<t*>(ptr.get());
                }
            }
        }
        throw std::runtime_error(std::format("Object not found: {}", _handle.id()));
    }

    // access is to access a object by handle
    template<typename t>
    const t& access(const handle<underlying_type>& _handle) const
    {
        return const_cast<graph<underlying_type>*>(this)->access<t>(_handle);
    }

    // new_object_at is to create a new object with specific handle
    template<typename t>
    t& new_object_at(const handle<underlying_type>& _handle)
    {
        if (m_handle_manager.is_allocated(_handle))
            throw std::runtime_error("Handle already allocated");
        auto ptr = std::make_unique<t>();
        t& ref = *ptr;
        m_elements.push_back(std::move(ptr));
        m_handle_manager.allocate_at(_handle);
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.template set<OP_type>(static_hash<t>());
        return ref;
    }

    // delete_object is to delete an object by handle
    void delete_object(const handle<underlying_type>& _handle)
    {
        if (!m_handle_manager.is_allocated(_handle))
            throw std::runtime_error("Handle not allocated");
        m_elements.erase(
          std::remove_if(m_elements.begin(),
                         m_elements.end(),
                         [&_handle](const std::unique_ptr<base_object>& ptr) {
                             return static_cast<handle<underlying_type>*>(ptr->get_handle().get())->id() ==
                                    _handle.id();
                         }),
          m_elements.end());
        m_handle_manager.deallocate(_handle);
    }

  private:
    handle_manager<underlying_type> m_handle_manager;
    std::vector<std::unique_ptr<base_object>> m_elements;
};
}