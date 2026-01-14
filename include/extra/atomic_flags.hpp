
#pragma once

#include <atomic>
#include <concepts>
#include <type_traits>

namespace extra
{
  /// Atomic wrapper for enum or unsigned integral flags.
  ///
  /// Provides atomic operations for setting, clearing, toggling, and querying
  /// bit flags in a thread-safe manner. Supports both enum types and unsigned
  /// integral types.
  ///
  /// @tparam Flags Enum or unsigned integral type representing bit flags.
  ///
  /// Example:
  ///
  /// ```cpp
  /// enum class MyFlags : unsigned int { A = 1, B = 2, C = 4 };
  ///
  /// extra::atomic_flags<MyFlags> flags;
  /// flags.set(MyFlags::A);
  /// if (flags.has_all(MyFlags::A))
  ///     std::cout << "Flag A is set\n";
  ///
  /// flags.toggle(MyFlags::A);
  /// if (flags.is_empty())
  ///     std::cout << "All flags cleared\n";
  /// ```
  template <typename Flags>
    requires(std::is_enum_v<Flags> or std::unsigned_integral<Flags>)
  class atomic_flags
  {
  private:
    /// Helper to determine underlying type of Flags.
    template <typename U>
    struct get_underlying_type;

    /// Specialization for enum types.
    template <typename U>
      requires std::is_enum_v<U>
    struct get_underlying_type<U> : std::underlying_type<U>
    {};

    /// Specialization for unsigned integral types.
    template <std::unsigned_integral U>
    struct get_underlying_type<U> : std::type_identity<U>
    {};

  public:
    /// User-facing flag type.
    using value_type      = Flags;
    /// Storage type.
    using underlying_type = typename get_underlying_type<Flags>::type;

    /// Default constructor, initializes flags to zero.
    [[nodiscard]] atomic_flags() noexcept = default;

    /// Construct from an initial flag value.
    ///
    /// @param init Initial flag value.
    [[nodiscard]] explicit atomic_flags(Flags init) noexcept
      : value_(static_cast<underlying_type>(init))
    {}

    /// Check whether all flags are cleared.
    ///
    /// @return true if no flags are set, false otherwise.
    [[nodiscard]] bool is_empty() const noexcept
    {
      return value_.load(std::memory_order_relaxed) == 0;
    }

    /// Check whether all specified flags are set.
    ///
    /// @param flags Flags to check.
    /// @return true if all specified flags are set, false otherwise.
    [[nodiscard]] bool has_all(Flags flags) const noexcept
    {
      auto mask  = static_cast<underlying_type>(flags);
      auto value = value_.load(std::memory_order_relaxed);
      return (value & mask) == mask;
    }

    /// Check whether any of the specified flags are set.
    ///
    /// @param flags Flags to check.
    /// @return true if at least one specified flag is set, false otherwise.
    [[nodiscard]] bool has_any(Flags flags) const noexcept
    {
      auto mask  = static_cast<underlying_type>(flags);
      auto value = value_.load(std::memory_order_relaxed);
      return (value & mask) != 0;
    }

    /// Atomically set the specified flags.
    ///
    /// @param flags Flags to set.
    void set(Flags flags) noexcept
    {
      auto mask = static_cast<underlying_type>(flags);
      value_.fetch_or(mask, std::memory_order_relaxed);
    }

    /// Atomically clear the specified flags.
    ///
    /// @param flags Flags to clear.
    void clear(Flags flags) noexcept
    {
      auto mask = ~static_cast<underlying_type>(flags);
      value_.fetch_and(mask, std::memory_order_relaxed);
    }

    /// Clear all flags.
    void clear_all() noexcept
    {
      value_.store(0, std::memory_order_relaxed);
    }

    /// Atomically toggle the specified flags.
    ///
    /// @param flags Flags to toggle.
    void toggle(Flags flags) noexcept
    {
      auto mask = static_cast<underlying_type>(flags);
      value_.fetch_xor(mask, std::memory_order_relaxed);
    }

  private:
    std::atomic<underlying_type> value_{}; ///< Underlying atomic storage
  };
} // namespace extra
