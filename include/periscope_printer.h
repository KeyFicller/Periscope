#pragma once

#include <ios>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>

#include "periscope_define.h"

#define MAXIMUM_BUFFER_SIZE 4096

namespace periscope
{
    enum class graph_type : uint8_t;

    // ------------------------ Main template -----------------------
    namespace printer
    {
        template <typename t>
        inline std::string print(const t& _value)
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

            return print(prompt_id::k_undefined_print);
        }

        template <unsigned n>
        inline std::string print(const char(&_value)[n])
        {
            return _value;
        }
    };

    namespace format_printer
    {
        template <typename ...args>
        inline std::string print (const std::string& _format, args&&... _args)
        {
            std::string result = _format;
            format_printer_impl<0>(result, std::forward<args&&>(_args)...);
            return result;
        }

        template <unsigned n>
        inline static void format_printer_impl(std::string& _buffer)
        {
            return;
        }

        inline static void offset_place_holder(std::string& _buffer, const int _offset, const std::set<int>& exclude_idx, int* sum)
        {
            std::regex pattern(R"(\[(\d+)\])");
            std::smatch match;
            std::string result;
            size_t last_pos = 0;

            auto begin = _buffer.cbegin();
            while (regex_search(begin, _buffer.cend(), match, pattern)) {
                result.append(match.prefix().first, match.prefix().second);

                int num = stoi(match[1]);
                if (sum)
                    *sum = std::max(*sum, num);
                if (exclude_idx.count(num))
                {
                    result += "[" + match[1].str() + "]";
                }
                else
                {
                    result += "[" + std::to_string(num + _offset) + "]";
                }
                begin = match.suffix().first;
            }

            result.append(begin, _buffer.cend());
            _buffer = result;
        }

        template <unsigned n, typename arg, typename ...args>
        inline static void format_printer_impl(std::string& _buffer, arg&& _arg, args&&... _args)
        {
            const std::string from = "[" + std::to_string(n) + "]";
            std::string to = printer::print(_arg);

            // Under test environment.
            if (PSCP_CTX().gs_embed_print_placeholder_enabled)
            {
                int sum = -1;
                offset_place_holder(to, n + 1, {}, &sum);
                offset_place_holder(_buffer, sum + 1, { n }, nullptr);
            }

            size_t start_pos = 0;
            bool replaced = false;
            while ((start_pos = _buffer.find(from, start_pos)) != std::string::npos)
            {
                _buffer.replace(start_pos, from.size(), to);
                replaced = true;
                start_pos += to.length();
            }

            if (!replaced)
            {
                auto error = format_printer::print("[0]", prompt_id::k_missing_placeholder, n);
                throw std::exception(error.c_str());
            }

            format_printer_impl<n + 1>(_buffer, std::forward<args&&>(_args)...);
        }

    };

    // ---------------------- Specialization(this) ------------------



    // --------------------- Specialization(other) ------------------
    namespace printer
    {
        template <>
        inline std::string print<prompt_id>(const prompt_id& _value)
        {
            return prompt(_value);
        }

    }
}
