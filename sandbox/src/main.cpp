#include "graph/periscope_graph.h"
#include "graph/periscope_graph_properties.h"
#include "node/periscope_node.h"
#include "node/periscope_node_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"

#include <fstream>
#include <iostream>

int
main()
{
    using namespace periscope;
    graph<unsigned int> _graph;
    node& _node = _graph.new_object<node>();
    _node.set<OP_name>("John Doe").set<NP_shape>(NP_shape::k_rectangle);
    node& _node2 = _graph.new_object<node>();
    _node2.set<OP_name>("Jane Doe").set<NP_shape>(NP_shape::k_rectangle);
    link& _link = _graph.new_object<link>();
    _link.set<LP_source>(_node.get_handle()).set<LP_target>(_node2.get_handle());
    link& _link2 = _graph.new_object<link>();
    _link2.set<LP_source>(_node2.get_handle()).set<LP_target>(_node.get_handle());
    _graph.template set<GP_type>(graph_type::k_sequence)
      .template set<GP_output_format>(graph_output_format::k_markdown)
      .template set<GP_display_node>(true);
    std::cout << _graph.to_string() << std::endl;

    std::ofstream fout(".output/temp.md");
    fout << _graph.to_string() << std::endl;
    fout.close();

    // _graph.remove<GP_type<unsigned int>>();
    // _graph.template set<GP_display_node<unsigned int>>(true);

    return 0;
}