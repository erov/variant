#pragma once

#include "variant-helpers.h"
#include "variant-type-traits.h"
#include "variant-destructible-base.h"

#include <cassert>


template <typename... Types>
struct variant
    : variant_impl::variant_destructible_base<Types...> {

private:
  using base = variant_impl::variant_destructible_base<Types...>;
  using T0 = typename variant_impl::alternative_by_index<0, Types...>::type;

public:
  constexpr variant() noexcept(NothrowConstructible<T0>)
      requires(DefaultConstructible<T0>)
      : base(in_place_index<0>)
  {}


  constexpr variant(variant const&) = delete;
  constexpr variant(variant const&) requires(TriviallyCopyConstructible<Types...>) = default;
  constexpr variant(variant const& other)
      requires(CopyConstructible<Types...> && !TriviallyCopyConstructible<Types...>) {
    if (!other.valueless_by_exception()) {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B&& rhs) {
            if constexpr (std::is_same_v<A, std::remove_cvref_t<B>>) {
              new (const_cast<std::remove_cvref_t<A>*>(std::addressof(lhs)))
                  std::remove_cvref_t<A>(std::forward<B>(rhs));
            }
          },
          *this, other);
    }
    this->holding_index = other.index();
  }


  constexpr variant(variant&& other) = delete;
  constexpr variant(variant&& other) requires(TriviallyMoveConstructible<Types...>) = default;
  constexpr variant(variant&& other) noexcept(NothrowMoveConstructible<Types...>)
      requires(MoveConstructible<Types...> && !TriviallyMoveConstructible<Types...>) {
    if (!other.valueless_by_exception()) {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B& rhs) {
            if constexpr (std::is_same_v<A, std::remove_cvref_t<B>>) {
              new (const_cast<std::remove_cvref_t<A>*>(std::addressof(lhs)))
                  std::remove_cvref_t<A>(std::move(rhs));
            }
          },
          *this, other);
    }
    this->holding_index = other.index();
  }


  template <typename T,
            typename T_j = decltype(variant_impl::converting_ctor_getter<T, Types...>::imaginary_func(std::declval<T&&>()))>
  constexpr variant(T&& t) noexcept(std::is_nothrow_constructible_v<T_j, T>)
      requires(sizeof...(Types) > 0 &&
               !std::is_same_v<std::remove_cvref_t<T>, variant<Types...>> &&
               !(std::is_same_v<std::remove_cvref_t<T>, in_place_type_t<Types>> || ...) &&
               !variant_impl::is_in_place_index_t_specialization<std::remove_cvref_t<T>>::value &&
               ConstructibleFrom<T_j, T>)
      : base(in_place_index<variant_impl::index_by_type<T_j, 0, Types...>::index>, std::forward<T>(t))
  {}

  template <typename T, typename... Args>
  constexpr explicit variant(in_place_type_t<T>, Args&&... args)
      requires(UniqueEntry<T, Types...> && ConstructibleFrom<T, Args...>)
      : base(in_place_index<variant_impl::index_by_type<T, 0, Types...>::index>, std::forward<Args>(args)...)
  {}

  template <std::size_t Id, typename... Args>
  constexpr explicit variant(in_place_index_t<Id>, Args&&... args)
      requires(InBound<Id, Types...> &&
               ConstructibleFrom<typename variant_impl::alternative_by_index<Id, Types...>::type, Args...>)
      : base(in_place_index<Id>, std::forward<Args>(args)...)
  {}


  constexpr ~variant() = default;


  constexpr variant& operator=(variant const& rhs) = delete;
  constexpr variant& operator=(variant const& rhs) requires(TriviallyCopyAssignable<Types...>) = default;
  constexpr variant& operator=(variant const& rhs)
      requires(CopyAssignable<Types...> && !TriviallyCopyAssignable<Types...>) {
    if (rhs.valueless_by_exception()) {
      if (!valueless_by_exception()) {
        this->destroy();
      }
      return *this;
    }
    if (index() == rhs.index()) {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B& rhs) {
            if constexpr (std::is_same_v<A, std::remove_cvref_t<B>>) {
              lhs = rhs;
            }
          },
          *this, rhs);
      return *this;
    }
    this->destroy();
    try {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B&& rhs) {
            if constexpr (std::is_same_v<A, std::remove_cvref_t<B>>) {
              new (const_cast<std::remove_cvref_t<A>*>(std::addressof(lhs)))
                  std::remove_cvref_t<A>(std::forward<B>(rhs));
            }
          },
          *this, rhs);
    } catch (...) {
      this->holding_index = variant_npos;
      throw;
    }
    this->holding_index = rhs.holding_index;
    return *this;
  }


  constexpr variant& operator=(variant&& rhs) = delete;
  constexpr variant& operator=(variant&& rhs) requires(TriviallyMoveAssignable<Types...>) = default;
  constexpr variant& operator=(variant&& rhs)
      noexcept(NothrowMoveAssignable<Types...> && NothrowMoveConstructible<Types...>)
      requires(MoveAssignable<Types...> && !TriviallyMoveAssignable<Types...>) {
    if (rhs.valueless_by_exception()) {
      if (!valueless_by_exception()) {
        this->destroy();
      }
      return *this;
    }
    if (index() == rhs.index()) {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B& rhs) {
            if constexpr (std::is_same_v<A, std::remove_cvref_t<B>>) {
              lhs = std::move(rhs);
            }
          },
          *this, rhs);
      return *this;
    }
    this->destroy();
    try {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B& rhs) {
            if constexpr (std::is_same_v<A, std::remove_cvref_t<B>>) {
              new (const_cast<std::remove_cvref_t<A>*>(std::addressof(lhs)))
                  std::remove_cvref_t<A>(std::move(rhs));
            }
          },
          *this, rhs);
    } catch (...) {
      this->holding_index = variant_npos;
      throw;
    }
    this->holding_index = rhs.holding_index;
    return *this;
  }

  template <typename T,
      typename T_j = decltype(variant_impl::converting_ctor_getter<T, Types...>::imaginary_func(std::declval<T&&>())),
      std::size_t J = variant_impl::index_by_type<T_j, 0, Types...>::index>
  constexpr variant& operator=(T&& t)
      noexcept(std::is_nothrow_assignable_v<T_j&, T> && std::is_nothrow_constructible_v<T_j, T>)
      requires(!std::is_same_v<std::remove_cvref_t<T>, variant<Types...>> &&
          std::is_assignable_v<T_j&, T> &&
          std::is_constructible_v<T_j, T>) {
    if (holds_alternative<T_j>(*this)) {
      get<J>(*this) = std::forward<T>(t);
      this->holding_index = J;
      return *this;
    }
    if (std::is_nothrow_constructible_v<T_j, T> || !std::is_nothrow_move_constructible_v<T_j>) {
      this->emplace<J>(std::forward<T>(t));
      this->holding_index = J;
      return *this;
    }
    this->emplace<J>(T_j(std::forward<T>(t)));
    this->holding_index = J;
    return *this;
  }


  constexpr std::size_t index() const noexcept {
    return this->holding_index;
  }

  constexpr bool valueless_by_exception() const noexcept {
    return this->holding_index == variant_npos;
  }

  template <typename T, typename... Args>
  requires(UniqueEntry<T, Types...> && ConstructibleFrom<T, Args...>)
  constexpr T& emplace(Args&&... args) {
    return emplace<variant_impl::index_by_type<T, 0, Types...>::index>(std::forward<Args>(args)...);
  }

  template <std::size_t Id, typename... Args>
  requires(InBound<Id, Types...> && ConstructibleFrom<typename variant_impl::alternative_by_index<Id, Types...>::type, Args...>)
  constexpr variant_alternative_t<Id, variant>& emplace(Args&&... args) {
    this->destroy();
    try {
      this->holding_index = Id;
      variant_impl::internal_visit(
          [&]<typename T>(T& alt) {
            if constexpr (std::is_constructible_v<std::remove_cvref_t<T>, Args...>) {
              new (const_cast<std::remove_cvref_t<T>*>(std::addressof(alt)))
                  std::remove_cvref_t<T>(std::forward<Args>(args)...);
            }
          },
          *this);
      return get<Id>(this->storage);
    } catch (...) {
      this->holding_index = variant_npos;
      throw;
    }
  }

  constexpr void swap(variant& rhs)
      noexcept(NothrowMoveConstructible<Types...> && NothrowSwappable<Types...>) {
    if (valueless_by_exception() && rhs.valueless_by_exception()) {
      return;
    }
    if (index() == rhs.index()) {
      variant_impl::internal_visit(
          []<typename A, typename B>(A& lhs, B& rhs) {
            if constexpr (std::is_same_v<A, B>) {
              using std::swap;
              swap(lhs, rhs);
            }
          },
          *this, rhs);
      return;
    }
    if (valueless_by_exception()) {
      *this = std::move(rhs);
      rhs.destroy();
      return;
    }
    if (rhs.valueless_by_exception()) {
      rhs = std::move(*this);
      this->destroy();
      return;
    }
    auto tmp(std::move(*this));
    *this = std::move(rhs);
    rhs = std::move(tmp);
  }
};

