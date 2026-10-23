#pragma once

#include "graph/periscope_graph_fwd.h"
#include "graph/periscope_graph_properties.h"
#include "link/periscope_link.h"
#include "link/periscope_link_operators.h"
#include "misc/periscope_class_def.h"
#include "misc/periscope_note.h"
#include "node/periscope_node.h"
#include "object/periscope_handle.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"
#include "tools/periscope_guard.h"
#include "type_hash/periscope_type_hash.h"
#include <format>
#include <memory>
#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------

// graph is container for nodes and links with graph type management
template<typename underlying_type>
class graph
  : public object<graph<underlying_type>>
  , public link_owner
{
  public:
    graph()
    {
        // default type is flowchart
        this->set(GP_type<underlying_type>::k_flowchart);
        this->template set<OP_printable>(true);
    }

    // to_string is to convert graph to string representation
    std::string to_string() const
    {
        if (!this->template has<GP_type<underlying_type>>())
            throw std::runtime_error("Graph type not set, please set it before calling to_string");
        io().GraphType = static_cast<graph_type>(this->template get<GP_type<underlying_type>>().Value);
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

    // set overload: deduce a graph property from its nested enum, e.g. set(GP_date_format<>::k_hour_minute)
    template<typename e>
        requires property_enum<e> && is_graph_property_tag<property_of_t<e>>::value
    graph& set(e _value)
    {
        return set_from_tag(property_of_t<e>{}, static_cast<enum_type>(_value));
    }

    // create_link is implementation of link_owner::create_link
    link& create_link() override { return new_object<link>(); }

  private:
    template<template<typename> class prop>
    graph& set_from_tag(graph_property_tag<prop>, enum_type _value)
    {
        return set<prop>(static_cast<typename prop<underlying_type>::type>(_value));
    }

  public:

  public:
    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        std::string str = "";

        graph_output_format output_format = graph_output_format::k_markdown;
        if (this->template has<GP_output_format<underlying_type>>())
            output_format = static_cast<graph_output_format>(
              this->template get<GP_output_format<underlying_type>>().Value);

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

        // Title
        if (this->template has<OP_name>()) {
            str += std::format("---\ntitle: {}\n---\n", this->template _V_str<OP_name>());
        }

        // Graph info
        str += this->template _V_str<GP_type<underlying_type>>();
        {
            std::string direction_str =
              this->template _V_str<GP_flowchart_direction<underlying_type>>();
            if (!direction_str.empty()) {
                str += " ";
                str += direction_str;
            }
        }
        str += "\n";

        // Configuration info
        switch (io().GraphType) {
            case graph_type::k_sequence:
                str += this->template _V_str<GP_sequence_show_number<underlying_type>>();
                str += "\n";
                break;
            case graph_type::k_gantt:
                str += std::format("    {}\n",
                                   this->template _V_str<GP_date_format<underlying_type>>());
                break;
            default:
                break;
        }

        // Forward declare class definitions
        traverser<class_def>::traverse(
          *this, [&str](class_def& class_def) { str += std::format("{}\n", class_def.to_string()); });

        // Draw nodes (gantt: format ticks from this graph's GP_date_format, not io())
        if (io().GraphType == graph_type::k_gantt) {
            enum_type date_fmt = GP_date_format<underlying_type>::date_format::k_hour_minute;
            if (this->template has<GP_date_format<underlying_type>>())
                date_fmt = this->template get<GP_date_format<underlying_type>>().Value;
            traverser<node>::traverse(*this, [&str, date_fmt](node& n) {
                std::string label_str = n.template get<OP_name>().Value;
                std::string start_tick_str;
                std::string end_tick_str;
                if (n.has<NP_time_tick_start>()) {
                    const auto& t = n.get<NP_time_tick_start>().Value;
                    start_tick_str = GP_date_format<underlying_type>::format_tick(t.first, t.second, date_fmt);
                }
                if (n.has<NP_time_tick_end>()) {
                    const auto& t = n.get<NP_time_tick_end>().Value;
                    end_tick_str = GP_date_format<underlying_type>::format_tick(t.first, t.second, date_fmt);
                }
                str += std::format("    {} : {}, {}\n", label_str, start_tick_str, end_tick_str);
            });
        } else {
            traverser<node>::traverse(*this, [&str](node& node) { str += std::format("{}\n", node.to_string()); });
        }

        // Draw links and notes
        traverser<type_list<link, note>>::traverse(
          *this, [&str](base_object& object) { str += std::format("{}\n", object.to_string()); });

        // Assign node names to class definitions
        traverser<node>::traverse(*this, [&str](node& node) {
            if (node.has<NP_class_def>()) {
                str +=
                  std::format("class {} {}Class\n", node.get_handle()->print(), node.template _V_str<NP_class_def>());
            }
        });
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
    // new_object creates an object and optionally applies deduced properties in one call,
    // e.g. new_object<node>("Start"_op_name, NP_shape::k_rectangle, "Start"_np_class_def).
    // Each owning property may appear at most once in the pack (same uniqueness as >>):
    // duplicate OP_name / NP_shape / … is a compile error — no silent overwrite via set().
    template<typename t, typename... args>
    t& new_object(args&&... _args)
    {
        static_assert(std::is_base_of_v<base_object, t>, "T must be a derived class of base_object");
        static_assert(detail::deduced_args_unique_v<args...>,
                      "periscope: duplicate owning property in new_object pack");
        auto _handle = m_handle_manager.allocate();
        auto ptr = std::make_unique<t>();
        t& ref = *ptr;
        m_elements.push_back(std::move(ptr));
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.m_owner = this;
        ref.template set<OP_type>(static_hash<t>());
        (ref.set(std::forward<args>(_args)), ...);
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

    // new_object_at: same deduced property pack as new_object, pinned to a handle
    // e.g. new_object_at<node>(h, "Client"_op_name, NP_shape::k_rectangle).
    // Pack uniqueness matches new_object (duplicate owning property is a compile error).
    template<typename t, typename... args>
    t& new_object_at(const handle<underlying_type>& _handle, args&&... _args)
    {
        static_assert(std::is_base_of_v<base_object, t>, "T must be a derived class of base_object");
        static_assert(detail::deduced_args_unique_v<args...>,
                      "periscope: duplicate owning property in new_object_at pack");
        if (m_handle_manager.is_allocated(_handle))
            throw std::runtime_error("Handle already allocated");
        auto ptr = std::make_unique<t>();
        t& ref = *ptr;
        m_elements.push_back(std::move(ptr));
        m_handle_manager.allocate_at(_handle);
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.m_owner = this;
        ref.template set<OP_type>(static_hash<t>());
        (ref.set(std::forward<args>(_args)), ...);
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