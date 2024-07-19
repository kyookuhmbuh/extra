
#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace extra
{
  template <typename Callable, typename Tuple>
  constexpr bool tuple_visit(Callable&& visitor, Tuple&& tuple)
  {
    auto wrapper = [&visitor]<typename Arg>(Arg&& arg) -> bool
    {
      if constexpr (std::predicate<Callable, Arg>)
      {
        return std::invoke(std::forward<Callable>(visitor),
                           std::forward<Arg>(arg));
      }
      else if constexpr (std::invocable<Callable, Arg>)
      {
        std::invoke(std::forward<Callable>(visitor), std::forward<Arg>(arg));
        return false;
      }
      else
      {
        static_assert(std::invocable<Callable, Arg>);
        return false;
      }
    };

    return std::apply(
      [&wrapper]<typename... Args>(Args&&... args)
      {
        return (not wrapper(std::forward<Args>(args)) and ...);
      },
      std::forward<Tuple>(tuple));
  }

  template <typename Ret, typename Callable, typename Tuple>
  constexpr std::optional<Ret> tuple_visit(Callable&& visitor, Tuple&& tuple)
  {
    auto wrapper = [&visitor]<typename Arg>(Arg&& arg) -> std::optional<Ret>
    {
      if constexpr (std::is_invocable_r_v<std::optional<Ret>, Callable, Arg> or
                    std::is_invocable_r_v<Ret, Callable, Arg>)
      {
        return std::invoke(std::forward<Callable>(visitor),
                           std::forward<Arg>(arg));
      }
      else if constexpr (std::invocable<Callable, Arg>)
      {
        std::invoke(std::forward<Callable>(visitor), std::forward<Arg>(arg));
        return std::nullopt;
      }
      else
      {
        static_assert(std::invocable<Callable, Arg>);
        return std::nullopt;
      }
    };

    return std::apply(
      [&wrapper]<typename... Args>(Args&&... args)
      {
        std::optional<Ret> result{};
        ((result = wrapper(std::forward<Args>(args)), not result) and ...);
        return result;
      },
      std::forward<Tuple>(tuple));
  }
} // namespace extra
