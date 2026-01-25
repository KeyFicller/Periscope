#pragma once

#include "periscope_object.h"
#include "periscope_printer.h"

#include <type_traits>
#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------
enum class graph_type : uint8_t;
template<typename t, graph_type gt>
class graph;

template<typename t>
class handle_manager;

template<typename t, typename derived>
class node;

template<typename t, typename derived>
class link_base;

enum class link_shape
{
    k_normal = 0x01,
    k_dash = 0x02,
    k_bold = 0x04
};

template<typename t, typename derived = void>
class link_base : public object<t, std::conditional_t<std::is_same_v<derived, void>, link_base<t>, derived>>
{
  public:
    using base_object = object<t, std::conditional_t<std::is_same_v<derived, void>, link_base<t>, derived>>;

    link_base(const std::vector<t>& _nodes)
      : m_nodes(_nodes)
    {
    }
    ~link_base() = default;

    std::string to_string() const override
    {
        if constexpr (std::is_same_v<derived, void>) {
            return to_string_impl();
        } else {
            return static_cast<const derived*>(this)->to_string_impl();
        }
    }

    std::string to_string_impl() const { return base_object::to_string_impl(); }

    // TODO: change to friend.
  public:
    void set_handle_manager(handle_manager<t>* _manager) { this->m_handle_manager = _manager; }

    handle_manager<t>* handle_manager() const { return this->m_handle_manager; }

  protected:
    std::vector<t> m_nodes;
    periscope::handle_manager<t>* m_handle_manager = nullptr;
};

template<typename t = const void*>
class unary_link : public link_base<t, unary_link<t>>
{
  public:
    using base_link = link_base<t, unary_link<t>>;

    unary_link(const t& _attached)
      : base_link({ _attached })
    {
    }
    unary_link(const object_base<t>& _attached)
      : unary_link(_attached.handle())
    {
    }
    ~unary_link() = default;

    const t& attacted() const { return base_link::m_nodes.at(0); }

    unary_link& set_note(const std::string& _note)
    {
        m_note = _note;
        return *this;
    }

    std::string to_string_impl() const
    {
        switch (PSCP_CTX().gs_graph_type) {
            case graph_type::k_sequence: {
                auto& attached_node = base_link::handle_manager()->template access<node<t, void>>(attacted());
                return "note right of " + attached_node.object<t, node<t, void>>::to_string_impl() + " :" + m_note;
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

template<typename t = const void*, typename derived = void>
class binary_link : public link_base<t, std::conditional_t<std::is_same_v<derived, void>, binary_link<t>, derived>>
{
  public:
    using base_link = link_base<t, std::conditional_t<std::is_same_v<derived, void>, binary_link<t>, derived>>;

    binary_link(const t& _from, const t& _to)
      : base_link({ _from, _to })
    {
    }
    binary_link(const object_base<t>& _from, const object_base<t>& _to)
      : binary_link(_from.handle(), _to.handle())
    {
    }
    ~binary_link() = default;

    std::string to_string() const override
    {
        if constexpr (std::is_same_v<derived, void>) {
            return to_string_impl();
        } else {
            return static_cast<const derived*>(this)->to_string_impl();
        }
    }

    std::string to_string_impl() const
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
                auto& from_node = base_link::handle_manager()->template access<node<t, void>>(from());
                auto& to_node = base_link::handle_manager()->template access<node<t, void>>(to());
                std::string live_mark;
                if (from_node.handle() != to_node.handle())
                    live_mark = from_node.insert_order() > to_node.insert_order()   ? "-"
                                : from_node.insert_order() < to_node.insert_order() ? "+"
                                                                                    : "";
                return from_node.object<t, node<t, void>>::to_string_impl() + "->>" + live_mark +
                       to_node.object<t, node<t, void>>::to_string_impl() + ": " + m_note;
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
    const t& from() const { return base_link::m_nodes.at(0); }
    const t& to() const { return base_link::m_nodes.at(1); }

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
class hierarchy_link : public binary_link<t, hierarchy_link<t>>
{
  public:
    using base_binary = binary_link<t, hierarchy_link<t>>;

    hierarchy_link(const t& _ancestor, const t& _descendant)
      : base_binary(_ancestor, _descendant)
    {
    }
    hierarchy_link(const object_base<t>& _ancestor, const object_base<t>& _descendant)
      : base_binary(_ancestor.handle(), _descendant.handle())
    {
    }
    ~hierarchy_link() = default;

  public:
    const t& ancestor() const { return base_binary::from(); }
    const t& descendant() const { return base_binary::to(); }

  public:
    std::string to_string_impl() const
    {
        std::string result;
        result += "subgraph ";
        auto& ancestor_node = base_binary::base_link::handle_manager()->template access<node<t, void>>(ancestor());
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
