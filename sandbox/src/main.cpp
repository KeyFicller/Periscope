#include "periscope.h"
#include "periscope_key.h"
#include "periscope_link.h"
#include "periscope_node.h"
#include <filesystem>
#include <fstream>
#include <iostream>

void
test_sequence_diagram()
{
    int index_sample = 1;
    auto& graph = PSCP_SEQUENCE(index_sample);
    graph.config().Show_sequenece_number = true;
    using user_node = periscope::node<PSCP_KEY_T(index_sample)>;
    using user_link = periscope::binary_link<PSCP_KEY_T(index_sample)>;
    using user_tag = periscope::unary_link<PSCP_KEY_T(index_sample)>;
    using anchor_node = periscope::anchor_node<PSCP_KEY_T(index_sample)>;

    // Create participants (nodes in sequence diagram)
    auto& client = graph.add_node<user_node>().set_note("Client");
    auto& server = graph.add_node<user_node>().set_note("Server");
    auto& database = graph.add_node<user_node>().set_note("Database");

    // Create interactions (messages between participants)
    // Client sends request to Server
    auto& req1 = graph.add_link<user_link>(client, server).set_note("Request Data");
    // Tag
    auto& tag1 = graph.add_link<user_tag>(client).set_note("Enuser access");
    // Server queries Database
    auto& req2 = graph.add_link<user_link>(server, database).set_note("Query");
    // Database inquring loop
    auto& anchor_s = graph.add_node<anchor_node>().set_note("Each item");
    auto& lop1 = graph.add_link<user_link>(database, database).set_note("Inquring");
    auto& anchor_e = graph.add_node<anchor_node>().set_is_anchor_end(true);
    // Database returns result to Server
    auto& resp1 = graph.add_link<user_link>(database, server).set_note("Result");
    // Server sends response to Client
    auto& resp2 = graph.add_link<user_link>(server, client).set_note("Response");

    // Set export type to markdown for better visualization
    PSCP_CTX().gs_export_type = periscope::export_type::k_mark_down;

    // Get the project root directory (assuming we're running from build/bin)
    // Go up two levels: build/bin -> build -> project root
    std::filesystem::path project_root = std::filesystem::current_path();
    if (project_root.filename() == "bin") {
        project_root = project_root.parent_path().parent_path();
    }

    // Create output directory if it doesn't exist
    std::filesystem::path output_dir = project_root / ".output";
    try {
        std::filesystem::create_directories(output_dir);
    } catch (const std::exception& e) {
        std::cerr << "Error creating output directory: " << e.what() << std::endl;
        return;
    }

    // Write the sequence diagram to file
    std::filesystem::path output_file = output_dir / "temp.md";
    std::ofstream outfile(output_file, std::ios::out | std::ios::trunc);
    if (outfile.is_open()) {
        outfile << periscope::format_printer::print("[0]", graph) << std::endl;
        outfile.close();
        std::cout << "Sequence diagram saved to " << std::filesystem::absolute(output_file) << std::endl;
    } else {
        std::cerr << "Error: Unable to open " << output_file << " for writing" << std::endl;
        std::cerr << "Current directory: " << std::filesystem::current_path() << std::endl;
    }
}

int
main()
{
    test_sequence_diagram();
    return 0;
}