#pragma once

#include "periscope_object.h"

#include <set>
#include <stdexcept>
#include <unordered_map>

namespace periscope {
// ------------------------ Main template -----------------------
template<typename t>
struct handle_deduction
{};

template<typename t>
concept next_deduction_enabled = requires(const t& _handle) { handle_deduction<t>::next(_handle); };

template<typename t>
concept overall_deduction_enabled = requires(const std::set<t>& _handles) { handle_deduction<t>::next(_handles); };

template<typename t>
class handle_manager;

template<typename t>
class handle_allocator
{
    friend class handle_manager<t>;

  public:
    bool is_valid_handle(const t& _handle) const { return !m_used_handles.count(_handle); }

    void insert_handle(const t& _handle)
    {
        if (is_valid_handle(_handle))
            m_used_handles.insert(_handle);
    }

    [[nodiscard]] t next_handle(const t& _handle) const
        requires next_deduction_enabled<t>
    {
        return handle_deduction<t>::next(_handle);
    }

  protected:
    std::set<t> m_used_handles;
};

template<typename t>
class handle_manager
{
  public:
    template<typename obj = object_base<t>>
    obj& access(const t& _handle) const
    {
        if (!exists(_handle))
            throw std::runtime_error(format_printer::print("[0]", prompt_id::k_invalid_handle, _handle));
        obj* object = dynamic_cast<obj*>(m_map_objects.at(_handle).get());
        if (!object)
            throw std::runtime_error(format_printer::print("[0]", prompt_id::k_not_that_type, typeid(obj).name()));
        return *object;
    }

    template<typename obj, typename... args>
    obj& create(args&&... _args)
        requires overall_deduction_enabled<t> /* && std::is_base_of_v<object_base<t>, obj>*/
    {
        t handle = handle_deduction<t>::next(m_allocator.m_used_handles);
        return create_at<obj, args&&...>(handle, std::forward<args>(_args)...);
    }

    template<typename obj, typename... args>
    obj& create_at(const t& _handle, args&&... _args) /* requires std::is_base_of_v<object_base<t>, obj>*/
    {
        if (exists(_handle))
            throw std::runtime_error(format_printer::print("[0]", prompt_id::k_used_handle, _handle));
        m_allocator.insert_handle(_handle);
        m_map_objects[_handle] =
          std::reinterpret_pointer_cast<object_base<t>>(std::make_shared<obj>(std::forward<args>(_args)...));
        m_map_objects[_handle]->set_handle(_handle);
        return *reinterpret_cast<obj*>(m_map_objects[_handle].get());
    }

    bool exists(const t& _handle) const { return !m_allocator.is_valid_handle(_handle); }

  protected:
    handle_allocator<t> m_allocator;
    std::unordered_map<t, std::shared_ptr<object_base<t>>> m_map_objects;
};

// ---------------------- Specialization(this) ------------------
template<>
struct handle_deduction<int>
{
    static int next(int _handle) { return _handle + 1; }

    static int next(const std::set<int>& _handles)
    {
        if (_handles.empty())
            return 1;
        return (*std::max_element(_handles.begin(), _handles.end()) + 1);
    }
};

template<typename t>
struct handle_deduction<t*>
{
    static const void* reserved()
    {
        static intptr_t count = 1;
        return reinterpret_cast<const void*>(count++);
    }

    static const void* next(void* _handle) { return reserved(); }

    static const void* next(const std::set<const void*>& _handles) { return reserved(); }
};

// --------------------- Specialization(other) ------------------
}
