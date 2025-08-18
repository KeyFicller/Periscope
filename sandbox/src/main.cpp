#include <iostream>
#include "periscope.h"


int main()
{
    int index_sample = 1;
    auto& graph = PSCP_FLOW_CHART(index_sample);
    using user_node = periscope::node < PSCP_KEY_T(index_sample)>;
    using user_link = periscope::binary_link<PSCP_KEY_T(index_sample)>;

    auto& E353 = graph.add_node<user_node>().set_note("E353");
    auto& F96 = graph.add_node<user_node>().set_note("F96");
    auto& F93 = graph.add_node<user_node>().set_note("F93");

    auto& L1 = graph.add_link<user_link>(E353, F96).set_note("Adjacent");
    auto& L2 = graph.add_link<user_link>(E353, F93).set_note("Adjacent");

    std::cout << periscope::format_printer::print("[0]", PSCP_FLOW_CHART(index_sample)) << std::endl;

    //int index_sample = 1;
    //auto& graph = PSCP_SEQUENCE(index_sample);
    //using user_node = periscope::node < PSCP_KEY_T(index_sample)>;
    //using user_link = periscope::binary_link<PSCP_KEY_T(index_sample)>;

    //auto& E353 = graph.add_node<user_node>().set_note("E353");
    //auto& F96 = graph.add_node<user_node>().set_note("F96");
    //auto& F93 = graph.add_node<user_node>().set_note("F93");

    //auto& L1 = graph.add_link<user_link>(E353, F96).set_note("Adjacent");
    //auto& L2 = graph.add_link<user_link>(E353, F93).set_note("Adjacent");
    //auto& L3 = graph.add_link<user_link>(F93, E353).set_note("Adjacent");
    //auto& L4 = graph.add_link<user_link>(F96, E353).set_note("Adjacent");

    //PSCP_CTX().gs_export_type = periscope::export_type::k_mark_down;
    //std::cout << periscope::format_printer::print("[0]", graph) << std::endl;

    //std::cout << periscope::format_printer::print("[0]", periscope::prompt_id::k_missing_placeholder, 3);

    //std::cout << periscope::format_printer::print("[0][1]", periscope::prompt_id::k_invalid_graph_element, 1, 2, periscope::prompt_id::k_invalid_graph_element, 3, 4);

    //int index_sample = 1;
    //auto& graph = PSCP_GANTT(index_sample);
    //using user_node = periscope::span_node<PSCP_KEY_T(index_sample)>;

    //auto& sn1 = graph.add_node<user_node>(periscope::date_tick("09:00"), periscope::date_tick("12:00"));
    //sn1.set_note("task1");
    //auto& sn2 = graph.add_node<user_node>(sn1, periscope::date_duration("3h"));
    //sn2.set_note("task2");
    //graph.add_node<user_node>(sn2, periscope::date_tick("17:00")).set_note("task3");
    //PSCP_CTX().gs_export_type = periscope::export_type::k_mark_down;
    //std::cout << periscope::format_printer::print("[0]", graph) << std::endl;
}