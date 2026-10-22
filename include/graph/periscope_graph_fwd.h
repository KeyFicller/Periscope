#pragma once

namespace periscope {
// ------------------------ Main template -----------------------

// graph_type is enum for graph types
enum class graph_type
{
    k_flowchart,
    k_sequence,
    k_gantt
};

// graph_output_format is enum for graph output formats
enum class graph_output_format
{
    k_mermaid,
    k_markdown,
};

// gantt_date_format is Mermaid gantt dateFormat; shared by GP_date_format and tick rendering
enum class gantt_date_format
{
    k_month_day,   // MM-DD
    k_hour_minute, // HH:mm
};

// graph_io_context is context for graph io (active while to_string runs)
struct graph_io_context
{
    graph_type GraphType{};
    // Active gantt dateFormat so NP_time_tick can match Mermaid zero-padded output
    gantt_date_format DateFormat = gantt_date_format::k_hour_minute;
};

// io is to get graph io context
inline graph_io_context&
io()
{
    static graph_io_context _instance;
    return _instance;
}
} // namespace periscope
