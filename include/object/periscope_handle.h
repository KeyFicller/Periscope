#pragma once

#include <set>
#include <string>
#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------

template<typename underlying_type>
class graph;

class base_handle
{
  public:
    virtual std::string print() const = 0;
};

template<typename underlying_type = unsigned int>
class handle : public base_handle
{
    using id_type = underlying_type;

  public:
    handle(id_type _id)
      : m_id(_id)
    {
    }

    id_type id() const { return m_id; }

    std::string print() const override
    {
        if constexpr (std::is_same_v<id_type, std::string>) {
            return m_id;
        } else {
            return std::to_string(m_id);
        }
    }

  protected:
    id_type m_id;
};

template<typename underlying_type>
inline bool
operator==(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() == rhs.id();
}

template<typename underlying_type>
inline bool
operator!=(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() != rhs.id();
}

template<typename underlying_type>
inline bool
operator<(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() < rhs.id();
}

template<typename underlying_type>
inline bool
operator>(const handle<underlying_type>& lhs, const handle<underlying_type>& rhs)
{
    return lhs.id() > rhs.id();
}

template<typename underlying_type, typename = void>
class handle_allocator
{
  public:
    static handle<void*> allocate()
    {
        static unsigned int next_id = 0;
        return handle<void*>(reinterpret_cast<void*>(next_id++));
    }
};

template<typename underlying_type>
class handle_manager
{
  public:
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

    void deallocate(const handle<underlying_type>& handle) { m_handles.erase(handle); }

    bool is_allocated(const handle<underlying_type>& handle) const { return m_handles.find(handle) != m_handles.end(); }

    bool allocate_at(const handle<underlying_type>& handle) { return m_handles.insert(handle).second; }

  protected:
    std::set<handle<underlying_type>> m_handles;
};

// ------------------------ Specializations -----------------------
template<typename underlying_type>
class handle_allocator<underlying_type, std::enable_if_t<std::is_integral_v<underlying_type>>>
{
  public:
    static handle<underlying_type> allocate()
    {
        static underlying_type next_id = 0;
        return handle<underlying_type>(next_id++);
    }
};

template<>
class handle_allocator<std::string>
{
  public:
    static handle<std::string> allocate()
    {
        static unsigned int next_id = 0;
        return handle<std::string>("handle_" + std::to_string(next_id++));
    }
};

}