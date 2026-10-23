#pragma once

#include "link/periscope_link.h"
#include "link/periscope_link_properties.h"
#include "node/periscope_node.h"
#include "type_traits/periscope_type_list.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

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
// (literals prefer "Yes"_op_name; no parallel property_literal factory; no bare string / const char*)
struct label
{
    std::string Value;
};

namespace detail {
// Normalize >> decorations into values link::set can deduce (enums / property_literal).
// label{} becomes property_literal<OP_name>; other link decorations pass through.
template<typename t>
struct normalize_link_decoration
{
    using type = std::remove_cvref_t<t>;
    static type apply(t&& _value) { return std::forward<t>(_value); }
};

template<>
struct normalize_link_decoration<label>
{
    using type = property_literal<OP_name>;
    static type apply(label&& _label) { return property_literal<OP_name>{ std::move(_label.Value) }; }
    static type apply(const label& _label) { return property_literal<OP_name>{ _label.Value }; }
};
// Owning property key for a >> decoration (after or before normalize).
// Uniqueness is by property (OP_name / LP_style / LP_activate), not by value type:
// "Yes"_op_name and label{} both key as OP_name; two LP_style::* collide.
template<typename deco>
struct link_decoration_property;

template<typename prop>
struct link_decoration_property<property_literal<prop>>
{
    using type = prop;
};

template<typename e>
    requires property_enum<e>
struct link_decoration_property<e>
{
    using type = property_of_t<e>;
};

template<>
struct link_decoration_property<label>
{
    using type = OP_name;
};

template<typename deco>
using link_decoration_property_t = typename link_decoration_property<std::remove_cvref_t<deco>>::type;

} // namespace detail

// pending_link is a typed incomplete edge used only as a temporary in a one-shot `>>` chain
// (`a >> deco >> … >> b`). Decorations are a heterogeneous pack (no std::function erasure);
// each `>> deco` appends and changes the instantiation. Not a supported DX path to name and
// complete later — only rvalue overloads exist. Incomplete chains must not be discarded:
// [[nodiscard]] + consume-on-connect + destructor abort if still live.
template<typename... decorations>
struct [[nodiscard("incomplete >> chain: end with a target node (a >> props >> b)")]] pending_link
{
    node* Source = nullptr;
    std::tuple<decorations...> Decorations{};

    pending_link() = default;
    explicit pending_link(node* _source)
      : Source(_source)
    {
    }
    pending_link(node* _source, std::tuple<decorations...> _decorations)
      : Source(_source)
      , Decorations(std::move(_decorations))
    {
    }

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
        std::fputs("periscope: incomplete >> chain — pending_link discarded without a target node "
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
// connect creates the link for a completed pending edge (typed decoration pack)
template<typename... decorations>
link_ref
connect(pending_link<decorations...> _pending, node& _target)
{
    // Release first so ~pending_link is inert even if create_link / set throws.
    node& source = *_pending.Source;
    auto decorations_pack = std::move(_pending.Decorations);
    _pending.release();
    if (source.owner() == nullptr || source.owner() != _target.owner())
        throw std::runtime_error("Cannot link nodes that do not belong to the same graph");
    link& created = source.owner()->create_link();
    created.set<LP_source>(source).set<LP_target>(_target);
    std::apply([&created](auto&&... _deco) { (created.apply_pack_arg(std::forward<decltype(_deco)>(_deco)), ...); },
               std::move(decorations_pack));
    return link_ref(created, _target);
}

inline pending_link<>
start(node& _source)
{
    return pending_link<>{ &_source };
}

// Properties applicable to a link: link-owned props, or base_object props shared by all objects (OP_name)
template<typename prop>
concept link_applicable_property = requires {
    typename prop::owner_type;
} && (std::is_base_of_v<link, typename prop::owner_type> || std::is_same_v<base_object, typename prop::owner_type>);
} // namespace detail

// Nested property enums whose owning property applies to link (e.g. LP_style::style, LP_activate::activation)
template<typename e>
concept link_property_enum = property_enum<e> && detail::link_applicable_property<property_of_t<e>>;

// String UDLs whose owning property applies to link (e.g. "Yes"_op_name -> OP_name)
template<typename lit>
concept link_property_literal =
  property_literal_value<lit> && detail::link_applicable_property<typename std::remove_cvref_t<lit>::property>;

// link_decoration lists what may sit between two nodes in a `>>` chain.
// Bare std::string / const char* are intentionally excluded (ambiguous with other string link
// props): literals use "..."_op_name; runtime names use label{} or set<OP_name>.
// Edges are one-shot rvalue chains; a chain that stops on a decoration (no target) yields a
// discarded pending_link and fails: [[nodiscard]] at compile time, abort if forced through (void).
// Replace-strategy owning props appear at most once (see operator>> static_assert);
// append-strategy (vector) props may repeat and fold at connect — none on links yet.
template<typename t>
concept link_decoration =
  link_property_enum<t> || link_property_literal<t> || std::is_same_v<std::remove_cvref_t<t>, label>;

// ---- pending edge decorations (typed pack growth; any deduced link property / UDL / label{}) ----

template<typename... decorations, typename deco>
    requires link_decoration<deco>
auto
operator>>(pending_link<decorations...>&& _pending, deco _decoration)
{
    // Reject duplicate *replace-strategy* owning properties at the append site (vs. silent
    // overwrite at connect/set). Pack-level among replace keys only (same helper as
    // new_object). Scalar keys OP_name / LP_style / LP_activate — so "Yes"_op_name +
    // label{}, or two LP_style::*, fail here. Append-strategy keys would be allowed.
    static_assert(detail::owning_properties_unique_v<detail::link_decoration_property_t<decorations>...,
                                                     detail::link_decoration_property_t<deco>>,
                  "periscope: duplicate replace-strategy property in >> chain — each of "
                  "OP_name / LP_style / LP_activate may appear at most once (e.g. not two "
                  "styles, not \"Yes\"_op_name together with label{})");
    using normalized = typename detail::normalize_link_decoration<deco>::type;
    node* source = _pending.Source;
    auto pack = std::move(_pending.Decorations);
    _pending.release();
    return pending_link<decorations..., normalized>{
        source,
        std::tuple_cat(std::move(pack),
                       std::make_tuple(detail::normalize_link_decoration<deco>::apply(std::move(_decoration))))
    };
}

// ---- starting a pending edge from a node or from the end of a chain ----

template<typename decoration>
    requires link_decoration<decoration>
auto
operator>>(node& _source, decoration _decoration)
{
    return detail::start(_source) >> std::move(_decoration);
}

template<typename decoration>
    requires link_decoration<decoration>
auto
operator>>(const link_ref& _chain, decoration _decoration)
{
    return detail::start(_chain.target()) >> std::move(_decoration);
}

// ---- completing an edge ----

template<typename... decorations>
link_ref
operator>>(pending_link<decorations...>&& _pending, node& _target)
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
