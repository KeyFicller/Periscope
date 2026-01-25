#pragma once

#include <type_traits>

namespace periscope {
// ------------------------ Main template -----------------------
template<typename t>
struct key_deduction
{
    using type = t;
};

template<typename t>
struct key_type
{
    using type = typename key_deduction<t>::type;
};

// ---------------------- Specialization(this) ------------------
template<typename t>
struct key_deduction<t*>
{
    using type = const void*;
};

// --------------------- Specialization(other) ------------------
}

#define PSCP_KEY_T(X) ::periscope::key_type<decltype(X)>::type