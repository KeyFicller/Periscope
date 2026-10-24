#pragma once

namespace periscope {
// ------------------------ Main template -----------------------

// Non-template bases hold the single enum definitions (source of truth).
// GP_type / GP_output_format templates re-export these via inheritance;
// graph_type / graph_output_format are aliases — not second enumerator lists.
// io() and internal switches use the aliases; call sites prefer graph >> GP_type<>::k_* (>> ≡ set).

struct GP_type_base
{
    // Named chart_type (not `type`) so it does not clash with base_property::type
    // when GP_type inherits this base.
    enum chart_type : unsigned int
    {
        k_flowchart = 0,
        k_sequence = 1,
        k_gantt = 2,
    };
};
using graph_type = GP_type_base::chart_type;

struct GP_output_format_base
{
    enum output_format : unsigned int
    {
        k_mermaid = 0,
        k_markdown = 1,
    };
};
using graph_output_format = GP_output_format_base::output_format;

// graph_io_context is context for graph io (graph-universal state only)
struct graph_io_context
{
    graph_type GraphType;
};

// io is to get graph io context
inline graph_io_context&
io()
{
    static graph_io_context _instance;
    return _instance;
}
} // namespace periscope
