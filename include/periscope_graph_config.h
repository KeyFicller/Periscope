#include "periscope_define.h"

namespace periscope {
// ------------------------ Main template -----------------------
template<graph_type gt>
struct graph_config
{};

// ---------------------- Specialization(this) ------------------
template<>
struct graph_config<graph_type::k_flow_chart>
{
    std::string set_up() const { return {}; }
};

template<>
struct graph_config<graph_type::k_sequence>
{
    bool Show_sequenece_number = false;

    std::string set_up() const
    {
        std::string config_str = "";
        if (Show_sequenece_number) {
            config_str += "autonumber";
        }
        return config_str;
    };
};

// --------------------- Specialization(other) ------------------
}