#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <vector>

// ------C++14 compatible nested map operations --------
namespace periscope {

namespace internal {

template<typename NestedMap, typename Key, typename = void>
struct is_key_applicable : std::false_type
{};

template<typename NestedMap, typename Key>
struct is_key_applicable<NestedMap,
                         Key,
                         std::enable_if_t<std::is_convertible<Key, typename NestedMap::key_type>::value>>
  : std::true_type
{};

template<typename NestedMap, typename Key>
constexpr bool is_key_applicable_v = is_key_applicable<NestedMap, Key>::value;

template<typename NestedMap, typename KeySequence, typename = void>
struct is_key_sequence_applicable_impl : std::false_type
{};

template<typename NestedMap, typename LastKey>
struct is_key_sequence_applicable_impl<NestedMap,
                                       std::tuple<LastKey>,
                                       std::enable_if_t<is_key_applicable_v<NestedMap, LastKey>>> : std::true_type
{};

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
struct is_key_sequence_applicable_impl<
  NestedMap,
  std::tuple<CurrentKey, SubsequentKeys...>,
  std::enable_if_t<
    is_key_applicable<NestedMap, CurrentKey>::value &&
    is_key_sequence_applicable_impl<typename NestedMap::mapped_type, std::tuple<SubsequentKeys...>>::value>>
  : std::true_type
{};

template<typename NestedMap, typename... Keys>
struct is_key_sequence_applicable : is_key_sequence_applicable_impl<NestedMap, std::tuple<Keys...>>
{};

template<typename NestedMap, typename... Keys>
constexpr bool is_key_sequence_applicable_v = is_key_sequence_applicable<NestedMap, Keys...>::value;

template<typename NestedMap, typename KeySequence, typename = void>
struct nested_map_op_traits_impl
{};

template<typename NestedMap, typename Key>
struct nested_map_op_traits_impl<NestedMap, std::tuple<Key>, std::enable_if_t<is_key_applicable_v<NestedMap, Key>>>
{
    using last_container_pointer_type = NestedMap*;
    using last_container_iterator_type =
      std::conditional_t<std::is_const_v<NestedMap>, typename NestedMap::const_iterator, typename NestedMap::iterator>;
    using value_pointer_type = std::conditional_t<std::is_const_v<NestedMap>,
                                                  const typename NestedMap::mapped_type*,
                                                  typename NestedMap::mapped_type*>;
};

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
struct nested_map_op_traits_impl<
  NestedMap,
  std::tuple<CurrentKey, SubsequentKeys...>,
  std::enable_if_t<is_key_applicable_v<NestedMap, CurrentKey> &&
                   is_key_sequence_applicable_v<typename NestedMap::mapped_type, SubsequentKeys...>>>
  : nested_map_op_traits_impl<std::conditional_t<std::is_const_v<NestedMap>,
                                                 const typename NestedMap::mapped_type,
                                                 typename NestedMap::mapped_type>,
                              std::tuple<SubsequentKeys...>>
{};

template<typename NestedMap, typename... Keys>
using nested_map_op_traits = nested_map_op_traits_impl<NestedMap, std::tuple<Keys...>>;

template<typename NestedMap, typename Key>
std::pair<typename nested_map_op_traits<NestedMap, Key>::last_container_pointer_type,
          typename nested_map_op_traits<NestedMap, Key>::last_container_iterator_type>
find_impl(NestedMap& nestMap, const Key& key)
{
    return { &nestMap, nestMap.find(key) };
}

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
std::pair<typename nested_map_op_traits<NestedMap, CurrentKey, SubsequentKeys...>::last_container_pointer_type,
          typename nested_map_op_traits<NestedMap, CurrentKey, SubsequentKeys...>::last_container_iterator_type>
find_impl(NestedMap& nestMap, const CurrentKey& currentKey, const SubsequentKeys&... subsequentKeys)
{
    auto iter = nestMap.find(currentKey);
    if (iter == nestMap.end()) {
        return std::pair<
          typename nested_map_op_traits<NestedMap, CurrentKey, SubsequentKeys...>::last_container_pointer_type,
          typename nested_map_op_traits<NestedMap, CurrentKey, SubsequentKeys...>::last_container_iterator_type>{
            nullptr, {}
        };
    }
    return find_impl(iter->second, subsequentKeys...);
}

template<typename NestedMap, typename Key>
void
ensure_impl(NestedMap& nestMap, const Key& key)
{
    if (nestMap.find(key) == nestMap.end()) {
        nestMap.emplace(key, typename NestedMap::mapped_type{});
    }
}

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
void
ensure_impl(NestedMap& nestMap, const CurrentKey& currentKey, const SubsequentKeys&... subsequentKeys)
{
    if (nestMap.find(currentKey) == nestMap.end()) {
        nestMap.emplace(currentKey, typename NestedMap::mapped_type{});
    }
    ensure_impl(nestMap.at(currentKey), subsequentKeys...);
}

template<typename Key>
struct key_adapter
{
    key_adapter(const std::function<bool(const Key&)>& func)
      : m_func(func)
    {
    }
    bool operator()(const Key& key) const { return m_func ? m_func(key) : false; }

