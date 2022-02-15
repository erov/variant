#pragma once

#include <concepts>
#include <type_traits>


template <typename... Types>
concept Destructible = (std::is_destructible_v<Types> && ...);

template <typename... Types>
concept TriviallyDestructible = (std::is_trivially_destructible_v<Types> && ...);

template <typename... Types>
concept NothrowDefaultDestructible = (std::is_nothrow_destructible_v<Types> && ...);


template <typename... Types>
concept DefaultConstructible = (std::is_default_constructible_v<Types> && ...);

template <typename... Types>
concept TriviallyConstructible = (std::is_trivially_default_constructible_v<Types> && ...);

template <typename... Types>
concept NothrowConstructible = (std::is_nothrow_default_constructible_v<Types> && ...);


template <typename... Types>
concept CopyConstructible = (std::is_copy_constructible_v<Types> && ...);

template <typename... Types>
concept TriviallyCopyConstructible = (std::is_trivially_copy_constructible_v<Types> && ...);

template <typename... Types>
concept NothrowCopyConstructible = (std::is_nothrow_copy_constructible_v<Types> && ...);


template <typename... Types>
concept MoveConstructible = (std::is_move_constructible_v<Types> && ...);

template <typename... Types>
concept TriviallyMoveConstructible = (std::is_trivially_move_constructible_v<Types> && ...);

template <typename... Types>
concept NothrowMoveConstructible = (std::is_nothrow_move_constructible_v<Types> && ...);


template <typename... Types>
concept CopyAssignable = (std::is_copy_assignable_v<Types> && ...) &&
                         CopyConstructible<Types...>;

template <typename... Types>
concept TriviallyCopyAssignable = (std::is_trivially_copy_assignable_v<Types> && ...) &&
                                  TriviallyCopyConstructible<Types...> &&
                                  TriviallyDestructible<Types...>;

template <typename... Types>
concept NothrowCopyAssignable = (std::is_nothrow_copy_assignable_v<Types> && ...) &&
                                NothrowCopyConstructible<Types...>;


template <typename... Types>
concept MoveAssignable = (std::is_move_assignable_v<Types> && ...) &&
                         MoveConstructible<Types...>;

template <typename... Types>
concept TriviallyMoveAssignable = (std::is_trivially_move_assignable_v<Types> && ...) &&
                                  TriviallyMoveConstructible<Types...> &&
                                  TriviallyDestructible<Types...>;

template <typename... Types>
concept NothrowMoveAssignable = (std::is_nothrow_move_assignable_v<Types> && ...) &&
                                NothrowMoveConstructible<Types...>;


template <typename... Types>
concept NothrowSwappable = (std::is_nothrow_swappable_v<Types> && ...);

template <typename... Types>
concept Swappable = (std::is_swappable_v<Types> && ...);


template <typename T, typename... Types>
concept UniqueEntry =
    ((static_cast<std::size_t>(std::is_same_v<T, Types>) + ...) == 1);

template <typename T, typename... Args>
concept ConstructibleFrom = (std::is_constructible_v<T, Args...>);

template <std::size_t Id, typename... Types>
concept InBound = (Id < sizeof...(Types));


namespace variant_impl {

template <typename T_i>
struct array_wrapper {
  T_i arr[1];
};

template <typename T_i, typename T>
concept ValidDeclarable = requires(T&& t) {
  array_wrapper<T_i>{std::forward<T>(t)};
};


template <typename T, typename T0, typename... TRest>
struct converting_ctor_getter : converting_ctor_getter<T, TRest>... {
  using converting_ctor_getter<T, TRest>::imaginary_func...;
  constexpr static T0 imaginary_func(T0) requires(ValidDeclarable<T0, T>);
};

template <typename T, typename T_i>
struct converting_ctor_getter<T, T_i> {
  constexpr static T_i imaginary_func(T_i) requires(ValidDeclarable<T_i, T>);
};

}

