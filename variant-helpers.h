#pragma once

#include "variant-type-traits.h"

#include <array>
#include <limits>
#include <tuple>


template <typename... Types>
struct variant;

template <std::size_t Id, typename T>
struct variant_alternative;

template <std::size_t Id, typename T>
using variant_alternative_t = typename variant_alternative<Id, T>::type;

template <std::size_t Id, typename... Types>
constexpr variant_alternative_t<Id, variant<Types...>>&
get(variant<Types...>& v);

template <std::size_t Id, typename... Types>
constexpr variant_alternative_t<Id, variant<Types...>>&&
get(variant<Types...>&& v);

template <std::size_t Id, typename... Types>
constexpr const variant_alternative_t<Id, variant<Types...>>&
get(const variant<Types...>& v);

template <std::size_t Id, typename... Types>
constexpr const variant_alternative_t<Id, variant<Types...>>&&
get(const variant<Types...>&& v);

template <typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor&&, Variants&&...);

template <typename R, typename Visitor, typename... Variants>
constexpr R visit(Visitor&&, Variants&&...);

template <typename T, typename... Types>
constexpr bool holds_alternative(variant<Types...> const& v) noexcept;


template <std::size_t Id>
struct in_place_index_t {
  explicit in_place_index_t() = default;
};

template <std::size_t Id>
inline constexpr in_place_index_t<Id> in_place_index{};


template <typename T>
struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template <typename T>
inline constexpr in_place_type_t<T> in_place_type{};


inline constexpr std::size_t variant_npos = std::numeric_limits<std::size_t>::max();


namespace variant_impl {

template<std::size_t Id, typename T, typename... TRest>
requires (InBound<Id, T, TRest...>)
struct alternative_by_index {
  using type = typename alternative_by_index<Id - 1, TRest...>::type;
};

template<typename T, typename... TRest>
struct alternative_by_index<0, T, TRest...> {
  using type = T;
};


template<typename S, std::size_t Id, typename T, typename... TRest>
struct index_by_type {
  static constexpr std::size_t index = (std::is_same_v<S, T> ?
      Id : index_by_type<S, Id + 1, TRest...>::index);
};

template<typename S, std::size_t Id, typename T>
struct index_by_type<S, Id, T> {
  static constexpr std::size_t index = (std::is_same_v<S, T> ? Id : variant_npos);
};


template <typename T>
struct is_in_place_index_t_specialization {
  constexpr static bool value = false;
};

template <std::size_t Id>
struct is_in_place_index_t_specialization<in_place_index_t<Id>> {
  constexpr static bool value = true;
};


template <typename T>
struct is_variant_specialization {
  constexpr static bool value = false;
};

template <typename... Types>
struct is_variant_specialization<variant<Types...>> {
  constexpr static bool value = true;
};


template <typename... Types>
union storage_t;


template <typename T>
struct is_storage_t_specialization {
  constexpr static bool value = false;
};

template <typename... Types>
struct is_storage_t_specialization<storage_t<Types...>> {
  constexpr static bool value = true;
};

}


struct bad_variant_access : std::runtime_error {
  explicit bad_variant_access(const std::string& arg)
      : std::runtime_error(arg) {}

  explicit bad_variant_access(const char* arg)
      : std::runtime_error(arg) {}
};


template <typename T>
struct variant_size;

template <typename... Types>
struct variant_size<variant<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <typename T>
struct variant_size<const T>
    : variant_size<T> {};

template <typename T>
inline constexpr std::size_t variant_size_v = variant_size<T>::value;


template <std::size_t Id, typename... Types>
struct variant_alternative<Id, variant<Types...>> {
  using type = typename variant_impl::alternative_by_index<Id, Types...>::type;
};

template <std::size_t Id, typename T>
struct variant_alternative<Id, const T> {
  using type = const typename variant_alternative<Id, T>::type;
};


