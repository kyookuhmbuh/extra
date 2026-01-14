
#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace extra
{

  namespace internal
  {

    /// Helper template to parse a string value into a type `T`.
    ///
    /// Specializations handle strings, integral types, floating-point types,
    /// and enums.
    ///
    /// @tparam T Type to parse.
    template <typename T>
    std::optional<T> parse_value(std::string_view value);

    /// Specialization for std::string.
    template <>
    inline std::optional<std::string> parse_value<std::string>(
      std::string_view value)
    {
      return std::string{ value };
    }

    /// Specialization for integral types.
    template <std::integral Int>
    inline std::optional<Int> parse_value(std::string_view string_value)
    {
      int value{};
      auto [ptr, ec] = std::from_chars(
        string_value.data(),
        string_value.data() + string_value.size(),
        value);
      if (ec == std::errc{})
      {
        return static_cast<Int>(value);
      }
      return std::nullopt;
    }

    /// Specialization for enum types.
    template <typename Enum>
      requires std::is_enum_v<Enum>
    inline std::optional<Enum> parse_value(std::string_view string_value)
    {
      auto raw_value = parse_value<std::underlying_type_t<Enum>>(string_value);
      if (raw_value)
      {
        return static_cast<Enum>(*raw_value);
      }
      return std::nullopt;
    }

    /// Specialization for floating-point types.
    template <std::floating_point Float>
    inline std::optional<Float> parse_value(std::string_view string_value)
    {
      try
      {
        return static_cast<Float>(std::stod(std::string{ string_value }));
      }
      catch (...)
      {
        return std::nullopt;
      }
    }

  } // namespace internal

  // NOLINTBEGIN(modernize-avoid-c-arrays)

  /// Get a command-line argument by name and convert it to type `T`.
  ///
  /// Supports both `--key value` and `--key=value` formats.
  ///
  /// @tparam T Type to convert the argument to.
  /// @param argc Argument count.
  /// @param argv Argument vector.
  /// @param name Name of the argument to find.
  /// @return Optional containing the parsed value, or `std::nullopt` if not found
  ///         or conversion failed.
  ///
  /// Example:
  ///
  /// ```cpp
  /// int main(int argc, char* argv[])
  /// {
  ///     auto port = extra::get_arg<int>(argc, argv, "port");
  ///     if (port)
  ///         std::cout << "Port: " << *port << "\n";
  /// }
  /// ```
  template <typename T>
  [[nodiscard]] std::optional<T> get_arg(
    int              argc,
    char*            argv[],
    std::string_view name)
  {
    std::string key =
      name.starts_with("--") ? std::string{ name } : "--" + std::string{ name };

    for (int i = 1; i < argc; ++i)
    {
      std::string_view arg = argv[i];

      // Формат --key=value
      if (arg.starts_with(key) and arg.find('=') != std::string_view::npos)
      {
        auto pos = arg.find('=');
        auto val = arg.substr(pos + 1);
        return internal::parse_value<T>(val);
      }

      // Формат --key value
      if (arg == key and i + 1 < argc)
      {
        return internal::parse_value<T>(argv[i + 1]);
      }
    }

    return std::nullopt;
  }

  /// Get a command-line argument by name with a default value.
  ///
  /// @tparam T Type to convert the argument to.
  /// @tparam Default Type of the default value.
  /// @param argc Argument count.
  /// @param argv Argument vector.
  /// @param name Name of the argument to find.
  /// @param default_value Value to return if the argument is not found.
  /// @return Parsed value or `default_value` if not found.
  ///
  /// Example:
  ///
  /// ```cpp
  /// int main(int argc, char* argv[])
  /// {
  ///     int port = extra::get_arg<int>(argc, argv, "port", 8080);
  ///     std::cout << "Port: " << port << "\n";
  /// }
  /// ```
  template <typename T, typename Default>
  [[nodiscard]] T get_arg(
    int              argc,
    char*            argv[],
    std::string_view name,
    Default&&        default_value)
  {
    return get_arg<T>(argc, argv, name)
      .value_or(std::forward<Default>(default_value));
  }

  // NOLINTEND(modernize-avoid-c-arrays)

} // namespace extra
