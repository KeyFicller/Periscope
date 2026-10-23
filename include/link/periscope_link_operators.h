#pragma once

#include "link/periscope_link.h"
#include "link/periscope_link_properties.h"
#include "node/periscope_node.h"
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace periscope {
// ------------------------ Main template -----------------------

// link_owner is the non-template interface a graph exposes so node operators can create links in it
class link_owner
{
  public:
    virtual link& create_link() = 0;

  protected:
    ~link_owner() = default;
};

// label is a tag carrying a link label, e.g. `a >> label{"Yes"} >> b` (prefer "Yes"_op_name)
struct label
{
    std::string Value;
};

// activate is a tag for sequence activation, e.g. `a >> activate{true} >> b`
struct activate
{
    bool Value = true;
};

// pending_link is an edge whose source is known and whose target is still missing.
// Decorations are type-erased setters applied to the created link (any deduced link property).
struct pending_link
{
    node* Source = nullptr;
    std::vector<std::function<void(link&)>> Decorations;
};

// link_ref is the result of `a >> b`: the created link plus its target, so chains continue from the target
class link_ref
{
  public:
    link_ref(link& _link, node& _target)
      : m_link(&_link)
      , m_target(&_target)
    {
    }

    // get is access to the created link
    link& get() const { return *m_link; }
    operator link&() const { return *m_link; }

    // target is the last node of the chain
    node& target() const { return *m_target; }

    // set forwards to link::set so a created edge can still be configured
    template<typename prop, typename value>
    link_ref& set(const value& _value)
    {
        m_link->template set<prop>(_value);
        return *this;
    }

    template<typename e>
        requires property_enum<e>
    link_ref& set(e _value)
    {
        m_link->set(_value);
        return *this;
    }

    template<typename lit>
        requires property_literal_value<lit>
    link_ref& set(lit _literal)
    {
        m_link->set(std::move(_literal));
        return *this;
    }

  private:
    link* m_link;
    node* m_target;
};

namespace detail {
// connect creates the link for a completed pending edge
inline link_ref
connect(const pending_link& _pending, node& _target)
{
    node& source = *_pending.Source;
    if (source.owner() == nullptr || source.owner() != _target.owner())
        throw std::runtime_error("Cannot link nodes that do not belong to the same graph");
    link& created = source.owner()->create_link();
    created.set<LP_source>(source).set<LP_target>(_target);
    for (const auto& apply : _pending.Decorations)
        apply(created);
    return link_ref(created, _target);
}

inline pending_link
start(node& _source)
{
    pending_link pending;
    pending.Source = &_source;
    return pending;
}

// Properties applicable to a link: link-owned props, or base_object props shared by all objects (OP_name)
template<typename prop>
concept link_applicable_property = requires { typename prop::owner_type; } &&
                                   (std::is_base_of_v<link, typename prop::owner_type> ||
                                    std::is_same_v<base_object, typename prop::owner_type>);
} // namespace detail

// Nested property enums whose owning property applies to link (e.g. LP_style::style)
template<typename e>
concept link_property_enum = property_enum<e> && detail::link_applicable_property<property_of_t<e>>;

// String UDLs whose owning property applies to link (e.g. "Yes"_op_name -> OP_name)
template<typename lit>
concept link_property_literal =
  property_literal_value<lit> && detail::link_applicable_property<typename std::remove_cvref_t<lit>::property>;

// link_decoration lists what may sit between two nodes in a `>>` chain
template<typename t>
concept link_decoration =
  link_property_enum<t> || link_property_literal<t> || std::is_same_v<std::remove_cvref_t<t>, label> ||
  std::is_same_v<std::remove_cvref_t<t>, activate> || std::is_convertible_v<t, const char*>;

// ---- pending edge decorations (any deduced link property / UDL / activate) ----

template<typename e>
    requires link_property_enum<e>
pending_link
operator>>(pending_link _pending, e _value)
{
    _pending.Decorations.push_back([_value](link& _link) { _link.set(_value); });
    return _pending;
}

template<typename lit>
    requires link_property_literal<lit>
pending_link
operator>>(pending_link _pending, lit _literal)
{
    _pending.Decorations.push_back([_literal = std::move(_literal)](link& _link) mutable { _link.set(std::move(_literal)); });
    return _pending;
}

inline pending_link
operator>>(pending_link _pending, label _label)
{
    return std::move(_pending) >> property_literal<OP_name>{ std::move(_label.Value) };
}

inline pending_link
operator>>(pending_link _pending, const char* _label)
{
    // Raw C-string sugar for OP_name; preferred form is "Yes"_op_name
    return std::move(_pending) >> property_literal<OP_name>{ _label };
}

inline pending_link
operator>>(pending_link _pending, activate _activate)
{
    _pending.Decorations.push_back([_activate](link& _link) { _link.template set<LP_activate>(_activate.Value); });
    return _pending;
}

// ---- starting a pending edge from a node or from the end of a chain ----

template<typename decoration>
    requires link_decoration<decoration>
pending_link
operator>>(node& _source, decoration _decoration)
{
    return detail::start(_source) >> std::move(_decoration);
}

template<typename decoration>
    requires link_decoration<decoration>
pending_link
operator>>(const link_ref& _chain, decoration _decoration)
{
    return detail::start(_chain.target()) >> std::move(_decoration);
}

// ---- completing an edge ----

inline link_ref
operator>>(pending_link _pending, node& _target)
{
    return detail::connect(_pending, _target);
}

inline link_ref
operator>>(node& _source, node& _target)
{
    return detail::connect(detail::start(_source), _target);
}

inline link_ref
operator>>(const link_ref& _chain, node& _target)
{
    return detail::connect(detail::start(_chain.target()), _target);
}
} // namespace periscope