namespace variant_impl {

template <typename... Types>
struct variant_destructible_base;


template <typename Func, std::size_t... SizeRest>
struct multidimensional_table {
  template <typename... IndexRest>
  constexpr Func get_function_ptr(IndexRest... index_rest) const {
    return storage;
  }
  Func storage;
};


template <typename Func, std::size_t Size0, std::size_t... SizeRest>
struct multidimensional_table<Func, Size0, SizeRest...> {
  template <typename Index0, typename... IndexRest>
  constexpr Func get_function_ptr(Index0 index0, IndexRest... index_rest) const {
    return storage[index0].get_function_ptr(index_rest...);
  }
  std::array<multidimensional_table<Func, SizeRest...>, Size0> storage;
};


template <typename Func, typename SizesWrapper, typename CapturedIndexesWrapper>
struct table_builder;


template <typename R, typename Visitor, typename... Objects,
          std::size_t... SizeRest, std::size_t... CapturedIndexes>
struct table_builder<R (*)(Visitor, Objects...),
                     std::index_sequence<SizeRest...>,
                     std::index_sequence<CapturedIndexes...>> {

  constexpr static decltype(auto) invoker(Visitor&& vis, Objects&&... vars) {
    return std::forward<Visitor>(vis)(get<CapturedIndexes>(std::forward<Objects>(vars))...);
  }

  constexpr static decltype(auto) init_level() {
    return multidimensional_table<R (*)(Visitor&&, Objects&&...)>{&invoker};
  }
};


template <typename R, typename Visitor, typename... Objects,
          std::size_t Size0, std::size_t... SizeRest, std::size_t... CapturedIndexes>
struct table_builder<R (*)(Visitor, Objects...),
                     std::index_sequence<Size0, SizeRest...>,
                     std::index_sequence<CapturedIndexes...>> {

  template <std::size_t... LevelIndexes>
  constexpr static decltype(auto) init_cell(std::index_sequence<LevelIndexes...>) {
    multidimensional_table<R (*)(Visitor&&, Objects&&...),
                           Size0,
                           SizeRest...> table;
    ((table.storage[LevelIndexes] =
          table_builder<R (*)(Visitor, Objects...),
          std::index_sequence<SizeRest...>,
          std::index_sequence<CapturedIndexes..., LevelIndexes>>
      ::init_level()), ...);
    return table;
  }

  constexpr static decltype(auto) init_level() {
    return init_cell(std::make_index_sequence<Size0>());
  }
};


template <typename R, typename Visitor, typename... Variants>
struct invoker {
  constexpr static decltype(auto) invoke(Visitor&& vis, Variants&&... vars) {
    return (*working_table.get_function_ptr(vars.index()...))
        (std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
  }
  constexpr static auto working_table = table_builder<R (*)(Visitor&&, Variants&&...),
                                                      std::index_sequence<variant_size_v<std::remove_reference_t<Variants>>...>,
                                                      std::index_sequence<>>::init_level();
};


template <typename T>
struct storage_size;

template <typename... Types>
struct storage_size<storage_t<Types...>>
    : std::integral_constant<std::size_t, sizeof...(Types)> {};

template <typename T>
struct storage_size<const T>
    : storage_size<T> {};

template <typename T>
inline constexpr std::size_t storage_size_v = storage_size<T>::value;


template <typename T>
  struct take_storage_t;

template <typename... Types>
struct take_storage_t<variant<Types...>> {
  using type = storage_t<Types...>;
};

template <typename... Types>
struct take_storage_t<const variant<Types...>> {
  using type = const storage_t<Types...>;
};

template <typename... Types>
struct take_storage_t<variant_destructible_base<Types...>> {
  using type = storage_t<Types...>;
};

template <typename... Types>
struct take_storage_t<const variant_destructible_base<Types...>> {
  using type = const storage_t<Types...>;
};


template <typename Visitor, typename... Variants>
struct internal_invoker {
  constexpr static decltype(auto) take_last(Variants&&... vars) {
    return (std::forward<Variants>(vars), ...);
  }

  template <typename T>
  constexpr static std::size_t get_or_default(T&& var, std::size_t def) {
    return var.holding_index == variant_npos ? def : var.holding_index;
  }

  /* Last item in vars... must hold some index -
   * it's called default and applied for every non-holding item in vars... */
  constexpr static decltype(auto) invoke(Visitor&& vis, Variants&&... vars) {
    std::size_t def = take_last(std::forward<Variants>(vars)...).holding_index;
    return (*working_table.get_function_ptr(get_or_default(std::forward<Variants>(vars), def)...))
        (std::forward<Visitor>(vis), std::forward<typename take_storage_t<std::remove_reference_t<Variants>>::type>(std::forward<Variants>(vars).storage)...);
  }
  constexpr static auto working_table = table_builder<void (*)(Visitor&&, typename take_storage_t<std::remove_reference_t<Variants>>::type&&...),
                                                      std::index_sequence<storage_size_v<std::remove_reference_t<decltype(std::remove_reference_t<Variants>::storage)>>...>,
                                                      std::index_sequence<>>::init_level();
};


template <typename Visitor, typename... Variants>
constexpr void internal_visit(Visitor&& vis, Variants&&... vars)  {
  return variant_impl::internal_invoker<Visitor, Variants...>
      ::invoke(std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
}

}

