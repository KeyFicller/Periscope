#pragma once

#include "link/periscope_link.h"
#include "link/periscope_link_properties.h"
#include "node/periscope_node.h"
#include <stdexcept>
#include <string>

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

// label is a tag carrying a link label, e.g. `a >> label("Yes") >> b`
struct label
{
    std::string Value;
};

// activate is a tag for sequence activation, e.g. `a >> activate(true) >> b`
struct activate
{
    bool Value = true;
};

// pending_link is an edge whose source is known and whose target is still missing
struct pending_link
{
    node* Source = nullptr;
    bool HasStyle = false;
    enum_type Style = 0;
    bool HasLabel = false;
    std::string Label;
    bool HasActivate = false;
    bool Activate = false;
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
    if (_pending.HasStyle)
        created.set<LP_style>(_pending.Style);
    if (_pending.HasLabel)
        created.set<OP_name>(_pending.Label);
    if (_pending.HasActivate)
        created.set<LP_activate>(_pending.Activate);
    return link_ref(created, _target);
}

inline pending_link
start(node& _source)
{
    pending_link pending;
    pending.Source = &_source;
    return pending;
}
} // namespace detail

// ---- pending edge decorations (style / label / activation) ----

inline pending_link
operator>>(pending_link _pending, LP_style::style _style)
{
    _pending.HasStyle = true;
    _pending.Style = _style;
    return _pending;
}

inline pending_link
operator>>(pending_link _pending, label _label)
{
    _pending.HasLabel = true;
    _pending.Label = std::move(_label.Value);
    return _pending;
}

inline pending_link
operator>>(pending_link _pending, const char* _label)
{
    return std::move(_pending) >> label{ _label };
}

inline pending_link
operator>>(pending_link _pending, activate _activate)
{
    _pending.HasActivate = true;
    _pending.Activate = _activate.Value;
    return _pending;
}

// ---- starting a pending edge from a node or from the end of a chain ----

template<typename decoration>
    requires requires(pending_link _p, decoration _d) { _p >> _d; }
pending_link
operator>>(node& _source, decoration _decoration)
{
    return detail::start(_source) >> std::move(_decoration);
}

template<typename decoration>
    requires requires(pending_link _p, decoration _d) { _p >> _d; }
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