template <typename Visitor, typename... Variants>
constexpr decltype(auto) visit(Visitor&& vis, Variants&&... vars)
    requires(variant_impl::is_variant_specialization<std::remove_cvref_t<Variants>>::value && ...) {
  if ((std::forward<Variants>(vars).valueless_by_exception() || ...)) {
    throw bad_variant_access("invoke visit on valueless variant");
  }
  using R = decltype(std::forward<Visitor>(vis)(get<0>(std::forward<Variants>(vars))...));
  return visit<R>(std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
}


template <typename R, typename Visitor, typename... Variants>
constexpr R visit(Visitor&& vis, Variants&&... vars)
    requires(variant_impl::is_variant_specialization<std::remove_cvref_t<Variants>>::value && ...) {
  if ((std::forward<Variants>(vars).valueless_by_exception() || ...)) {
    throw bad_variant_access("invoke visit on valueless variant");
  }
  return variant_impl::invoker<R, Visitor, Variants...>
      ::invoke(std::forward<Visitor>(vis), std::forward<Variants>(vars)...);
}


template <typename T, typename... Types>
constexpr bool holds_alternative(variant<Types...> const& v) noexcept {
  return v.index() == variant_impl::index_by_type<T, 0, Types...>::index;
}


template <std::size_t Id, typename... Types>
constexpr variant_alternative_t<Id, variant<Types...>>& get(variant<Types...>& v) {
  if (v.index() == Id) {
    return get<Id>(v.storage);
  }
  throw bad_variant_access("accessing non-holding alternative");
}

template <std::size_t Id, typename... Types>
constexpr variant_alternative_t<Id, variant<Types...>>&& get(variant<Types...>&& v) {
  if (v.index() == Id) {
    return std::move(get<Id>(std::move(v.storage)));
  }
  throw bad_variant_access("accessing non-holding alternative");
}

template <std::size_t Id, typename... Types>
constexpr const variant_alternative_t<Id, variant<Types...>>& get(const variant<Types...>& v) {
  if (v.index() == Id) {
    return get<Id>(v.storage);
  }
  throw bad_variant_access("accessing non-holding alternative");
}

template <std::size_t Id, typename... Types>
constexpr const variant_alternative_t<Id, variant<Types...>>&& get(const variant<Types...>&& v) {
  if (v.index() == Id) {
    return std::move(get<Id>(std::move(v.storage)));
  }
  throw bad_variant_access("accessing non-holding alternative");
}


template <typename T, typename... Types>
constexpr T& get(variant<Types...>& v) {
  if (holds_alternative<T>(v)) {
    return get<variant_impl::index_by_type<T, 0, Types...>::index>(v);
  }
  throw bad_variant_access("accessing non-holding alternative");
}

template <typename T, typename... Types>
constexpr T&& get(variant<Types...>&& v) {
  if (holds_alternative<T>(v)) {
    return std::move(get<variant_impl::index_by_type<T, 0, Types...>::index>(std::move(v)));
  }
  throw bad_variant_access("accessing non-holding alternative");
}


template <typename T, typename... Types>
constexpr const T& get(const variant<Types...>& v) {
  if (holds_alternative<T>(v)) {
    return get<variant_impl::index_by_type<T, 0, Types...>::index>(v);
  }
  throw bad_variant_access("accessing non-holding alternative");
}

template <typename T, typename... Types>
constexpr const T&& get( const std::variant<Types...>&& v ) {
  if (holds_alternative<T>(v)) {
    return std::move(get<variant_impl::index_by_type<T, 0, Types...>::index>(std::move(v)));
  }
  throw bad_variant_access("accessing non-holding alternative");
}


template <std::size_t Id, typename... Types>
constexpr std::add_pointer_t<variant_alternative_t<Id, variant<Types...>>>
get_if(variant<Types...>* pv) noexcept {
  if (pv != nullptr && pv->index() == Id) {
    return std::addressof(get<Id>(pv->storage));
  }
  return nullptr;
}

template <std::size_t Id, typename... Types>
constexpr std::add_pointer_t<const variant_alternative_t<Id, variant<Types...>>>
get_if(const variant<Types...>* pv) noexcept {
  if (pv != nullptr && pv->index() == Id) {
    return std::addressof(get<Id>(pv->storage));
  }
  return nullptr;
}


template <typename T, typename... Types>
constexpr std::add_pointer_t<T> get_if(variant<Types...>* pv) noexcept {
  return get_if<variant_impl::index_by_type<T, 0, Types...>::index>(pv);
}

template <typename T, typename... Types>
constexpr std::add_pointer_t<const T> get_if(const variant<Types...>* pv) noexcept {
  return get_if<variant_impl::index_by_type<T, 0, Types...>::index>(pv);
}


template <typename... Types>
constexpr bool operator==(const variant<Types...>& v, const variant<Types...>& w) {
  if (v.index() != w.index()) {
    return false;
  }
  if (v.valueless_by_exception()) {
    return true;
  }
  return visit([]<typename A, typename B>(A const& lhs, B const& rhs) -> bool {
    if constexpr (std::is_same_v<A, B>) {
      return lhs == rhs;
    } else {
      return false;
    }
  }, v, w);
}

template <typename... Types>
constexpr bool operator!=(const variant<Types...>& v, const variant<Types...>& w) {
  return !(v == w);
}

template <typename... Types>
constexpr bool operator<(const variant<Types...>& v, const variant<Types...>& w) {
  if (w.valueless_by_exception()) {
    return false;
  }
  if (v.valueless_by_exception()) {
    return true;
  }
  if (v.index() < w.index()) {
    return true;
  }
  if (v.index() > w.index()) {
    return false;
  }

  return visit([]<typename A, typename B>(A const& lhs, B const& rhs) -> bool {
    if constexpr (std::is_same_v<A, B>) {
      return lhs < rhs;
    } else {
      return false;
    }
  }, v, w);
}

template <typename... Types>
constexpr bool operator>(const variant<Types...>& v, const variant<Types...>& w) {
  if (v.valueless_by_exception()) {
    return false;
  }
  if (w.valueless_by_exception()) {
    return true;
  }
  if (v.index() > w.index()) {
    return true;
  }
  if (v.index() < w.index()) {
    return false;
  }

  return visit([]<typename A, typename B>(A const& lhs, B const& rhs) -> bool {
    if constexpr (std::is_same_v<A, B>) {
      return lhs > rhs;
    } else {
      return false;
    }
  }, v, w);
}

template <typename... Types>
constexpr bool operator<=(const variant<Types...>& v, const variant<Types...>& w) {
  if (v.valueless_by_exception()) {
    return true;
  }
  if (w.valueless_by_exception()) {
    return false;
  }
  if (v.index() < w.index()) {
    return true;
  }
  if (v.index() > w.index()) {
    return false;
  }

  return visit([]<typename A, typename B>(A const& lhs, B const& rhs) -> bool {
    if constexpr (std::is_same_v<A, B>) {
      return lhs <= rhs;
    } else {
      return false;
    }
  }, v, w);
}

template <typename... Types>
constexpr bool operator>=(const variant<Types...>& v, const variant<Types...>& w) {
  if (w.valueless_by_exception()) {
    return true;
  }
  if (v.valueless_by_exception()) {
    return false;
  }
  if (v.index() > w.index()) {
    return true;
  }
  if (v.index() < w.index()) {
    return false;
  }

  return visit([]<typename A, typename B>(A const& lhs, B const& rhs) -> bool {
    if constexpr (std::is_same_v<A, B>) {
      return lhs >= rhs;
    } else {
      return false;
    }
  }, v, w);
}


template <typename... Types>
constexpr void swap(variant<Types...>& lhs, variant<Types...>& rhs) noexcept(noexcept(lhs.swap(rhs)))
    requires(MoveConstructible<Types...> && Swappable<Types...>) {
  lhs.swap(rhs);
}

