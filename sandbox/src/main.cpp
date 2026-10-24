#include "periscope.h"
#include <iostream>
#include <map>

using namespace periscope;

// Example 1: Flowchart with int as handle
void
example1_flowchart()
{
    graph _graph;
    _graph.set("Hello"_op_name);

    // Class defs + literal NP_class_def bind ("Name"_np_class_def).
    _graph.new_object<class_def>("Start"_op_name, "#4A90E2"_mcd_fill, "#2E5C8A"_mcd_stroke, "#fff"_mcd_color);
    _graph.new_object<class_def>("Process"_op_name, "#F5A623"_mcd_fill, "#C97E1C"_mcd_stroke, "#fff"_mcd_color);
    _graph.new_object<class_def>("Decision"_op_name, "#7ED321"_mcd_fill, "#5FA319"_mcd_stroke, "#fff"_mcd_color);
    _graph.new_object<class_def>("End"_op_name, "#D0021B"_mcd_fill, "#A00115"_mcd_stroke, "#fff"_mcd_color);

    // Create nodes: pack name / shape / "Name"_np_class_def. Duplicate replace-strategy props
    // are a compile error (two OP_name / two NP_shape). Vector-of-handle props: see example4.
    node& start = _graph.new_object<node>("Start"_op_name, NP_shape::k_rectangle, "Start"_np_class_def);
    node& process = _graph.new_object<node>("Process Data"_op_name, NP_shape::k_rectangle, "Process"_np_class_def);
    node& decision = _graph.new_object<node>("Decision"_op_name, NP_shape::k_diamond, "Decision"_np_class_def);
    node& end = _graph.new_object<node>("End"_op_name, NP_shape::k_rectangle, "End"_np_class_def);

    // Link props live in the `>>` chain as nested enums / UDLs (same deduction as set(enum)).
    // LP_style is bitwise_or: pieces may repeat and OR into one slot (same as composed *_arrow).
    start >> process >> decision >> "Yes"_op_name >> end;
    // Incomplete chain (no destination) is rejected: typed pending_link<...> is [[nodiscard]]
    // and its destructor aborts if never connected — e.g. the following would fail, not silently drop:
    //   decision >> "No"_op_name >> LP_style::k_dashed_arrow;
    // Duplicate replace-strategy props in one chain are a compile error, e.g.:
    //   decision >> "No"_op_name >> label{"x"} >> process;          // two OP_name
    // Piece-wise LP_style folds via bitwise_or (equiv. k_dashed_arrow):
    decision >> "No"_op_name >> LP_style::k_dashed >> LP_style::k_arrow_mask >> process;

    // Configure graph: nested enums via set(enum), same pattern as NP_shape / GP_date_format
    _graph.set(GP_type<>::k_flowchart).set(GP_output_format<>::k_markdown).set(GP_flowchart_direction<>::k_top_to_down);

    std::cout << "=== Example 1: Flowchart ===\n";
    std::cout << _graph.to_string() << std::endl;
}

// Example 2: Sequence diagram with pointer as handle (runtime hook simulation)
class Service
{
  public:
    std::string name;
    Service(const std::string& n)
      : name(n)
    {
    }

    static graph<const void*>& get_hook_graph()
    {
        static graph<const void*> hook_graph;
        return hook_graph;
    }

    void send_request(Service* target, const std::string& msg, bool activate_target = false)
    {
        auto& hook_graph = get_hook_graph();
        static std::map<const void*, bool> registered;

        handle<const void*> this_handle(reinterpret_cast<const void*>(this));
        handle<const void*> target_handle(reinterpret_cast<const void*>(target));

        // new_object_at accepts the same deduced property pack as new_object; runtime
        // strings still use explicit set<OP_name> (no parallel property_literal factory).
        if (!registered[this]) {
            hook_graph.new_object_at<node>(this_handle).set<OP_name>(name);
            registered[this] = true;
        }
        if (!registered[target]) {
            hook_graph.new_object_at<node>(target_handle).set<OP_name>(target->name);
            registered[target] = true;
        }

        // Runtime message via explicit label{}; literals prefer "..."_op_name.
        // Decorations grow a typed pending_link<...> pack (no std::function erasure).
        // Edges are one-shot: `src >> props >> dst` in a single expression (no named/deferred edge).
        auto& src = hook_graph.access<node>(this_handle);
        auto& dst = hook_graph.access<node>(target_handle);
        if (activate_target)
            src >> LP_activate::k_activate >> label{ msg } >> dst;
        else
            src >> label{ msg } >> dst;
    }

