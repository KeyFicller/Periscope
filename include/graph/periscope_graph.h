#pragma once

#include "graph/periscope_graph_fwd.h"
#include "graph/periscope_graph_properties.h"
#include "link/periscope_link.h"
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

template<typename underlying_type>
class graph : public object<graph<underlying_type>>
{
  public:
    graph()
    {
        // default type is flowchart
        this->template set<GP_type<underlying_type>>(graph_type::k_flowchart);
        this->template set<OP_printable>(true);
    }

    std::string to_string() const
    {
        return object<graph<underlying_type>>::to_string(this->template get<GP_type<underlying_type>>().Value);
    }

  public:
    std::string to_string_impl(graph_type graph_type) const
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

        // Header info
        switch (this->template get<GP_type<underlying_type>>().Value) {
            case graph_type::k_flowchart:
                str += "flowchart\n";
                break;
            case graph_type::k_sequence:
                str += "sequenceDiagram\n";
                break;
            default:
                throw std::runtime_error("Unsupported graph type");
        }

        // Draw nodes
        for_each_object<node>(
          [&str, graph_type](node& node) { str += std::format("{}\n", node.to_string(graph_type)); });

        // Draw links
        for_each_object<link>(
          [&str, graph_type](link& link) { str += std::format("{}\n", link.to_string(graph_type)); });

        return str;
    }

    template<typename t>
    void for_each_object(const std::function<void(t&)>& callback) const
    {
        for (auto& ptr : m_elements) {
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
        m_elements.push_back(std::move(ptr));
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
        m_elements.push_back(std::move(ptr));
        m_handle_manager.allocate_at(_handle);
        ref.m_handle = std::make_shared<handle<underlying_type>>(_handle.id());
        ref.template set<OP_type>(static_hash<t>());
        return ref;
    }

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