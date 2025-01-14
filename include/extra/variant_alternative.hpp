
#pragma once

#include <concepts>
#include <utility>
#include <variant>

namespace extra
{

  /// Concept that checks whether a type `T` is one of the alternatives
  /// of a `std::variant`.
  ///
  /// This is useful for constraining templates or functions that should
  /// only accept types present in a given `std::variant`.
  ///
  /// @tparam T       Type to check.
  /// @tparam Variant Variant type to inspect.
  ///
  /// Example:
  ///
  /// ```cpp
  /// using my_variant = std::variant<int, double, std::string>;
  ///
  /// static_assert(extra::variant_alternative<int, my_variant>);
  /// static_assert(extra::variant_alternative<std::string, my_variant>);
  /// static_assert(not extra::variant_alternative<float, my_variant>);
  ///
  /// template <extra::variant_alternative<my_variant> T>
  /// void process(T value) { /* ... */ }
  /// ```
  template <typename T, typename Variant>
  concept variant_alternative = []<std::size_t... I>(std::index_sequence<I...>)
  {
    return (std::same_as<T, std::variant_alternative_t<I, Variant>> or ...);
  }(std::make_index_sequence<std::variant_size_v<Variant>>{});

} // namespace extra
