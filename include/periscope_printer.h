#pragma once

#include <ios>
#include <sstream>
#include <string>
#include <type_traits>

namespace periscope
{
    // ------------------------ Main template -----------------------
    struct printer
    {
        template <typename t>
        std::string operator() (const t& _value) const
        {
            if constexpr (std::is_same_v<t, std::string> || std::is_same_v<t, const char*>)
                return _value;
            else if constexpr (std::is_arithmetic_v<t>)
                return std::to_string(_value);
            else if constexpr (std::is_pointer_v<t>)
            {
                std::ostringstream oss;
                oss << "0x" << std::uppercase << std::hex << reinterpret_cast<uintptr_t>(_value);
                return oss.str();
            }
            else if constexpr (requires {_value.to_string(); })
            {
                return _value.to_string();
            }

            return "{Undefined print behavior}";
        }

        template <unsigned n>
        std::string operator()(const char(&_value)[n]) const
        {
            return _value;
        }
    };

    struct format_printer
    {
        template <typename ...args>
        std::string operator() (const std::string& _format, args&&... _args)
        {
            std::string result = _format;
            format_printer_impl<0>(result, std::forward<args&&>(_args)...);
            return result;
        }

        template <unsigned n>
        static void format_printer_impl(std::string& _buffer)
        {
            return;
        }

        template <unsigned n, typename arg, typename ...args>
        static void format_printer_impl(std::string& _buffer, arg&& _arg, args&&... _args)
        {
            const std::string from = "[" + std::to_string(n) + "]";
            const std::string to = printer()(_arg);
            size_t start_pos = 0;
            while ((start_pos = _buffer.find(from, start_pos)) != std::string::npos)
            {
                _buffer.replace(start_pos, from.size(), to);
                start_pos += to.length();
            }

            format_printer_impl<n + 1>(_buffer, std::forward<args&&>(_args)...);
        }
    };

    // ---------------------- Specialization(this) ------------------



    // --------------------- Specialization(other) ------------------
}
