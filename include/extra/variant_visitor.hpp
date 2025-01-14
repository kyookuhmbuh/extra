
#pragma once

#include <concepts>
#include <utility>
#include <variant>

namespace extra
{

  /// Concept that checks whether a callable `T` can be invoked with
  /// every alternative of a `std::variant`.
  ///
  /// This is useful for constraining visitor objects for `std::visit`,
  /// ensuring that the callable can handle all types contained in the variant.
  ///
  /// @tparam T       Callable type to check.
  /// @tparam Variant Variant type whose alternatives must be accepted.
  ///
  /// Example:
  ///
  /// ```cpp
  /// using my_variant = std::variant<int, double, std::string>;
  ///
  /// auto visitor = [](auto value)
  /// {
  ///     std::cout << value << "\n";
  /// };
  ///
  /// static_assert(extra::variant_visitor<decltype(visitor), my_variant>);
  ///
  /// my_variant v = 42;
  /// std::visit(visitor, v); // safe, visitor handles all alternatives
  /// ```
  template <typename T, typename Variant>
  concept variant_visitor = []<std::size_t... I>(std::index_sequence<I...>)
  {
    return (std::invocable<T, std::variant_alternative_t<I, Variant>> and ...);
  }(std::make_index_sequence<std::variant_size_v<Variant>>{});

} // namespace extra
