#pragma once

#include "graph/periscope_graph_fwd.h"
#include "object/periscope_handle.h"
#include "object/periscope_object_properties.h"
#include "type_hash/periscope_type_hash.h"

#include <any>
#include <map>

namespace periscope {
// ------------------------ Main template -----------------------

// base_object is the base class for all objects with property management
class base_object
{
  private:
    // remove_child_properties_helper is helper to remove child properties from type_list
    template<typename child_list>
    struct remove_child_properties_helper;

    // remove_child_properties_helper is specialization for non-empty type_list
    template<typename... child_props>
    struct remove_child_properties_helper<type_list<child_props...>>
    {
        static void apply(base_object* self) { (self->template remove<child_props>(), ...); }
    };

    // remove_child_properties_helper is specialization for empty type_list
    template<>
    struct remove_child_properties_helper<type_list<>>
    {
        static void apply(base_object* self) {}
    };

    // check_parent_properties_helper is helper to check parent properties from type_list
    template<typename parent_list>
    struct check_parent_properties_helper;

    // check_parent_properties_helper is specialization for non-empty type_list
    template<typename... parent_props>
    struct check_parent_properties_helper<type_list<parent_props...>>
    {
        static void apply(const base_object* self)
        {
            if ((self->template has<parent_props>() && ...) == false)
                throw std::runtime_error("Parent property doesn't exist");
        }
    };

    // check_parent_properties_helper is specialization for empty type_list
    template<>
    struct check_parent_properties_helper<type_list<>>
    {
        static void apply(const base_object* self) {}
    };

    // on_property_removed is callback when property is removed
    template<typename prop>
    void on_property_removed()
    {
        using child_list = typename prop::child_properties;
        remove_child_properties_helper<child_list>::apply(this);
    }

    // on_property_added is callback when property is added
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
    // to_string is interface to convert object to string representation
    virtual std::string to_string(graph_type _graph_type = io().GraphType) const = 0;

  public:
    // has is whether the property exists
    template<typename prop>
    bool has() const
    {
        return m_properties.contains(static_hash<prop>());
    }

    // get is const accessor to property
    template<typename prop>
    const prop& get() const
    {
        if (!has<prop>())
            throw std::runtime_error("Property not found");
        return std::any_cast<const prop&>(m_properties.at(static_hash<prop>()));
    }

    // get is mutable accessor to property
    template<typename prop>
    prop& get()
    {
        if (!has<prop>())
            throw std::runtime_error("Property not found");
        return std::any_cast<prop&>(m_properties[static_hash<prop>()]);
    }

    // is_printable_if_unset is to check if property is printable if unset
    template<typename prop, typename = void>
    struct is_printable_if_unset : std::false_type
    {};

    // is_printable_if_unset is specialization for property with to_string_default
    template<typename prop>
    struct is_printable_if_unset<prop, std::void_t<decltype(prop::to_string_default)>> : std::true_type
    {};

    // str is to convert property to string representation
    template<typename prop>
    std::string _V_str(graph_type _graph_type = io().GraphType) const
    {
        if (!has<prop>()) {
            if constexpr (is_printable_if_unset<prop>::value) {
                return prop::to_string_default(_graph_type);
            } else {
                return "";
            }
        }
        return prop::to_string(get<prop>(), _graph_type);
    }

    // create is to create a property if it doesn't exist
    template<typename prop>
    base_object& create()
    {
        on_property_added<prop>();
        if (!has<prop>())
            m_properties[static_hash<prop>()] = prop{};
        return *this;
    }

    // get_or_create is to get property or create if it doesn't exist
    template<typename prop>
    prop& get_or_create()
    {
        if (!has<prop>()) {
            create<prop>();
        }
        return get<prop>();
    }

    // remove is to remove a property and its child properties
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

// object is CRTP base class for typed objects with property management
template<typename derived>
class object : public base_object
{
  public:
    object() = default;

    // set is to set property value with type checking
    template<typename prop>
    object& set(const typename prop::type& _value)
    {
        static_assert(std::is_base_of_v<typename prop::owner_type, derived>,
                      "Owner must be a derived class of prop::owner_type");
        get_or_create<prop>().Value = _value;
        return *this;
    }

    // set overload: accept objects with get_handle() for handle properties
    template<typename prop, typename t>
        requires(std::is_same_v<typename prop::type, std::shared_ptr<base_handle>> && has_get_handle<t>::value &&
                 !std::is_same_v<t, typename prop::type>)
    object& set(const t& _value)
    {
        static_assert(std::is_base_of_v<typename prop::owner_type, derived>,
                      "Owner must be a derived class of prop::owner_type");
        get_or_create<prop>().Value = _value.get_handle();
        return *this;
    }

  public:
    // to_string is implementation of base_object::to_string
    std::string to_string(graph_type _graph_type = io().GraphType) const override
    {
        if (!has<OP_printable>())
            return "";
        return static_cast<const derived*>(this)->to_string_impl(_graph_type);
    }

    // to_string_impl is default implementation for derived classes
    std::string to_string_impl(graph_type _graph_type = io().GraphType) const
    {
        if (has<OP_name>())
            return get<OP_name>().Value;
        return "";
    }

  protected:
};
}