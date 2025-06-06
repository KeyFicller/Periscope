#pragma once

#include "periscope_printer.h"

namespace periscope
{
    // ------------------------ Main template -----------------------
    template <typename t>
    class handle_manager;

    template <typename t>
    class object
    {
        friend class handle_manager<t>;
    public:
        object() = default;
        object(const object& _other) = delete;
        object& operator = (const object& _other) = delete;
        virtual ~object() = default;

        const t& handle() const
        {
            return m_handle;
        }

    protected:
        void set_handle(const t& _handle)
        {
            m_handle = _handle;
        }

    public:
        std::string to_string() const
        {
            return to_string_impl();
        }

        virtual std::string to_string_impl() const
        {
            return printer::print(m_handle);
        }

        void set_insert_order(int _order)
        {
            m_insert_order = _order;
        }

        int insert_order() const
        {
            return m_insert_order;
        }

    protected:
        t m_handle;
        int m_insert_order = -1;
    };

    // ---------------------- Specialization(this) ------------------



    // --------------------- Specialization(other) ------------------
}
