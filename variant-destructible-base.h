#pragma once

#include "variant-type-traits.h"
#include "variant-helpers.h"
#include "variant-storage.h"

#include <utility>


namespace variant_impl {

template <typename... Types>
struct variant_destructible_base {

  constexpr variant_destructible_base()
      : holding_index(variant_npos)
  {}

  template <std::size_t Id, typename... Args>
  constexpr explicit variant_destructible_base(
      in_place_index_t<Id>, Args&&... args) try
      : holding_index(Id),
        storage(in_place_index<Id>, std::forward<Args>(args)...)
  {} catch(...) {
    holding_index = variant_npos;
    throw;
  }

  constexpr ~variant_destructible_base() {
    destroy();
  }

  constexpr void destroy() {
    if (holding_index != variant_npos) {
      internal_visit([]<typename T>(T const& val) { val.~T(); }, *this);
      holding_index = variant_npos;
    }
  }

  std::size_t holding_index;
  storage_t<Types...> storage;
};


template <typename... Types>
requires (TriviallyDestructible<Types...>)
struct variant_destructible_base<Types...> {

  constexpr variant_destructible_base()
      : holding_index(variant_npos)
  {}

  template <std::size_t Id, typename... Args>
  constexpr explicit variant_destructible_base(
      in_place_index_t<Id>, Args&&... args) try
      : holding_index(Id),
        storage(in_place_index<Id>, std::forward<Args>(args)...)
  {} catch(...) {
    holding_index = variant_npos;
    throw;
  }

  constexpr ~variant_destructible_base() = default;

  constexpr void destroy() {
    holding_index = variant_npos;
  }

  std::size_t holding_index;
  storage_t<Types...> storage;
};

}

