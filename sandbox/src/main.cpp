#include "periscope.h"
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
    _node2.set<OP_name>("Jane Doe").set<NP_shape>(NP_shape::k_diamond);

    link& _link = _graph.new_object<link>();
    _link.set<LP_source>(_node.get_handle())
      .set<LP_target>(_node2.get_handle())
      .set<LP_style>(LP_style::k_dashed | LP_style::k_arrow_mask);

    note& _note = _graph.new_object<note>();
    _note.set<MNP_orient>(MNP_orient::k_right).set<MNP_basis>({ _node.get_handle() }).set<OP_name>("Test Note");

    link& _link2 = _graph.new_object<link>();
    _link2.set<LP_source>(_node2.get_handle()).set<LP_target>(_node.get_handle()).set<OP_name>("Link 2");
    _graph.template set<GP_type>(graph_type::k_sequence)
      .template set<GP_output_format>(graph_output_format::k_markdown)
      .template set<GP_sequence_show_number>(true);
    std::cout << _graph.to_string() << std::endl;

    std::ofstream fout(".output/temp.md");
    fout << _graph.to_string() << std::endl;
    fout.close();

    return 0;
}