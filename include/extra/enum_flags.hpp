
#pragma once

#include <type_traits>

namespace extra::flags
{
  /// Type trait used to opt-in enum types for flag semantics.
  ///
  /// Specialize this trait for an enum type to explicitly mark it as a
  /// bitmask/flags enum and enable the provided utilities and operators.
  ///
  /// @tparam Flags Enum type to be treated as flags.
  template <typename Flags>
  struct is_enum_flags : std::false_type
  {};

  /// Convenience variable template for is_enum_flags.
  ///
  /// @tparam Flags Enum type to be queried.
  template <typename Flags>
  inline constexpr bool is_enum_flags_v = is_enum_flags<Flags>::value;

  /// Concept satisfied by enums explicitly marked as flags.
  ///
  /// This concept requires:
  /// - The type to be an enum.
  /// - The enum to be explicitly opted in via is_enum_flags.
  ///
  /// @tparam Flags Enum type representing a set of bit flags.
  template <typename Flags>
  concept enum_flags = std::is_enum_v<Flags> and is_enum_flags_v<Flags>;

  /// Bitwise operators for enum flag types.
  ///
  /// Operators are defined in a separate namespace so they can be
  /// selectively imported and also participate in ADL when brought
  /// into the global namespace.
  namespace bitwise_operators
  {
    /// Bitwise OR operator for flag enums.
    ///
    /// @param left Left-hand operand.
    /// @param right Right-hand operand.
    ///
    /// @return Bitwise OR of both operands.
    template <enum_flags Flags>
    constexpr Flags operator|(Flags left, Flags right) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      return static_cast<Flags>(static_cast<U>(left) | static_cast<U>(right));
    }