    static void print_graph()
    {
        auto& hook_graph = get_hook_graph();
        hook_graph.set(GP_type<>::k_sequence).set(GP_output_format<>::k_markdown).set(GP_sequence_show_number<>::k_on);
        std::cout << "=== Example 2: Sequence Diagram ===\n";
        std::cout << hook_graph.to_string() << std::endl;
    }
};

void
example2_sequence()
{
    Service client("Client");
    Service server("Server");
    Service database("Database");

    // Business logic triggers hooks (activate server / database on the request path)
    client.send_request(&server, "Request Data", true);
    server.send_request(&database, "Query", true);
    database.send_request(&server, "Result");
    server.send_request(&client, "Response");

    // Output sequence diagram
    Service::print_graph();
}

void
example3_gantt()
{
    graph _graph;
    _graph.set(GP_type<>::k_gantt).set(GP_output_format<>::k_markdown).set(GP_date_format<>::k_hour_minute);

    // Three tasks: pack name + ticks via deduced UDLs (runtime vars: explicit set<NP_time_tick_*> / set<OP_name>)
    _graph.new_object<node>("Design"_op_name, "9:0"_np_time_tick_start, "12:0"_np_time_tick_end);
    _graph.new_object<node>("Build"_op_name, "12:0"_np_time_tick_start, "15:30"_np_time_tick_end);
    _graph.new_object<node>("Test"_op_name, "15:30"_np_time_tick_start, "17:0"_np_time_tick_end);

    std::cout << "=== Example 3: Gantt Chart ===\n";
    std::cout << _graph.to_string() << std::endl;
}

// Example 4: subgraph + sequence notes — property_literal<Prop>{ node_a, node_b }
// packs handles (no .get_handle()). NP_class_def via "Name"_np_class_def literal.
void
example4_subgraph_and_notes()
{
    graph _graph;
    _graph.set(GP_type<>::k_flowchart).set(GP_output_format<>::k_markdown).set(GP_flowchart_direction<>::k_top_to_down);

    _graph.new_object<class_def>("GroupStyle"_op_name, "#4A90E2"_mcd_fill, "#2E5C8A"_mcd_stroke, "#fff"_mcd_color);

    node& a = _graph.new_object<node>("A"_op_name, NP_shape::k_rectangle);
    node& b = _graph.new_object<node>("B"_op_name, NP_shape::k_rectangle);
    node& c = _graph.new_object<node>("C"_op_name, NP_shape::k_rectangle);

    // One NP_subgraph_node chunk from node refs; class style via "Name"_np_class_def.
    node& group = _graph.new_object<node>(
      "Group"_op_name, property_literal<NP_subgraph_node>{ a, b, c }, "GroupStyle"_np_class_def);

    std::cout << "=== Example 4: subgraph + classDef literal ===\n";
    std::cout << "subgraph member count (expect 3): " << group.get<NP_subgraph_node>().Value.size() << "\n";
    std::cout << _graph.to_string() << "\n";

    // Sequence note spanning participants via MNP_basis handle pack
    graph seq;
    seq.set(GP_type<>::k_sequence).set(GP_output_format<>::k_markdown);
    node& client = seq.new_object<node>("Client"_op_name);
    node& server = seq.new_object<node>("Server"_op_name);
    seq.new_object<note>("shared context"_op_name, property_literal<MNP_basis>{ client, server });
    // Single-participant note with orient
    seq.new_object<note>("on client"_op_name, property_literal<MNP_basis>{ client }, MNP_orient::k_right);

    std::cout << "=== Example 4b: sequence notes ===\n";
    std::cout << seq.to_string() << std::endl;
}

int
main()
{
    example1_flowchart();
    std::cout << "\n";
    example2_sequence();
    std::cout << "\n";
    example3_gantt();
    std::cout << "\n";
    example4_subgraph_and_notes();
    return 0;
}
