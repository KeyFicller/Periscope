#pragma once

// Included from the end of periscope_graph.h after class graph is complete.
// Do not include this header directly — pull periscope.h / periscope_graph.h.

#include "misc/periscope_class_def.h"
#include "misc/periscope_note.h"
#include "node/periscope_node.h"
#include "object/periscope_object_properties.h"
#include "type_traits/periscope_type_list.h"
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <utility>

namespace periscope {
namespace detail {
// Properties applicable to Object: Object-owned props, or base_object props (OP_name).
// Same rule new_object<Object> uses via set() owner_type checks.
template<typename Object, typename prop>
concept object_applicable_property = requires {
    typename prop::owner_type;
} && (std::is_base_of_v<Object, typename prop::owner_type> || std::is_same_v<base_object, typename prop::owner_type>);
} // namespace detail

// Nested property enums / literals whose owning property applies to Object.
template<typename Object, typename e>
concept object_property_enum = property_enum<e> && detail::object_applicable_property<Object, property_of_t<e>>;

template<typename Object, typename lit>
concept object_property_literal =
  property_literal_value<lit> &&
  detail::object_applicable_property<Object, typename std::remove_cvref_t<lit>::property>;

// What may sit in an `Object(graph) >> …` builder chain (same deduced forms as new_object).
// Bare std::string / const char* / label{} are excluded (label{} is edge-only).
template<typename Object, typename t>
concept object_decoration = object_property_enum<Object, t> || object_property_literal<Object, t>;

// object_builder is a typed incomplete create used only as a temporary in a one-shot
// `Object(graph) >> props…` expression. Distinct from pending_link / edge `>>`.
// Materializes via new_object<Object> + apply_pack_arg — either by converting to Object&
// (`Object& o = Object(g) >> props`) or automatically in the destructor when the finished
// chain is discarded as a statement (`Object(g) >> props;`, like unused new_object).
//
// Shared by node / note / class_def. link is intentionally excluded — edge `>>` already
// owns that spelling; keep link create as new_object<link>.
// Bare Object(graph) with no `>>` still aborts in ~create_seed (incomplete seed).
template<typename Object, typename... props>
struct object_builder
{
    void* Graph = nullptr;
    Object& (*CreateFn)(void*) = nullptr;
    std::tuple<props...> Props{};

    object_builder() = default;
    object_builder(void* _graph, Object& (*_create_fn)(void*), std::tuple<props...> _props = {})
      : Graph(_graph)
      , CreateFn(_create_fn)
      , Props(std::move(_props))
    {
    }

    object_builder(object_builder&& other) noexcept
      : Graph(std::exchange(other.Graph, nullptr))
      , CreateFn(std::exchange(other.CreateFn, nullptr))
      , Props(std::move(other.Props))
    {
    }
    object_builder& operator=(object_builder&& other) noexcept
    {
        if (this != &other) {
            Graph = std::exchange(other.Graph, nullptr);
            CreateFn = std::exchange(other.CreateFn, nullptr);
            Props = std::move(other.Props);
        }
        return *this;
    }
    object_builder(const object_builder&) = delete;
    object_builder& operator=(const object_builder&) = delete;

    // Materialize into the owning graph (same pack rules as new_object<Object>).
    // Rvalue-only so a named builder cannot be converted twice.
    operator Object&() && { return materialize(); }

    // Finished chain discarded as a statement: auto-materialize (same as unused new_object).
    ~object_builder()
    {
        if (Graph == nullptr)
            return;
        (void)materialize();
    }

    void release() noexcept
    {
        Graph = nullptr;
        CreateFn = nullptr;
    }

  private:
    // Shared by conversion and destructor — Graph/CreateFn cleared before create so a
    // re-entrant / double path cannot materialize twice.
    Object& materialize()
    {
        void* g = std::exchange(Graph, nullptr);
        auto* create = std::exchange(CreateFn, nullptr);
        auto pack = std::move(Props);
        Object& obj = create(g);
        std::apply([&obj](auto&&... _p) { (obj.apply_pack_arg(std::forward<decltype(_p)>(_p)), ...); },
                   std::move(pack));
        return obj;
    }
};

// Convenience aliases (node was the first fluent-create type).
template<typename... props>
using node_builder = object_builder<node, props...>;
template<typename... props>
using note_builder = object_builder<note, props...>;
template<typename... props>
using class_def_builder = object_builder<class_def, props...>;

// ---- growing an object_builder pack (typed; any deduced Object property / UDL) ----

template<typename Object, typename... props, typename deco>
    requires object_decoration<Object, deco>
auto
operator>>(object_builder<Object, props...>&& _builder, deco _decoration)
{
    // Reject duplicate *replace-strategy* owning properties at the append site (vs. silent
    // overwrite at set). Same helper as new_object / edge >>. Append / bitwise_or may repeat.
    static_assert(detail::deduced_args_unique_v<props..., deco>,
                  "periscope: duplicate replace-strategy owning property in Object(graph) >> chain");
    using normalized = std::remove_cvref_t<deco>;
    void* g = _builder.Graph;
    auto* create = _builder.CreateFn;
    auto pack = std::move(_builder.Props);
    _builder.release();
    return object_builder<Object, props..., normalized>{
        g, create, std::tuple_cat(std::move(pack), std::make_tuple(std::move(_decoration)))
    };
}

// ---- Object(graph&) create-intermediary (constructor entry; type name names the factory) ----

namespace detail {
template<typename Object>
concept has_create_seed = requires(Object& o) {
    { o.is_create_seed() } -> std::convertible_to<bool>;
    { o.extract_create() };
};
} // namespace detail

// Thin typed trampolines — create-seed fields / move / dtor live in create_seed<Object>.
template<typename underlying_type>
node::node(graph<underlying_type>& _graph) noexcept
  : create_seed<node>(
      &_graph,
      +[](void* p) -> node& { return static_cast<graph<underlying_type>*>(p)->template new_object<node>(); })
{
    // Hollow temporary — not OP_printable, not inserted; first `>>` starts object_builder.
}

template<typename underlying_type>
note::note(graph<underlying_type>& _graph) noexcept
  : create_seed<note>(
      &_graph,
      +[](void* p) -> note& { return static_cast<graph<underlying_type>*>(p)->template new_object<note>(); })
{
}

template<typename underlying_type>
class_def::class_def(graph<underlying_type>& _graph) noexcept
  : create_seed<class_def>(
      &_graph,
      +[](void* p) -> class_def& { return static_cast<graph<underlying_type>*>(p)->template new_object<class_def>(); })
{
}

// Rvalue Object >> deco starts the typed builder (lvalue node& >> deco remains edge start).
// Constrained to create_seed<Object> types (node / note / class_def). link has no create seed —
// edge `>>` stays the only link spelling; create links with new_object<link>.
template<typename Object, typename deco>
    requires detail::has_create_seed<Object> && object_decoration<Object, deco>
auto
operator>>(Object&& _obj, deco _decoration)
{
    if (!_obj.is_create_seed()) {
        std::fputs("periscope: Object&& >> prop — not a create-intermediary (use Object(graph) >> props)\n", stderr);
        std::abort();
    }
    auto seed = _obj.extract_create();
    using normalized = std::remove_cvref_t<deco>;
    static_assert(detail::deduced_args_unique_v<deco>,
                  "periscope: duplicate replace-strategy owning property in Object(graph) >> chain");
    return object_builder<Object, normalized>{ seed.Graph, seed.CreateFn, std::make_tuple(std::move(_decoration)) };
}
} // namespace periscope
