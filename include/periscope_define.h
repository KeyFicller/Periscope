#pragma once

#include <map>

namespace periscope {
// ------------------------ Main template -----------------------
enum class graph_type : uint8_t
{
    k_careless,
    k_flow_chart,
    k_sequence,
    k_gantt,
    k_class_diagram
};

enum class export_type : uint8_t
{
    k_mermaid,
    k_mark_down,
    k_html
};

enum class prompt_id : uint8_t
{
    k_undefined_print,
    k_missing_placeholder,
    k_invalid_graph_element,
    k_invalid_handle,
    k_not_that_type,
    k_used_handle,
    k_invalid_date_tick,
    k_invalid_date_duration,
    k_discontinuous_dateline
};

inline const std::string&
prompt(const prompt_id& _id)
{
    static const std::map<prompt_id, std::string> prompts = {
        { prompt_id::k_undefined_print, "{Undefined print behavior.}" },
        { prompt_id::k_missing_placeholder, "{[0] is not found during replacing.}" },
        { prompt_id::k_invalid_graph_element, "{Can't insert [0] to [1].}" },
        { prompt_id::k_invalid_handle, "{Object handle [0] invalid.}" },
        { prompt_id::k_not_that_type, "{Object is not a [0] type.}" },
        { prompt_id::k_used_handle, "{Object handle [0] has been used.}" },
        { prompt_id::k_invalid_date_tick, "{Date format [0] can't be parsed, use HH::MM.}" },
        { prompt_id::k_invalid_date_duration, "{Date format [0] can't be parsed, use ?h?m.}" },
        { prompt_id::k_discontinuous_dateline, "{Date format [0] - [1] is discontinuous.}" },
    };
    return prompts.at(_id);
}

class scoped_export
{
  public:
    scoped_export(std::string& _str, export_type _type)
      : m_str(_str)
      , m_type(_type)
    {
        switch (m_type) {
            case export_type::k_mermaid: {
                break;
            }
            case export_type::k_mark_down: {
                m_str += "```mermaid\n";
                break;
            }
            case export_type::k_html: {
                break;
            }
        }
    }
    ~scoped_export()
    {
        switch (m_type) {
            case export_type::k_mermaid: {
                break;
            }
            case export_type::k_mark_down: {
                m_str += "```\n";
                break;
            }
            case export_type::k_html: {
                break;
            }
        }
    }
    scoped_export(const scoped_export& _other) = delete;
    scoped_export& operator=(const scoped_export& _other) = delete;

  private:
    std::string& m_str;
    export_type m_type = export_type::k_mermaid;
};

struct context
{
    export_type gs_export_type = export_type::k_mermaid;

    // be careful with embed logic.
    bool gs_embed_print_placeholder_enabled = true;
};

inline context&
gio()
{
    static context ctx;
    return ctx;
}

// ---------------------- Specialization(this) ------------------

// --------------------- Specialization(other) ------------------

}

#define PSCP_CTX() ::periscope::gio()
#ifdef _MSC_VER
#define PSCP_NOT_IMPLEMENT() __debugbreak()
#else
#define PSCP_NOT_IMPLEMENT()
#endif