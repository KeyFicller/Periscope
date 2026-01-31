#pragma once

#include <cstddef>

namespace periscope {
// ------------------------ Main template -----------------------
using type_hash_result = std::size_t;

namespace internal {

template<typename type>
constexpr const char*
hash_key_impl() noexcept
{
#ifdef _MSC_VER
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
}
}
// hash_key is the function signature of the type
template<typename type>
constexpr const char*
hash_key() noexcept
{
    using base_type = std::remove_cv_t<type>;
    return internal::hash_key_impl<base_type>();
}

namespace internal {
constexpr std::size_t
strlen(const char* _str)
{
    std::size_t len = 0;
    while (_str[len] != '\0')
        len++;
    return len;
}

constexpr type_hash_result
static_hash_impl(const char* _str, std::size_t _len)
{
    constexpr type_hash_result offset_basis = 14695981039346656037ULL; // FNV-1a offset basis
    constexpr type_hash_result prime = 1099511628211ULL;               // FNV-1a prime

    type_hash_result hash = offset_basis;
    for (std::size_t i = 0; i < _len; ++i) {
        hash ^= static_cast<unsigned char>(_str[i]);
        hash *= prime;
    }
    return hash;
}

template<typename type>
constexpr type_hash_result
static_hash_impl()
{
    return internal::static_hash_impl(hash_key<type>(), internal::strlen(hash_key<type>()));
}
}

// static hash is the hash result of a type calculated in compile time
template<typename type>
constexpr type_hash_result
static_hash()
{
    return internal::static_hash_impl<type>();
}
// ---------------------- Specialization(this) ------------------
}