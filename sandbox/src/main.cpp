#include "graph/periscope_graph.h"
#include "graph/periscope_graph_properties.h"
#include "node/periscope_node.h"
#include "node/periscope_node_properties.h"
#include "object/periscope_object.h"
#include "object/periscope_object_properties.h"

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
    _graph.set<GP_type<unsigned int>>(GP_type<unsigned int>::k_flowchart);
    std::cout << _graph.to_string() << std::endl;
    return 0;
}