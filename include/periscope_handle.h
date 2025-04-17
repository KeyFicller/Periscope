#pragma once

#include "periscope_object.h"

#include <set>
#include <unordered_map>

namespace periscope
{
    // ------------------------ Main template -----------------------
    template <typename t>
    struct handle_deduction {};

    template <typename t>
    concept next_deduction_enabled = requires(const t& _handle)
    {
        handle_deduction<t>::next(_handle);
    };

    template <typename t>
    concept overall_deduction_enabled = requires(const std::set<t>&_handles)
    {
        handle_deduction<t>::next(_handles);
    };

    template <typename t>
    class handle_allocator
    {
        friend class handle_manager;
    public:
        bool is_valid_handle(const t& _handle) const
        {
            return !m_used_handles.find(_handle);
        }

        void insert_handle(const t& _handle) const
        {
            if (is_valid_handle(_handle))
                m_used_handles.insert(_handle);
        }

        [[nodiscard]] t next_handle(const t& _handle) const requires next_deduction_enabled<t>
        {
            return handle_deduction<t>::next(_handle);
        }

    protected:
        std::set<t> m_used_handles;
    };

    template <typename t>
    class handle_manager
    {
    public:
        template <typename obj>
        obj& access(const t& _handle) const
        {
            if (!exists(_handle))
                throw std::exception("Invalid Handle");
            obj* object = dynamic_cast<obj*>(m_map_objects.at(_handle));
            if (!object)
                throw std::exception("Invalid Type");
            return *object;
        }

        template <typename obj, typename... args>
        obj& create(args&&... _args) requires overall_deduction_enabled<t>
        {
            t handle = handle_deduction::next(m_allocator.m_used_handles);

        }

        template <typename obj, typename... args>
        obj& create_at(const t& _handle, args&&... _args)
        {
            if (exists(_handle))
                throw std::exception("Duplicate Handle");
            m_allocator
            m_map_objects[]
        }

        bool exists(const t& _handle) const
        {
            return !m_allocator.is_valid_handle(_handle);
        }


    protected:
        handle_allocator<t> m_allocator;
        std::unordered_map<t, object<t>*> m_map_objects;
    };

    // ---------------------- Specialization(this) ------------------
    template <>
    struct handle_deduction<int>
    {
        static int next(int _handle)
        {
            return _handle + 1;
        }

        static int next(const std::set<int>& _handles)
        {
            if (_handles.empty())
                return 1;
            return (*_handles.cbegin() + 1);
        }
    };


    // --------------------- Specialization(other) ------------------
}
