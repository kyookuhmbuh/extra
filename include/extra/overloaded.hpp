
#pragma once

namespace extra
{

  /// Helper type for creating an overloaded callable from multiple callables.
  ///
  /// This utility is primarily intended for use with `std::visit` and similar
  /// facilities that require a single callable object with multiple
  /// `operator()` overloads.
  ///
  /// The resulting type inherits from all provided callable types and brings
  /// their `operator()` members into scope.
  ///
  /// @tparam Ts Callable types to be combined.
  ///
  /// Example:
  ///
  /// ```cpp
  /// std::variant<int, std::string> v = "text";
  ///
  /// std::visit(
  ///   extra::overloaded{
  ///     [](int value)        { /* handle int */ },
  ///     [](std::string_view) { /* handle string */ }
  ///   },
  ///   v
  /// );
  /// ```
  template <typename... Ts>
  struct overloaded : Ts...
  {
    /// Expose all operator() overloads from base callables.
    using Ts::operator()...;
  };

  /// Deduction guide for overloaded.
  ///
  /// Allows `extra::overloaded` to be constructed without explicitly specifying
  /// template arguments.
  ///
  /// @tparam Ts Callable types to be combined.
  template <typename... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

} // namespace extra
