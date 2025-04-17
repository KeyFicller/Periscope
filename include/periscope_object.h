#pragma once

#include "periscope_printer.h"

namespace periscope
{
    // ------------------------ Main template -----------------------
    template <typename t>
    class object
    {
        friend class handle_manager;
    protected:
        void set_handle(const t& _handle)
        {
            m_handle = _handle;
        }

        const t& handle() const
        {
            return m_handle;
        }

    protected:
        t m_handle;
    };

    // ---------------------- Specialization(this) ------------------



    // --------------------- Specialization(other) ------------------
}