    /// Bitwise AND operator for flag enums.
    ///
    /// @param left Left-hand operand.
    /// @param right Right-hand operand.
    ///
    /// @return Bitwise AND of both operands.
    template <enum_flags Flags>
    constexpr Flags operator&(Flags left, Flags right) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      return static_cast<Flags>(static_cast<U>(left) & static_cast<U>(right));
    }

    /// Bitwise XOR operator for flag enums.
    ///
    /// @param left Left-hand operand.
    /// @param right Right-hand operand.
    ///
    /// @return Bitwise XOR of both operands.
    template <enum_flags Flags>
    constexpr Flags operator^(Flags left, Flags right) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      return static_cast<Flags>(static_cast<U>(left) ^ static_cast<U>(right));
    }

    /// Bitwise NOT operator for flag enums.
    ///
    /// @param value Operand to negate.
    ///
    /// @return Bitwise negation of the operand.
    template <enum_flags Flags>
    constexpr Flags operator~(Flags value) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      return static_cast<Flags>(~static_cast<U>(value));
    }

    /// Compound OR assignment for flag enums.
    ///
    /// @param left Left-hand operand to modify.
    /// @param right Right-hand operand.
    ///
    /// @return Reference to the modified left-hand operand.
    template <enum_flags Flags>
    constexpr Flags& operator|=(Flags& left, Flags right) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      left = static_cast<Flags>(static_cast<U>(left) | static_cast<U>(right));
      return left;
    }

    /// Compound AND assignment for flag enums.
    ///
    /// @param left Left-hand operand to modify.
    /// @param right Right-hand operand.
    ///
    /// @return Reference to the modified left-hand operand.
    template <enum_flags Flags>
    constexpr Flags& operator&=(Flags& left, Flags right) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      left = static_cast<Flags>(static_cast<U>(left) & static_cast<U>(right));
      return left;
    }

    /// Compound XOR assignment for flag enums.
    ///
    /// @param left Left-hand operand to modify.
    /// @param right Right-hand operand.
    ///
    /// @return Reference to the modified left-hand operand.
    template <enum_flags Flags>
    constexpr Flags& operator^=(Flags& left, Flags right) noexcept
    {
      using U = std::underlying_type_t<Flags>;
      left = static_cast<Flags>(static_cast<U>(left) ^ static_cast<U>(right));
      return left;
    }
  } // namespace bitwise_operators

  /// Check whether a flag value is empty (no bits set).
  ///
  /// @param value Flag value to test.
  ///
  /// @return true if no flags are set, false otherwise.
  template <enum_flags Flags>
  constexpr bool is_empty(Flags value) noexcept
  {
    return value == Flags{};
  }

  /// Check whether no bits from the given mask are set in the value.
  ///
  /// @param value Flag value to test.
  /// @param mask Mask specifying bits of interest.
  ///
  /// @return true if none of the masked bits are set, false otherwise.
  template <enum_flags Flags>
  constexpr bool is_empty(Flags value, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return (value & mask) == Flags{};
  }

  /// Check whether all bits from the mask are set in the value.
  ///
  /// @param value Flag value to test.
  /// @param mask Mask specifying required bits.
  ///
  /// @return true if all masked bits are set, false otherwise.
  template <enum_flags Flags>
  constexpr bool has_all(Flags value, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return (value & mask) == mask;
  }

  /// Check whether any bit from the mask is set in the value.
  ///
  /// @param value Flag value to test.
  /// @param mask Mask specifying bits of interest.
  ///
  /// @return true if at least one masked bit is set, false otherwise.
  template <enum_flags Flags>
  constexpr bool has_any(Flags value, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return (value & mask) != Flags{};
  }

  /// Set bits specified by the mask.
  ///
  /// @param old Original flag value.
  /// @param mask Bits to set.
  ///
  /// @return New flag value with the specified bits set.
  template <enum_flags Flags>
  constexpr Flags set(Flags old, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return old | mask;
  }

  /// Set bits specified by the mask in place.
  ///
  /// @param output Flag value to modify.
  /// @param mask Bits to set.
  ///
  /// @return Reference to the modified flag value.
  template <enum_flags Flags>
  constexpr Flags& set_inplace(Flags& output, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return output |= mask;
  }

  /// Clear bits specified by the mask.
  ///
  /// @param old Original flag value.
  /// @param mask Bits to clear.
  ///
  /// @return New flag value with the specified bits cleared.
  template <enum_flags Flags>
  constexpr Flags clear(Flags old, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return old & ~mask;
  }

  /// Clear bits specified by the mask in place.
  ///
  /// @param output Flag value to modify.
  /// @param mask Bits to clear.
  ///
  /// @return Reference to the modified flag value.
  template <enum_flags Flags>
  constexpr Flags& clear_inplace(Flags& output, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return output &= ~mask;
  }

  /// Apply a boolean value to the bits specified by the mask.
  ///
  /// If @p value is true, the masked bits are set.
  /// If @p value is false, the masked bits are cleared.
  ///
  /// @param old Original flag value.
  /// @param mask Bits to modify.
  /// @param value Desired state of the masked bits.
  ///
  /// @return New flag value with the mask applied.
  template <enum_flags Flags>
  constexpr Flags apply(Flags old, Flags mask, bool value = true) noexcept
  {
    using namespace bitwise_operators;
    Flags new_value = value ? mask : Flags{};
    return (old & ~mask) | (new_value & mask);
  }

  /// Apply a boolean value to the bits specified by the mask in place.
  ///
  /// @param output Flag value to modify.
  /// @param mask Bits to modify.
  /// @param value Desired state of the masked bits.
  ///
  /// @return Reference to the modified flag value.
  template <enum_flags Flags>
  constexpr Flags& apply_inplace(
    Flags& output,
    Flags  mask,
    bool   value = true) noexcept
  {
    using namespace bitwise_operators;
    Flags new_value = value ? mask : Flags{};
    return output   = (output & ~mask) | (new_value & mask);
  }

  /// Toggle bits specified by the mask.
  ///
  /// @param old Original flag value.
  /// @param mask Bits to toggle.
  ///
  /// @return New flag value with the specified bits toggled.
  template <enum_flags Flags>
  constexpr Flags toggle(Flags old, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return old ^ mask;
  }

  /// Toggle bits specified by the mask in place.
  ///
  /// @param output Flag value to modify.
  /// @param mask Bits to toggle.
  ///
  /// @return Reference to the modified flag value.
  template <enum_flags Flags>
  constexpr Flags& toggle_inplace(Flags& output, Flags mask) noexcept
  {
    using namespace bitwise_operators;
    return output ^= mask;
  }
} // namespace extra::flags

/// Import bitwise operators for flag enums into the global namespace.
///
/// This is required to enable argument-dependent lookup (ADL) for enum
/// types defined outside of the extra::flags namespace.
using namespace extra::flags::bitwise_operators;
