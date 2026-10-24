#pragma once

#include "misc/periscope_class_def_properties.h"
#include "object/periscope_object.h"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace periscope {

// Forward-declared in periscope_handle.h (default underlying_type = int).
template<typename underlying_type>
class graph;

// class_def is a class definition object.
// Create spelling: `class_def& c = class_def(graph) >> props…` — same create-intermediary
// shape as node (graph& ctor → object_builder<class_def, …> → new_object<class_def>).
// new_object<class_def> still works.
class class_def : public object<class_def>
{
  public:
    class_def() { set<OP_printable>(true); }

    // Create-intermediary entry (defined after graph<> is complete in periscope_object_builder.h).
    template<typename underlying_type>
    explicit class_def(graph<underlying_type>& _graph) noexcept;

    class_def(const class_def&) = delete;
    class_def& operator=(const class_def&) = delete;

    class_def(class_def&& other) noexcept
      : object<class_def>(static_cast<object<class_def>&&>(other))
      , m_create_graph(std::exchange(other.m_create_graph, nullptr))
      , m_create_fn(std::exchange(other.m_create_fn, nullptr))
    {
    }

    class_def& operator=(class_def&& other) noexcept
    {
        if (this != &other) {
            static_cast<object<class_def>&>(*this) = static_cast<object<class_def>&&>(other);
            m_create_graph = std::exchange(other.m_create_graph, nullptr);
            m_create_fn = std::exchange(other.m_create_fn, nullptr);
        }
        return *this;
    }

    ~class_def()
    {
        if (m_create_fn == nullptr)
            return;
        std::fputs("periscope: incomplete class_def(graph) — discarded without materializing to class_def& "
                   "(write `class_def& c = class_def(g) >> props`, not `class_def(g) >> props`)\n",
                   stderr);
        std::abort();
    }

    bool is_create_seed() const noexcept { return m_create_fn != nullptr; }

    struct create_seed_state
    {
        void* Graph = nullptr;
        class_def& (*CreateFn)(void*) = nullptr;
    };

    create_seed_state extract_create() noexcept
    {
        return { std::exchange(m_create_graph, nullptr), std::exchange(m_create_fn, nullptr) };
    }

    // to_string_impl is implementation of object::to_string_impl
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        if (_graph_type == graph_type::k_flowchart) {
            if (!has<OP_name>()) {
                throw std::runtime_error("class_def: name is required");
            }
            std::string props = _V_str<MCD_fill>() + _V_str<MCD_stroke>() + _V_str<MCD_color>();
            if (props.back() == ',') {
                props.pop_back();
            }
            return "classDef " + _V_str<OP_name>() + "Class" + " " + props;
        }
        return "";
    }

  private:
    void* m_create_graph = nullptr;
    class_def& (*m_create_fn)(void*) = nullptr;
};

} // namespace periscope
