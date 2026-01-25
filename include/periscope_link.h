#pragma once

#include "periscope_object.h"
#include "periscope_printer.h"

#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------
enum class graph_type : uint8_t;
template<typename t, graph_type gt>
class graph;

template<typename t>
class handle_manager;

template<typename t>
class node;

enum class link_shape
{
    k_normal = 0x01,
    k_dash = 0x02,
    k_bold = 0x04
};

template<typename t>
class link_base : public object<t>
{
  public:
    link_base(const std::vector<t>& _nodes)
      : m_nodes(_nodes)
    {
    }
    ~link_base() override = default;

    // TODO: change to friend.
  public:
    void set_handle_manager(handle_manager<t>* _manager) { link_base<t>::m_handle_manager = _manager; }

    handle_manager<t>* handle_manager() const { return link_base<t>::m_handle_manager; }

  protected:
    std::vector<t> m_nodes;
    periscope::handle_manager<t>* m_handle_manager = nullptr;
};

template<typename t = const void*>
class unary_link : public link_base<t>
{
  public:
    unary_link(const t& _attached)
      : link_base<t>({ _attached })
    {
    }
    unary_link(const object<t>& _attached)
      : unary_link(_attached.handle())
    {
    }
    ~unary_link() override = default;

    const t& attacted() const { return link_base<t>::m_nodes.at(0); }

    unary_link& set_note(const std::string& _note)
    {
        m_note = _note;
        return *this;
    }

    std::string to_string_impl() const override
    {
        switch (PSCP_CTX().gs_graph_type) {
            case graph_type::k_sequence: {
                auto& attached_node = link_base<t>::handle_manager()->template access<node<t>>(attacted());
                return "note right of " + attached_node.object<t>::to_string_impl() + " :" + m_note;
            }
            case graph_type::k_flow_chart:
            case graph_type::k_careless:
            case graph_type::k_gantt:
            case graph_type::k_class_diagram:
            default: {
                PSCP_NOT_IMPLEMENT();
                return {};
            }
        }
    }

  protected:
    std::string m_note;
};

template<typename t = const void*>
class binary_link : public link_base<t>
{
  public:
    binary_link(const t& _from, const t& _to)
      : link_base<t>({ _from, _to })
    {
    }
    binary_link(const object<t>& _from, const object<t>& _to)
      : binary_link(_from.handle(), _to.handle())
    {
    }
    ~binary_link() override = default;

    std::string to_string_impl() const override
    {
        switch (PSCP_CTX().gs_graph_type) {
            case graph_type::k_flow_chart: {
                std::string arrow;
                if (m_note.empty()) {
                    arrow = printer::print(m_shape) + ">";
                } else {
                    auto lr = printer::print(m_shape);
                    auto rr = lr;
                    std::reverse(rr.begin(), rr.end());
                    arrow = format_printer::print("[0][1][2]", lr, m_note, rr) + ">";
                }
                return printer::print(from()) + arrow + printer::print(to());
            }
            case graph_type::k_sequence: {
                auto& from_node = link_base<t>::handle_manager()->template access<node<t>>(from());
                auto& to_node = link_base<t>::handle_manager()->template access<node<t>>(to());
                std::string live_mark;
                if (from_node.handle() != to_node.handle())
                    live_mark = from_node.insert_order() > to_node.insert_order()   ? "-"
                                : from_node.insert_order() < to_node.insert_order() ? "+"
                                                                                    : "";
                return from_node.object<t>::to_string_impl() + "->>" + live_mark + to_node.object<t>::to_string_impl() +
                       ": " + m_note;
            }
            case graph_type::k_careless:
            case graph_type::k_gantt:
            case graph_type::k_class_diagram:
            default: {
                PSCP_NOT_IMPLEMENT();
                return {};
            }
        }
    }

  public:
    const t& from() const { return link_base<t>::m_nodes.at(0); }
    const t& to() const { return link_base<t>::m_nodes.at(1); }

    binary_link& set_note(const std::string& _note)
    {
        m_note = _note;
        return *this;
    }

    const std::string& note() const { return m_note; }

    binary_link& set_shape(const link_shape& _shape)
    {
        m_shape = _shape;
        return *this;
    }

    link_shape shape() const { return m_shape; }

  protected:
    std::string m_note;
    link_shape m_shape = link_shape::k_normal;
};

template<typename t = const void*>
class hierarchy_link : public binary_link<t>
{
  public:
    hierarchy_link(const t& _ancestor, const t& _descendant)
      : binary_link<t>(_ancestor, _descendant)
    {
    }
    hierarchy_link(const object<t>& _ancestor, const object<t>& _descendant)
      : binary_link<t>(_ancestor.handle(), _descendant.handle())
    {
    }
    ~hierarchy_link() override = default;

  public:
    const t& ancestor() const { return binary_link<t>::from(); }
    const t& descendant() const { return binary_link<t>::to(); }

  public:
    std::string to_string_impl() const override
    {
        std::string result;
        result += "subgraph ";
        auto& ancestor_node = link_base<t>::handle_manager()->template access<node<t>>(ancestor());
        result += ancestor_node.sub_graph_tag();
        result += "\n";
        result += printer::print(descendant());
        result += "\n";
        result += "end\n";

        return result;
    }
};

// ---------------------- Specialization(this) ------------------

// --------------------- Specialization(other) ------------------
namespace printer {
template<>
std::string
print<link_shape>(const link_shape& _value)
{
    switch (_value) {
        case link_shape::k_normal:
            return "--";
        case link_shape::k_dash:
            return "-.";
        case link_shape::k_bold:
            return "==";
    }

    throw std::invalid_argument("{}");
    return "--";
}

}

}
