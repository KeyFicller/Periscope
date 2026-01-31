#pragma once

#include "graph/periscope_graph_fwd.h"
#include <set>
#include <string>
#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------

template<typename underlying_type>
class graph;

// base_handle is base class for all handles
class base_handle
{
  public:
    // print is interface to convert handle to string representation
    virtual std::string print(graph_type graph_type) const = 0;
};

// handle is typed handle with underlying type
template<typename underlying_type = unsigned int>
class handle : public base_handle
{
    using id_type = underlying_type;

  public:
    handle(id_type _id)
      : m_id(_id)
    {
    }

    // id is accessor to handle id
    id_type id() const { return m_id; }

    // print is implementation of base_handle::print
    std::string print(graph_type graph_type) const override
    {
        if constexpr (std::is_same_v<id_type, std::string>) {
            return m_id;
        } else if constexpr (std::is_integral_v<id_type>) {
            if (graph_type == graph_type::k_sequence) {
                return "OBJ" + std::to_string(m_id);
            } else {
                return std::to_string(m_id);
            }
        } else {
            return std::to_string(m_id);
        }
    }

  protected:
    id_type m_id;
};

// operator== is equality comparison for handles
template<typename underlying_type>
inline bool
operator==(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() == rhs.id();
}

// operator!= is inequality comparison for handles
template<typename underlying_type>
inline bool
operator!=(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() != rhs.id();
}

// operator< is less-than comparison for handles
template<typename underlying_type>
inline bool
operator<(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() < rhs.id();
}

// operator> is greater-than comparison for handles
template<typename underlying_type>
inline bool
operator>(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() > rhs.id();
}

// handle_allocator is default allocator for handles
template<typename underlying_type, typename = void>
class handle_allocator
{
  public:
    // allocate is to allocate a new handle
    static handle<void*> allocate()
    {
        static unsigned int next_id = 0;
        return handle<void*>(reinterpret_cast<void*>(next_id++));
    }
};

// handle_manager is manager for handle allocation and deallocation
template<typename underlying_type>
class handle_manager
{
  public:
    // allocate is to allocate a unique handle
    handle<underlying_type> allocate()
    {
        while (true) {
            auto handle = handle_allocator<underlying_type>::allocate();
            if (m_handles.find(handle) == m_handles.end()) {
                m_handles.insert(handle);
                return handle;
            }
        }
    }

    // deallocate is to deallocate a handle
    void deallocate(const handle<underlying_type>& handle) { m_handles.erase(handle); }

    // is_allocated is to check if handle is allocated
    bool is_allocated(const handle<underlying_type>& handle) const { return m_handles.find(handle) != m_handles.end(); }

    // allocate_at is to allocate a handle at specific value
    bool allocate_at(const handle<underlying_type>& handle) { return m_handles.insert(handle).second; }

  protected:
    std::set<handle<underlying_type>> m_handles;
};

// ------------------------ Specializations -----------------------
// handle_allocator is specialization for integral types
template<typename underlying_type>
class handle_allocator<underlying_type, std::enable_if_t<std::is_integral_v<underlying_type>>>
{
  public:
    // allocate is to allocate a handle with integral type
    static handle<underlying_type> allocate()
    {
        static underlying_type next_id = 0;
        return handle<underlying_type>(next_id++);
    }
};

// handle_allocator is specialization for std::string
template<>
class handle_allocator<std::string>
{
  public:
    // allocate is to allocate a handle with string type
    static handle<std::string> allocate()
    {
        static unsigned int next_id = 0;
        return handle<std::string>("handle_" + std::to_string(next_id++));
    }
};

}