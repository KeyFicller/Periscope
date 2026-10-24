#pragma once

#include "object/periscope_object.h"
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace periscope {

// create_seed is the CRTP base for fluent-create object types (node / note / class_def).
// Inheritance chain: Derived → create_seed<Derived> → object<Derived> → base_object.
// Holds the create-intermediary state (target graph + typed new_object trampoline), move,
// incomplete-seed destructor abort, and extract helpers used by object_builder.
//
// link intentionally does NOT inherit create_seed — edge `>>` already owns that spelling;
// keep link create as new_object<link>.
template<typename Derived>
class create_seed : public object<Derived>
{
  public:
    create_seed(const create_seed&) = delete;
    create_seed& operator=(const create_seed&) = delete;

    create_seed(create_seed&& other) noexcept
      : object<Derived>(static_cast<object<Derived>&&>(other))
      , m_create_graph(std::exchange(other.m_create_graph, nullptr))
      , m_create_fn(std::exchange(other.m_create_fn, nullptr))
    {
    }

    create_seed& operator=(create_seed&& other) noexcept
    {
        if (this != &other) {
            static_cast<object<Derived>&>(*this) = static_cast<object<Derived>&&>(other);
            m_create_graph = std::exchange(other.m_create_graph, nullptr);
            m_create_fn = std::exchange(other.m_create_fn, nullptr);
        }
        return *this;
    }

    // Bare Object(graph) with no `>>` props is incomplete — abort (not throw: safe if
    // destruction runs during another exception unwind). A finished `Object(g) >> props`
    // chain auto-materializes in ~object_builder (statement or assign both OK).
    ~create_seed()
    {
        if (m_create_fn == nullptr)
            return;
        std::fputs("periscope: incomplete Object(graph) — bare create-seed discarded without `>>` props "
                   "(write `Object(g) >> props…`, not bare `Object(g)`)\n",
                   stderr);
        std::abort();
    }

    bool is_create_seed() const noexcept { return m_create_fn != nullptr; }

    struct create_seed_state
    {
        void* Graph = nullptr;
        Derived& (*CreateFn)(void*) = nullptr;
    };

    create_seed_state extract_create() noexcept
    {
        return { std::exchange(m_create_graph, nullptr), std::exchange(m_create_fn, nullptr) };
    }

  protected:
    create_seed() = default;

    // Used by Derived(graph&) create-intermediary constructors (out-of-line in object_builder).
    create_seed(void* _graph, Derived& (*_create_fn)(void*)) noexcept
      : m_create_graph(_graph)
      , m_create_fn(_create_fn)
    {
    }

  private:
    // Create-intermediary only (null for graph-owned objects): target graph + typed trampoline.
    void* m_create_graph = nullptr;
    Derived& (*m_create_fn)(void*) = nullptr;
};

} // namespace periscope
