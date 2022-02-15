#pragma once

#include "variant-helpers.h"
#include "variant-type-traits.h"


namespace variant_impl {

template <typename... Types>
union storage_t
{};


template <typename T0, typename... TRest>
requires(TriviallyDestructible<T0, TRest...>)
union storage_t<T0, TRest...> {

  constexpr storage_t()
      : rest_alternatives()
  {}

  template <std::size_t Id, typename... Args>
  constexpr explicit storage_t(in_place_index_t<Id>, Args&&... args)
      : rest_alternatives(in_place_index<Id - 1>, std::forward<Args>(args)...)
  {}

  template <typename... Args>
  constexpr explicit storage_t(in_place_index_t<0>, Args&&... args)
      : current_alternative(std::forward<Args>(args)...)
  {}

  constexpr ~storage_t() = default;

  template <typename... Args>
  constexpr void construct(Args&&... args) {
    new (const_cast<std::remove_cvref_t<T0>*>
         (std::addressof(current_alternative)))
        T0(std::forward<Args>(args)...);
  }

  constexpr T0& get_current() {
    return current_alternative;
  }

  constexpr const T0& get_current() const {
    return current_alternative;
  }

  T0 current_alternative;
  storage_t<TRest...> rest_alternatives;
};


template <typename T0, typename... TRest>
union storage_t<T0, TRest...> {

  constexpr storage_t()
      : rest_alternatives()
  {}

  template <std::size_t Id, typename... Args>
  constexpr explicit storage_t(in_place_index_t<Id>, Args&&... args)
      : rest_alternatives(in_place_index<Id - 1>, std::forward<Args>(args)...)
  {}

  template <typename... Args>
  constexpr explicit storage_t(in_place_index_t<0>, Args&&... args)
      : current_alternative(std::forward<Args>(args)...)
  {}

  constexpr ~storage_t()
  {}

  template <typename... Args>
  constexpr void construct(Args&&... args) {
    new (const_cast<std::remove_cvref_t<T0>*>
         (std::addressof(current_alternative)))
        T0(std::forward<Args>(args)...);
  }

  constexpr T0& get_current() {
    return current_alternative;
  }

  constexpr const T0& get_current() const {
    return current_alternative;
  }

  T0 current_alternative;
  storage_t<TRest...> rest_alternatives;
};

template <std::size_t Id, typename Storage>
constexpr decltype(auto) get(Storage&& storage)
    requires(is_storage_t_specialization<std::remove_cvref_t<Storage>>::value) {
  if constexpr (Id == 0) {
    return storage.get_current();
  } else {
    return get<Id - 1>(
        std::forward<Storage>(storage).rest_alternatives);
  }
}

}

