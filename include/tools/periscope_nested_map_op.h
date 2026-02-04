#pragma once

#include <type_traits>
#include <vector>

// ------C++14 compatible nested map operations --------
namespace periscope {

namespace internal {

template<typename NestedMap, typename CurrentKey>
using nested_map_mapped =
  std::remove_reference_t<decltype(std::declval<NestedMap&>().at(std::declval<const CurrentKey&>()))>;

template<typename...>
struct make_void
{
    using type = void;
}; // void_t is introduced in C++17

template<typename NestedMap, typename CurrentKey, typename = void>
struct is_key_findable : std::false_type
{};

template<typename NestedMap, typename CurrentKey>
struct is_key_findable<
  NestedMap,
  CurrentKey,
  typename make_void<decltype(std::declval<NestedMap>().find(std::declval<const CurrentKey&>()))>::type>
  : std::true_type
{};

template<typename NestedMap, typename CurrentKey>
constexpr bool is_key_findable_v = is_key_findable<NestedMap, CurrentKey>::value;

template<typename NestedMap, typename KeySequence, typename = void>
struct is_key_sequence_findable_impl : std::false_type
{};

template<typename NestedMap, typename LastKey>
struct is_key_sequence_findable_impl<NestedMap,
                                     std::tuple<LastKey>,
                                     std::enable_if_t<is_key_findable_v<NestedMap, LastKey>>> : std::true_type
{};

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
struct is_key_sequence_findable_impl<
  NestedMap,
  std::tuple<CurrentKey, SubsequentKeys...>,
  std::enable_if_t<
    is_key_findable_v<NestedMap, CurrentKey> &&
    is_key_sequence_findable_impl<nested_map_mapped<NestedMap, CurrentKey>, std::tuple<SubsequentKeys...>>::value>>
  : std::true_type
{};

template<typename NestedMap, typename... Keys>
struct is_key_sequence_findable : is_key_sequence_findable_impl<NestedMap, std::tuple<Keys...>>
{};

template<typename NestedMap, typename... Keys>
constexpr bool is_key_sequence_findable_v = is_key_sequence_findable<NestedMap, Keys...>::value;

template<typename NestedMap, typename LastKey>
auto
find_impl(NestedMap& nestMap, LastKey&& key)
{
    return std::make_pair(&nestMap, nestMap.find(std::forward<LastKey>(key)));
}

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
decltype(find_impl(std::declval<nested_map_mapped<NestedMap, CurrentKey>&>(), std::declval<SubsequentKeys&>()...))
find_impl(NestedMap& nestMap, CurrentKey&& key, SubsequentKeys&&... keys)
{
    auto iter = nestMap.find(std::forward<CurrentKey>(key));
    if (iter == nestMap.end())
        return { nullptr, {} };
    return find_impl(iter->second, std::forward<SubsequentKeys>(keys)...);
}

template<typename NestedMap, typename LastKey>
void
ensure_impl(NestedMap& nestMap, LastKey&& key)
{
    if (nestMap.find(std::forward<LastKey>(key)) == nestMap.end())
        nestMap.emplace(std::forward<LastKey>(key), typename NestedMap::mapped_type{});
}

template<typename NestedMap, typename LastKey, typename Value>
std::enable_if_t<
  std::is_assignable<decltype(std::declval<NestedMap&>().at(std::declval<const LastKey&>())), Value>::value>
ensure_impl(NestedMap& nestMap, LastKey&& key, Value&& value)
{
    if (nestMap.find(std::forward<LastKey>(key)) == nestMap.end())
        nestMap.emplace(std::forward<LastKey>(key), std::forward<Value>(value));
}

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeyVals>
void
ensure_impl(NestedMap& nestMap, CurrentKey&& key, SubsequentKeyVals&&... keyVals)
{
    auto iter = nestMap.find(std::forward<CurrentKey>(key));
    if (iter == nestMap.end()) {
        auto result = nestMap.emplace(std::forward<CurrentKey>(key), typename NestedMap::mapped_type{});
        iter = result.first;
    }
    ensure_impl(iter->second, std::forward<SubsequentKeyVals>(keyVals)...);
}

template<typename NestedMap, typename KeyValSequence, typename = void>
struct is_key_value_sequence_valid_impl : std::false_type
{};

template<typename NestedMap, typename LastKey>
struct is_key_value_sequence_valid_impl<NestedMap,
                                        std::tuple<LastKey>,
                                        std::enable_if_t<is_key_findable_v<NestedMap, LastKey>>> : std::true_type
{};

template<typename NestedMap, typename LastKey, typename Value>
struct is_key_value_sequence_valid_impl<
  NestedMap,
  std::tuple<LastKey, Value>,
  std::enable_if_t<
    is_key_findable_v<NestedMap, LastKey> &&
    std::is_assignable<decltype(std::declval<NestedMap&>().at(std::declval<const LastKey&>())), Value>::value>>
  : std::true_type
{};

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeyVals>
struct is_key_value_sequence_valid_impl<
  NestedMap,
  std::tuple<CurrentKey, SubsequentKeyVals...>,
  std::enable_if_t<is_key_findable_v<NestedMap, CurrentKey> &&
                   is_key_value_sequence_valid_impl<nested_map_mapped<NestedMap, CurrentKey>,
                                                    std::tuple<SubsequentKeyVals...>>::value>> : std::true_type
{};

template<typename NestedMap, typename... KeyVals>
struct is_key_value_sequence_valid : is_key_value_sequence_valid_impl<NestedMap, std::tuple<KeyVals...>>
{};

template<typename NestedMap, typename... KeyVals>
constexpr bool is_key_value_sequence_valid_v = is_key_value_sequence_valid<NestedMap, KeyVals...>::value;

} // namespace internal

template<typename NestedMap, typename... Keys>
auto
find(NestedMap& nestMap, Keys&&... keys)
{
    static_assert(internal::is_key_sequence_findable_v<NestedMap, Keys...>,
                  "KeySequence not applicable for NMap_Op::find");
    return internal::find_impl(nestMap, std::forward<Keys>(keys)...);
}

template<typename NestedMap, typename... KeyVals>
void
ensure(NestedMap& nestMap, KeyVals&&... keyVals)
{
    static_assert(internal::is_key_value_sequence_valid_v<NestedMap, KeyVals...>,
                  "KeyValSequence is not applicable for NMap_Op::ensure");
    internal::ensure_impl(nestMap, std::forward<KeyVals>(keyVals)...);
}
} // namespace periscope