
#pragma once

#include <concepts>
#include <format>
#include <iostream>
#include <source_location>
#include <syncstream>
#include <type_traits>
#include <utility>

namespace extra
{
  namespace detail
  {
    /// Wrapper combining a format string and its source location.
    ///
    /// Used internally by debug_print to capture the call site.
    template <typename... Args>
    struct format_string_with_location
    {
      std::format_string<Args...> value;    ///< Format string
      std::source_location        location; ///< Source location of the call

      /// Construct from a format string and optional source location.
      ///
      /// @tparam U Type convertible to std::format_string<Args...>.
      /// @param format Format string.
      /// @param location Source location (defaults to current location).
      template <typename U>
        requires std::constructible_from<std::format_string<Args...>, U&&>
      consteval explicit(false) format_string_with_location(
        U&&                         format,
        std::source_location const& location = std::source_location::current())
        : value(std::forward<U>(format))
        , location(location)
      {}
    };

    /// Core implementation of debug_print.
    ///
    /// Formats the string and prints it to std::cout with source location info.
    template <typename... Args>
    void debug_print_core(
      std::source_location        source_location,
      std::format_string<Args...> format,
      Args&&... args)
    {
      constexpr std::size_t         buffer_size = 2048;
      std::array<char, buffer_size> buffer{};

      auto const result = std::format_to_n(
        buffer.begin(),
        buffer.size(),
        format,
        std::forward<Args>(args)...);
      *result.out = '\0';

      std::osyncstream{ std::cout }
        << source_location.file_name() << ":" << source_location.line() << ": "
        << std::string_view{ buffer.begin(), result.out };
    }
  } // namespace detail

  /// Print a debug message with source location info.
  ///
  /// @param fmt Format string wrapped in format_string_with_location.
  /// @param args Arguments to format.
  ///
  /// Example:
  ///
  /// ```cpp
  /// extra::debug_print("Value: {}\n", 42);
  /// ```
  template <typename... Args>
  void debug_print(
    std::type_identity_t<detail::format_string_with_location<Args...>> fmt,
    Args&&... args)
  {
    detail::debug_print_core(
      fmt.location,
      fmt.value,
      std::forward<Args>(args)...);
  }

  /// Conditionally print a debug message.
  ///
  /// @param enabled If false, the message is not printed.
  /// @param fmt Format string wrapped in format_string_with_location.
  /// @param args Arguments to format.
  ///
  /// Example:
  ///
  /// ```cpp
  /// bool verbose = true;
  /// extra::debug_print_if(verbose, "Value: {}\n", 42);
  /// ```
  template <typename... Args>
  void debug_print_if(
    bool                                                               enabled,
    std::type_identity_t<detail::format_string_with_location<Args...>> fmt,
    Args&&... args)
  {
    if (enabled)
    {
      debug_print(fmt, std::forward<Args>(args)...);
    }
  }

  /// RAII guard that prints scope entry and exit messages.
  ///
  /// Can be used to trace function or scope execution in debug builds.
  struct scope_trace_guard
  {
    std::string          scope_name;      ///< Name of the scope
    bool                 enabled;         ///< Whether tracing is enabled
    std::source_location source_location; ///< Captured source location

    /// Construct and print the entry message.
    ///
    /// @param scope_name Name of the scope (optional).
    /// @param enabled Whether tracing is enabled (default true).
    /// @param source_location Source location (defaults to current location).
    [[nodiscard]] explicit scope_trace_guard(
      std::string_view     scope_name      = "",
      bool                 enabled         = true,
      std::source_location source_location = std::source_location::current())
      : scope_name{ scope_name }
      , enabled{ enabled }
      , source_location{ source_location }
    {
      if (enabled)
      {
        detail::debug_print_core(
          source_location,
          "ENTER |{}| {}\n",
          scope_name.empty() ? "function" : scope_name,
          source_location.function_name());
      }
    }

    scope_trace_guard(scope_trace_guard const&)            = delete;
    scope_trace_guard(scope_trace_guard&&)                 = delete;
    scope_trace_guard& operator=(scope_trace_guard const&) = delete;
    scope_trace_guard& operator=(scope_trace_guard&&)      = delete;

    /// Destructor prints the exit message.
    ~scope_trace_guard()
    {
      if (enabled)
      {
        detail::debug_print_core(
          source_location,
          "EXIT  |{}| {}\n",
          scope_name.empty() ? "function" : scope_name,
          source_location.function_name());
      }
    }
  };

} // namespace extra
