#pragma once

#include "link/periscope_link.h"
#include "link/periscope_link_properties.h"
#include "node/periscope_node.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
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

// label is a tag carrying a runtime link label, e.g. `a >> label{msg} >> b`
// (literals prefer "Yes"_op_name; no parallel property_literal factory)
struct label
{
    std::string Value;
};

// pending_link is an edge whose source is known and whose target is still missing.
// Decorations are type-erased setters applied to the created link (any deduced link property).
// Incomplete chains (no destination node) must not be discarded: [[nodiscard]] + consume-on-connect.
struct [[nodiscard("incomplete >> chain: end with a target node (a >> props >> b)")]] pending_link
{
    node* Source = nullptr;
    std::vector<std::function<void(link&)>> Decorations;

    pending_link() = default;
    pending_link(pending_link&& other) noexcept
      : Source(std::exchange(other.Source, nullptr))
      , Decorations(std::move(other.Decorations))
    {
    }
    pending_link& operator=(pending_link&& other) noexcept
    {
        if (this != &other) {
            Source = std::exchange(other.Source, nullptr);
            Decorations = std::move(other.Decorations);
        }
        return *this;
    }
    pending_link(const pending_link&) = delete;
    pending_link& operator=(const pending_link&) = delete;

    // connect() releases before the edge is realized; a non-null Source here means the chain
    // never reached a target node (silent no-op would drop labels / styles).
    // Abort (not throw): safe if destruction runs during another exception unwind.
    ~pending_link()
    {
        if (Source == nullptr)
            return;
        std::fputs(
          "periscope: incomplete >> chain — pending_link discarded without a target node "
          "(write `a >> props >> b`, not `a >> props`)\n",
          stderr);
        std::abort();
    }

    void release() noexcept { Source = nullptr; }
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
connect(pending_link _pending, node& _target)
{
    // Release first so ~pending_link is inert even if create_link / set throws.
    node& source = *_pending.Source;
    auto decorations = std::move(_pending.Decorations);
    _pending.release();
    if (source.owner() == nullptr || source.owner() != _target.owner())
        throw std::runtime_error("Cannot link nodes that do not belong to the same graph");
    link& created = source.owner()->create_link();
    created.set<LP_source>(source).set<LP_target>(_target);
    for (const auto& apply : decorations)
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

// Nested property enums whose owning property applies to link (e.g. LP_style::style, LP_activate::activation)
template<typename e>
concept link_property_enum = property_enum<e> && detail::link_applicable_property<property_of_t<e>>;

// String UDLs whose owning property applies to link (e.g. "Yes"_op_name -> OP_name)
template<typename lit>
concept link_property_literal =
  property_literal_value<lit> && detail::link_applicable_property<typename std::remove_cvref_t<lit>::property>;

// link_decoration lists what may sit between two nodes in a `>>` chain.
// A chain that stops on a decoration (no target node) yields a discarded pending_link
// and fails: [[nodiscard]] at compile time, logic_error if forced through (void).
template<typename t>
concept link_decoration =
  link_property_enum<t> || link_property_literal<t> || std::is_same_v<std::remove_cvref_t<t>, label> ||
  std::is_convertible_v<t, const char*>;

// ---- pending edge decorations (any deduced link property / UDL) ----

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
    return detail::connect(std::move(_pending), _target); // releases before destroy
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
