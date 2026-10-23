#pragma once

namespace periscope {
// ------------------------ Main template -----------------------

// Freestanding graph_type / graph_output_format exist for io() and internal
// switches only. Numeric values MUST stay aligned with GP_type<>::chart_type and
// GP_output_format<>::output_format so set(GP_type<>::k_gantt) writes a value
// that compares equal after cast to graph_type. Prefer set(enum) with the
// nested enums at call sites; do not treat these freestanding enums as the
// property API.
enum class graph_type
{
    k_flowchart = 0,
    k_sequence = 1,
    k_gantt = 2
};

enum class graph_output_format
{
    k_mermaid = 0,
    k_markdown = 1,
};

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
