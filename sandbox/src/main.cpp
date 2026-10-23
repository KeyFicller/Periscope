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

    // Create class definitions (name via variadic new_object; color props still need set<MCD_*>)
    class_def& start_class =
      _graph.new_object<class_def>("Start"_op_name).set<MCD_fill>("#4A90E2").set<MCD_stroke>("#2E5C8A").set<MCD_color>(
        "#fff");

    class_def& process_class = _graph.new_object<class_def>("Process"_op_name)
                                 .set<MCD_fill>("#F5A623")
                                 .set<MCD_stroke>("#C97E1C")
                                 .set<MCD_color>("#fff");

    class_def& decision_class = _graph.new_object<class_def>("Decision"_op_name)
                                  .set<MCD_fill>("#7ED321")
                                  .set<MCD_stroke>("#5FA319")
                                  .set<MCD_color>("#fff");

    class_def& end_class =
      _graph.new_object<class_def>("End"_op_name).set<MCD_fill>("#D0021B").set<MCD_stroke>("#A00115").set<MCD_color>(
        "#fff");

    (void)start_class;
    (void)process_class;
    (void)decision_class;
    (void)end_class;

    // Create nodes: one call packs name / shape / class (UDLs + nested enums from #5)
    node& start = _graph.new_object<node>("Start"_op_name, NP_shape::k_rectangle, "Start"_np_class_def);
    node& process = _graph.new_object<node>("Process Data"_op_name, NP_shape::k_rectangle, "Process"_np_class_def);
    node& decision = _graph.new_object<node>("Decision"_op_name, NP_shape::k_diamond, "Decision"_np_class_def);
    node& end = _graph.new_object<node>("End"_op_name, NP_shape::k_rectangle, "End"_np_class_def);

    // Link props in the `>>` world — two equivalent patterns:
    // 1) In-chain decoration: property enums / UDLs between nodes (preferred for labels)
    start >> process >> decision >> "Yes"_op_name >> end;
    // 2) Trailing set on the link_ref returned by `>>`
    (decision >> "No"_op_name >> process).set(LP_style::k_dashed | LP_style::k_arrow_mask);

    // Configure graph: nested enums via set(enum), same pattern as NP_shape / GP_date_format
    _graph.set(GP_type<>::k_flowchart)
      .set(GP_output_format<>::k_markdown)
      .set(GP_flowchart_direction<>::k_top_to_down);

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

    void send_request(Service* target, const std::string& msg)
    {
        auto& hook_graph = get_hook_graph();
        static std::map<const void*, bool> registered;

        handle<const void*> this_handle(reinterpret_cast<const void*>(this));
        handle<const void*> target_handle(reinterpret_cast<const void*>(target));

        if (!registered[this]) {
            hook_graph.new_object_at<node>(this_handle).set<OP_name>(name);
            registered[this] = true;
        }
        if (!registered[target]) {
            hook_graph.new_object_at<node>(target_handle).set<OP_name>(target->name);
            registered[target] = true;
        }

        // A sequence message is the same `>>` edge; the label becomes the message text
        hook_graph.access<node>(this_handle) >> label{ msg } >> hook_graph.access<node>(target_handle);
    }

    static void print_graph()
    {
        auto& hook_graph = get_hook_graph();
        hook_graph.set(GP_type<>::k_sequence)
          .set(GP_output_format<>::k_markdown)
          .template set<GP_sequence_show_number>(true);
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

    // Business logic triggers hooks
    client.send_request(&server, "Request Data");
    server.send_request(&database, "Query");
    database.send_request(&server, "Result");
    server.send_request(&client, "Response");

    // Output sequence diagram
    Service::print_graph();
}

void
example3_gantt()
{
    graph _graph;
    _graph.set(GP_type<>::k_gantt)
      .set(GP_output_format<>::k_markdown)
      .set(GP_date_format<>::k_hour_minute);

    // Three tasks; name in new_object, ticks still need set<NP_time_tick_*>
    node& design = _graph.new_object<node>("Design"_op_name)
                     .set<NP_time_tick_start>("9:0"_tick)
                     .set<NP_time_tick_end>("12:0"_tick);

    node& build = _graph.new_object<node>("Build"_op_name)
                    .set<NP_time_tick_start>("12:0"_tick)
                    .set<NP_time_tick_end>("15:30"_tick);

    node& test = _graph.new_object<node>("Test"_op_name)
                   .set<NP_time_tick_start>("15:30"_tick)
                   .set<NP_time_tick_end>("17:0"_tick);

    (void)design;
    (void)build;
    (void)test;

    std::cout << "=== Example 3: Gantt Chart ===\n";
    std::cout << _graph.to_string() << std::endl;
}

int
main()
{
    example1_flowchart();
    std::cout << "\n";
    example2_sequence();
    std::cout << "\n";
    example3_gantt();
    return 0;
}
