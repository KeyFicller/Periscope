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
}