#pragma once

#include "periscope_printer.h"

namespace periscope {
// ------------------------ Main template -----------------------
template<typename t>
class handle_manager;

template<typename t>
class object_base
{
    friend class handle_manager<t>;

  public:
    object_base() = default;
    object_base(const object_base& _other) = delete;
    object_base& operator=(const object_base& _other) = delete;
    virtual ~object_base() = default;

    const t& handle() const { return m_handle; }

  protected:
    void set_handle(const t& _handle) { m_handle = _handle; }

  public:
    virtual std::string to_string() const = 0;

    void set_insert_order(int _order) { m_insert_order = _order; }

    int insert_order() const { return m_insert_order; }

  protected:
    t m_handle;
    int m_insert_order = -1;
};

template<typename t, typename derived>
class object : public object_base<t>
{
  public:
    std::string to_string() const override { return static_cast<const derived*>(this)->to_string_impl(); }

    std::string to_string_impl() const
    {
        // Can't use note right of 1 ...
        return "\"" + printer::print(this->m_handle) + "\"";
    }
};

// ---------------------- Specialization(this) ------------------

// --------------------- Specialization(other) ------------------
}
