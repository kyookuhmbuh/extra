
#pragma once

#include <variant> // IWYU pragma: export

namespace extra
{
  template <typename... Callable>
  struct overload : Callable...
  {
    using Callable::operator()...;

    template <class...>
    static inline constexpr bool always_false_v = false;

    template <typename T>
    constexpr void operator()(T) const
    {
      static_assert(always_false_v<T>, "Unsupported type");
    }
  };
} // namespace extra
