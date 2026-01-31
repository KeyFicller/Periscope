#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"
#include "type_hash/periscope_type_hash.h"

#include <any>
#include <map>

namespace periscope {
// ------------------------ Main template -----------------------

class base_object
{
  private:
    template<typename child_list>
    struct remove_child_properties_helper;

    template<typename... child_props>
    struct remove_child_properties_helper<type_list<child_props...>>
    {
        static void apply(base_object* self) { (self->template remove<child_props>(), ...); }
    };

    template<>
    struct remove_child_properties_helper<type_list<>>
    {
        static void apply(base_object* self) {}
    };

    template<typename parent_list>
    struct check_parent_properties_helper;

    template<typename... parent_props>
    struct check_parent_properties_helper<type_list<parent_props...>>
    {
        static void apply(const base_object* self)
        {
            if ((self->template has<parent_props>() && ...) == false)
                throw std::runtime_error("Parent property doesn't exist");
        }
    };

    template<>
    struct check_parent_properties_helper<type_list<>>
    {
        static void apply(const base_object* self) {}
    };

    template<typename prop>
    void on_property_removed()
    {
        using child_list = typename prop::child_properties;
        remove_child_properties_helper<child_list>::apply(this);
    }

    template<typename prop>
    void on_property_added()
    {
        using parent_list = typename prop::parent_properties;
        check_parent_properties_helper<parent_list>::apply(this);
    }

  protected:
    base_object() = default;

  public:
    virtual ~base_object() = default;

  public:
    // interfaces
    virtual std::string to_string(graph_type graph_type) const = 0;

  public:
    template<typename prop>
    bool has() const
    {
        return m_properties.contains(static_hash<prop>());
    }

    template<typename prop>
    const prop& get() const
    {
        if (!has<prop>())
            throw std::runtime_error("Property not found");
        return std::any_cast<const prop&>(m_properties.at(static_hash<prop>()));
    }

    template<typename prop>
    prop& get()
    {
        if (!has<prop>())
            throw std::runtime_error("Property not found");
        return std::any_cast<prop&>(m_properties[static_hash<prop>()]);
    }

    template<typename prop>
    base_object& create()
    {
        on_property_added<prop>();
        if (!has<prop>())
            m_properties[static_hash<prop>()] = prop{};
        return *this;
    }

    template<typename prop>
    prop& get_or_create()
    {
        if (!has<prop>()) {
            create<prop>();
        }
        return get<prop>();
    }

    template<typename prop>
    base_object& remove()
    {
        on_property_removed<prop>();
        m_properties.erase(static_hash<prop>());
        return *this;
    }

    std::shared_ptr<base_handle> get_handle() const { return m_handle; }

  private:
    std::map<type_hash_result, std::any> m_properties;
    template<typename underlying_type>
    friend class graph;

    std::shared_ptr<base_handle> m_handle;
};

template<typename derived>
class object : public base_object
{
  public:
    object() = default;

    template<typename prop>
    object& set(const prop::type& _value)
    {
        static_assert(std::is_base_of_v<typename prop::owner_type, derived>,
                      "Owner must be a derived class of prop::owner_type");
        get_or_create<prop>().Value = _value;
        return *this;
    }

  public:
    std::string to_string(graph_type graph_type) const override
    {
        if (!has<OP_printable>())
            return "";
        return static_cast<const derived*>(this)->to_string_impl(graph_type);
    }

    std::string to_string_impl(graph_type graph_type) const
    {
        if (has<OP_name>())
            return get<OP_name>().Value;
        return "";
    }

  protected:
};

}