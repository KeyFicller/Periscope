#pragma once

#include "periscope_object.h"

#include <map>

namespace periscope {
// ------------------------ Main template -----------------------

enum class node_shape
{
    k_rectangle,
    k_rounded,
    k_stadium,
    k_sub_process,
    k_cylinder,
    k_circle,
    k_lean_l,
    k_lean_r
};

template<typename t>
class node : public object<t>
{
  public:
    std::string to_string_impl() const override
    {
        std::string base = object<t>::to_string_impl();
        switch (PSCP_CTX().gs_graph_type) {
            case graph_type::k_flow_chart:
                return format_printer::print("[0]@{ shape: [1], label: \"[2]\" }", base, m_shape, m_note);
            case graph_type::k_sequence:
                return format_printer::print("participant [0] as [1]", base, m_note);
            case graph_type::k_careless:
            case graph_type::k_gantt:
            case graph_type::k_class_diagram:
            default:
                PSCP_NOT_IMPLEMENT();
                return {};
        }
    }

    std::string sub_graph_tag() const
    {
        std::string base = object<t>::to_string_impl();
        // static std::map<node_shape, std::vector<std::string>> brackets = {
        //     {node_shape::k_rectangle, {"[", "]"}},
        //     {node_shape::k_rounded, {"(", ")"}},
        // };
        // if (brackets.find(m_shape) != brackets.end())
        //     return format_printer::print("[0][1][2][3]", base, brackets[m_shape][0], m_note, brackets[m_shape][1]);
        return format_printer::print("[0][[1]]", base, m_note);
    }

    node& set_note(const std::string& _note)
    {
        m_note = _note;
        return *this;
    }

    const std::string& note() const { return m_note; }

  public:
    // style utils
    node& set_shape(node_shape _shape)
    {
        m_shape = _shape;
        return *this;
    }

    node_shape shape() const { return m_shape; }

  protected:
    std::string m_note;
    node_shape m_shape = node_shape::k_rectangle;
};

struct date_duration
{
    int hour;
    int minute;

    bool parse(const std::string& _buffer, int& _hour, int& _minute)
    {
        std::stringstream ss(_buffer);
        std::string str_hour;
        std::string str_minute;
        std::regex pattern(R"(^(?:(\d+)h\s*)?(?:(\d+)m)?$)");
        std::smatch matches;

        if (regex_match(_buffer, matches, pattern)) {
            str_hour = matches[1].str();
            str_minute = matches[2].str();
        } else {
            return false;
        }

        try {
            _hour = str_hour.empty() ? 0 : std::stoi(str_hour);
            _minute = str_minute.empty() ? 0 : std::stoi(str_minute);
        } catch (...) {
            return false;
        }

        if (!(0 <= _hour && _hour <= 23))
            return false;
        if (!(0 <= _minute && _minute <= 59))
            return false;

        return true;
    }

    std::string to_string() const
    {
        std::string str_hour;
        if (hour)
            str_hour = printer::print(hour) + "h";
        std::string str_minute;
        if (minute)
            str_minute = printer::print(minute) + "m";
        return str_hour + str_minute;
    }

    date_duration(const std::string& _buffer)
    {
        if (!parse(_buffer, hour, minute))
            throw std::invalid_argument(format_printer::print("[0]", prompt_id::k_invalid_date_tick, _buffer));
    }
};

struct date_tick
{
    int hour;
    int minute;

    bool parse(const std::string& _buffer, int& _hour, int& _minute)
    {
        std::stringstream ss(_buffer);
        std::string str_hour;
        std::string str_minute;
        if (!getline(ss, str_hour, ':'))
            return false;
        getline(ss, str_minute);
        if (str_hour.empty() || str_minute.empty())
            return false;

        try {
            _hour = std::stoi(str_hour);
            _minute = std::stoi(str_minute);
        } catch (...) {
            return false;
        }

        if (!(0 <= _hour && _hour <= 23))
            return false;
        if (!(0 <= _minute && _minute <= 59))
            return false;

        return true;
    }

    std::string to_string() const
    {
        auto str_hour = printer::print(hour);
        if (hour < 10)
            str_hour = "0" + str_hour;
        auto str_minute = printer::print(minute);
        if (minute < 10)
            str_minute = "0" + str_minute;
        return format_printer::print("[0]:[1]", str_hour, str_minute);
    }

    date_tick(const std::string& _buffer)
    {
        if (!parse(_buffer, hour, minute))
            throw std::invalid_argument(format_printer::print("[0]", prompt_id::k_invalid_date_tick, _buffer));
    }

    date_tick operator+(const date_duration& _dur) const
    {
        date_tick res = *this;
        res.hour += _dur.hour;
        res.minute += _dur.minute;
        if (_dur.minute >= 60) {
            res.minute %= 60;
            res.hour += 1;
        }

        return res;
    }

    bool operator<(const date_tick& _other) const
    {
        return hour < _other.hour || (hour == _other.hour && minute < _other.minute);
    }
};

template<typename t>
class span_node : public node<t>
{
  public:
    span_node(const date_tick& _start, const date_tick& _end)
      : m_start_tick(_start)
      , m_end_tick(_end)
    {
    }
    span_node(const date_tick& _start, const date_duration& _dur)
      : m_start_tick(_start)
      , m_end_tick(_start + _dur)
    {
    }
    span_node(const span_node& _last, const date_duration& _dur)
      : m_start_tick(_last.m_end_tick)
      , m_end_tick(_last.m_end_tick + _dur)
    {
    }
    span_node(const span_node& _last, const date_tick& _end)
      : m_start_tick(_last.m_end_tick)
      , m_end_tick(_end)
    {
        if (_end.operator<(_last.m_end_tick))
            throw std::invalid_argument(
              format_printer::print("[0]", prompt_id::k_discontinuous_dateline, _last.m_end_tick, _end));
    }

    std::string to_string_impl() const override
    {
        std::string base = object<t>::to_string_impl();
        switch (PSCP_CTX().gs_graph_type) {
            case graph_type::k_gantt:
                return format_printer::print("[0]: [1], [2], [3]", node<t>::m_note, base, m_start_tick, m_end_tick);
            case graph_type::k_careless:
            case graph_type::k_flow_chart:
            case graph_type::k_sequence:
            case graph_type::k_class_diagram:
            default:
                PSCP_NOT_IMPLEMENT();
                return {};
        }
    }

  protected:
    date_tick m_start_tick;
    date_tick m_end_tick;
};

// ---------------------- Specialization(this) ------------------

// --------------------- Specialization(other) ------------------

namespace printer {
template<>
std::string
print<node_shape>(const node_shape& _value)
{
    switch (_value) {
        case node_shape::k_rectangle:
            return "rect";
        case node_shape::k_rounded:
            return "rounded";
        case node_shape::k_stadium:
            return "stadium";
        case node_shape::k_sub_process:
            return "subproc";
        case node_shape::k_cylinder:
            return "cyl";
        case node_shape::k_circle:
            return "circle";
        case node_shape::k_lean_l:
            return "lean-l";
        case node_shape::k_lean_r:
            return "lean-r";
    }

    throw std::invalid_argument("{}");
    return "rect";
}

}

}