  protected:
    std::function<bool(const Key&)> m_func = nullptr;
};

template<typename NestedMap, typename Key>
struct is_key_applicable<NestedMap,
                         key_adapter<Key>,
                         std::enable_if_t<std::is_convertible<Key, typename NestedMap::key_type>::value>>
  : std::true_type
{};

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
struct is_key_sequence_applicable_impl<
  NestedMap,
  std::tuple<key_adapter<CurrentKey>, SubsequentKeys...>,
  std::enable_if_t<
    is_key_applicable<NestedMap, key_adapter<CurrentKey>>::value &&
    is_key_sequence_applicable_impl<typename NestedMap::mapped_type, std::tuple<SubsequentKeys...>>::value>>
  : std::true_type
{};

template<typename NestedMap, typename Key>
void
view_impl(NestedMap& nestMap,
          std::vector<typename nested_map_op_traits<NestedMap, Key>::value_pointer_type>& result,
          const Key& key)
{
    auto iter = nestMap.find(key);
    if (iter == nestMap.end()) {
        return;
    }
    result.push_back(&iter->second);
}

template<typename NestedMap, typename Key>
void
view_impl(NestedMap& nestMap,
          std::vector<typename nested_map_op_traits<NestedMap, Key>::value_pointer_type>& result,
          const key_adapter<Key>& adapter)
{
    for (auto& iter : nestMap) {
        if (adapter(iter.first)) {
            result.push_back(&iter.second);
        }
    }
}

template<typename NestedMap,
         typename CurrentKey,
         typename... SubsequentKeys,
         typename = std::enable_if_t<(sizeof...(SubsequentKeys) > 0)>>
void
view_impl(
  NestedMap& nestMap,
  std::vector<typename nested_map_op_traits<NestedMap, CurrentKey, SubsequentKeys...>::value_pointer_type>& result,
  const CurrentKey& currentKey,
  const SubsequentKeys&... subsequentKeys)
{
    auto iter = nestMap.find(currentKey);
    if (iter == nestMap.end()) {
        return;
    }
    view_impl(iter->second, result, subsequentKeys...);
}

template<typename NestedMap,
         typename CurrentKey,
         typename... SubsequentKeys,
         typename = std::enable_if_t<(sizeof...(SubsequentKeys) > 0)>>
void
view_impl(
  NestedMap& nestMap,
  std::vector<typename nested_map_op_traits<NestedMap, CurrentKey, SubsequentKeys...>::value_pointer_type>& result,
  const key_adapter<CurrentKey>& adapter,
  const SubsequentKeys&... subsequentKeys)
{
    for (auto& iter : nestMap) {
        if (adapter(iter.first)) {
            view_impl(iter.second, result, subsequentKeys...);
        }
    }
}

template<typename NestedMap, typename Key>
void
truncate_impl(NestedMap& nestMap, const Key& key)
{
    nestMap.erase(key);
}

template<typename NestedMap, typename Key>
void
truncate_impl(NestedMap& nestMap, const key_adapter<Key>& adapter)
{
    for (auto iter = nestMap.begin(); iter != nestMap.end();) {
        if (adapter(iter->first)) {
            iter = nestMap.erase(iter);
        } else {
            ++iter;
        }
    }
}

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
void
truncate_impl(NestedMap& nestMap, const CurrentKey& currentKey, const SubsequentKeys&... subsequentKeys)
{
    auto iter = nestMap.find(currentKey);
    if (iter == nestMap.end()) {
        return;
    }
    truncate_impl(iter->second, subsequentKeys...);
    if (iter->second.empty()) {
        nestMap.erase(iter);
    }
}

template<typename NestedMap, typename CurrentKey, typename... SubsequentKeys>
void
truncate_impl(NestedMap& nestMap, const key_adapter<CurrentKey>& adapter, const SubsequentKeys&... subsequentKeys)
{
    for (auto iter = nestMap.begin(); iter != nestMap.end();) {
        if (adapter(iter->first)) {
            truncate_impl(iter->second, subsequentKeys...);
            if (iter->second.empty()) {
                iter = nestMap.erase(iter);
            } else {
                ++iter;
            }
        } else {
            ++iter;
        }
    }
}
} // namespace internal

template<typename NestedMap, typename... Keys>
auto
find(const NestedMap& nestMap, const Keys&... keys)
{
    static_assert(internal::is_key_sequence_applicable_v<NestedMap, Keys...>,
                  "KeySequence not applicable for NMap_Op::find");
    return internal::find_impl(nestMap, keys...);
}

template<typename NestedMap, typename... Keys>
void
ensure(NestedMap& nestMap, const Keys&... keys)
{
    static_assert(internal::is_key_sequence_applicable_v<NestedMap, Keys...>,
                  "KeySequence is not applicable for NMap_Op::ensure");
    internal::ensure_impl(nestMap, keys...);
}

template<typename NestedMap, typename... Keys>
auto
view(NestedMap& nestMap, const Keys&... keys)
{
    static_assert(internal::is_key_sequence_applicable_v<NestedMap, Keys...>,
                  "KeySequence not applicable for NMap_Op::view");
    std::vector<typename internal::nested_map_op_traits<NestedMap, Keys...>::value_pointer_type> result;
    internal::view_impl(nestMap, result, keys...);
    return result;
}

template<typename NestedMap, typename... Keys>
void
truncate(NestedMap& nestMap, const Keys&... keys)
{
    static_assert(internal::is_key_sequence_applicable_v<NestedMap, Keys...>,
                  "KeySequence not applicable for NMap_Op::truncate");
    internal::truncate_impl(nestMap, keys...);
}

} // namespace periscope