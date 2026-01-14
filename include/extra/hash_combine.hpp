
#pragma once

#include <concepts>
#include <cstddef>
#include <functional>

namespace extra
{
  /// Combine multiple hash values into a single hash.
  ///
  /// This function incrementally mixes the hash values of the provided
  /// arguments into an initial seed, producing a single combined hash.
  ///
  /// The algorithm is based on `boost::hash_combine` and is suitable for
  /// building hash values for composite objects.
  ///
  /// @tparam Hash Unsigned integral type used for the hash value.
  ///              Defaults to std::size_t.
  /// @tparam T    Types of the values to be hashed and combined.
  ///
  /// @param seed   Initial hash value.
  /// @param values Values whose hashes will be combined with the seed.
  ///
  /// @return Combined hash value.
  ///
  /// @note The quality of the result depends on the quality of
  ///       std::hash specializations for the provided types.
  ///
  /// Example:
  ///
  /// ```cpp
  /// struct point
  /// {
  ///   int x;
  ///   int y;
  /// };
  ///
  /// std::size_t hash_point(point const& p)
  /// {
  ///   std::size_t seed = 0;
  ///   return extra::hash_combine(seed, p.x, p.y);
  /// }
  /// ```
  template <std::unsigned_integral Hash = std::size_t, typename... T>
  constexpr Hash hash_combine(Hash const& seed, T const&... values) noexcept
  {
    // Golden ratio constant used to improve bit dispersion.
    constexpr auto magic = static_cast<Hash>(0x9e3779b97f4a7c15ULL);

    auto result = seed;

    auto combine_one = [&](Hash hash) constexpr noexcept
    {
      result ^= hash + magic + (result << 6) + (result >> 2);
    };

    (combine_one(std::hash<std::decay_t<T>>{}(values)), ...);

    return result;
  }

} // namespace extra
