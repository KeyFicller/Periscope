#pragma once

#include "periscope_define.h"
#include "periscope_graph_config.h"
#include "periscope_handle.h"
#include "periscope_key.h"
#include "periscope_link.h"
#include "periscope_node.h"
#include "periscope_type.h"

namespace periscope {
// ------------------------ Main template -----------------------

template<typename t, graph_type gt>
struct available_elements_traits
{
    static_assert(false, "Invalid graph type");
};

template<typename t, graph_type gt>
struct available_elements
{
    template<typename elem>
    static constexpr bool contains()
    {
        return type_list_contains_derived_v<typename available_elements_traits<t, gt>::types, elem>;
    }
};

template<typename t>
class graph_base
{
  public:
    handle_manager<t>& handle_manager() { return graph_base<t>::m_handle_manager; }

  protected:
    periscope::handle_manager<t> m_handle_manager;
};

template<typename t, graph_type gt>
class graph : public graph_base<t>
{
  public:
    static graph& instance()
    {
        static graph gs_graph;
        return gs_graph;
    }

    graph_config<gt>& config() { return m_config; }

    template<typename obj, typename... args>
    obj& add_node(args&&... _args)
        requires std::is_base_of_v<node<t>, obj>
    {
        static_assert(available_elements<t, gt>::template contains<obj>(), "Invalid graph element");
        auto& result = graph_base<t>::m_handle_manager.template create<obj, args&&...>(std::forward<args>(_args)...);
        m_node_handles.push_back(result.handle());
        result.set_insert_order(static_cast<int>(m_node_handles.size()));
        return result;
    }

    template<typename obj, typename... args>
    obj& add_node_at(const t& _handle, args&&... _args)
        requires std::is_base_of_v<node<t>, obj>
    {
        static_assert(available_elements<t, gt>::template contains<obj>(), "Invalid graph element");
        auto& result =
          graph_base<t>::m_handle_manager.template create_at<obj, args&&...>(_handle, std::forward<args>(_args)...);
        m_node_handles.push_back(_handle);
        result.set_insert_order(static_cast<int>(m_node_handles.size()));
        return result;
    }

    template<typename obj, typename... args>
    obj& add_link(args&&... _args)
        requires std::is_base_of_v<link_base<t>, obj>
    {
        static_assert(available_elements<t, gt>::template contains<obj>(), "Invalid graph element");
        auto& result = graph_base<t>::m_handle_manager.template create<obj, args&&...>(std::forward<args>(_args)...);
        result.set_handle_manager(&graph_base<t>::handle_manager());
        m_link_handles.push_back(result.handle());
        result.set_insert_order(static_cast<int>(m_link_handles.size()));
        return result;
    }

    template<typename obj, typename... args>
    obj& add_link_at(const t& _handle, args&&... _args)
        requires std::is_base_of_v<link_base<t>, obj>
    {
        static_assert(available_elements<t, gt>::template contains<obj>(), "Invalid graph element");
        auto& result =
          graph_base<t>::m_handle_manager.template create_at<obj, args&&...>(_handle, std::forward<args>(_args)...);
        result.set_handle_manager(&graph_base<t>::handle_manager());
        m_link_handles.push_back(_handle);
        result.set_insert_order(static_cast<int>(m_link_handles.size()));
        return result;
    }

    std::string to_string() const
    {
        PSCP_CTX().gs_graph_type = gt;
        std::string result;

        {
            scoped_export guard(result, PSCP_CTX().gs_export_type);

            result += printer::print(gt);
            result += "\n";
            result += m_config.set_up();
            result += "\n";

            // result += "Nodes: \n";
            for (const auto& handle : m_node_handles) {
                result += printer::print(graph_base<t>::m_handle_manager.access(handle));
                result += "\n";
            }

            // result += "Links: \n";
            for (const auto& handle : m_link_handles) {
                result += printer::print(graph_base<t>::m_handle_manager.access(handle));
                result += "\n";
            }
        }

        PSCP_CTX().gs_graph_type = graph_type::k_careless;

        return result;
    }

    node<t>& access_node(const t& _handle) { return graph_base<t>::m_handle_manager.template access<node<t>>(_handle); }

  protected:
    graph() = default;
    virtual ~graph() = default;

  protected:
    std::vector<t> m_node_handles;
    std::vector<t> m_link_handles;
    graph_config<gt> m_config;
};

// ---------------------- Specialization(this) ------------------

template<typename t>
struct available_elements_traits<t, graph_type::k_flow_chart>
{
    using types = type_list<node<t>, link_base<t>>;
};

template<typename t>
struct available_elements_traits<t, graph_type::k_sequence>
{
    using types = type_list<node<t>, link_base<t>>;
};

template<typename t>
struct available_elements_traits<t, graph_type::k_gantt>
{
    using types = type_list<span_node<t>, link_base<t>>;
};

// --------------------- Specialization(other) ------------------
namespace printer {
template<>
std::string
print<graph_type>(const graph_type& _value)
{

    switch (_value) {
        case graph_type::k_flow_chart:
            return "flowchart";
        case graph_type::k_sequence:
            return "sequenceDiagram";
        case graph_type::k_gantt:
            return "gantt";
        case graph_type::k_careless:
        case graph_type::k_class_diagram:
        default:
            PSCP_NOT_IMPLEMENT();
            return "flowchart";
    }
}

}

}

#define PSCP_FLOW_CHART(X)                                                                                             \
    ::periscope::graph<periscope::key_type<decltype(X)>::type, ::periscope::graph_type::k_flow_chart>::instance()
#define PSCP_SEQUENCE(X)                                                                                               \
    ::periscope::graph<periscope::key_type<decltype(X)>::type, ::periscope::graph_type::k_sequence>::instance()
#define PSCP_GANTT(X)                                                                                                  \
    ::periscope::graph<periscope::key_type<decltype(X)>::type, ::periscope::graph_type::k_gantt>::instance()