#pragma once

namespace periscope {
// ------------------------ Main template -----------------------

// graph_type is enum for graph types
enum class graph_type
{
    k_flowchart,
    k_sequence,
};

// graph_output_format is enum for graph output formats
enum class graph_output_format
{
    k_mermaid,
    k_markdown,
};

// graph_io_context is context for graph io
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