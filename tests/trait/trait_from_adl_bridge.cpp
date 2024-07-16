
#include <catch2/catch_test_macros.hpp>
#include <extra/trait.hpp>

namespace domain
{
  struct to_string;
  struct validate;
} // namespace domain

namespace client
{
  struct ignorant
  {};

  enum class target_enum
  {
    first,
    second
  };

  struct target_enum_ext
  {
    template <typename...>
    struct trait;
  };

  template <>
  struct target_enum_ext::trait<domain::to_string>
  {
    constexpr char const* operator()(target_enum e) const noexcept
    {
      switch (e)
      {
        case target_enum::first:
          return "first";
        case target_enum::second:
          return "second";
        default:
          return ""; // std::unreachable();
      }
    }
  };

  template <>
  struct target_enum_ext::trait<domain::validate>
  {
    constexpr bool operator()(target_enum e) const noexcept
    {
      switch (e)
      {
        case target_enum::first:
        case target_enum::second:
          return true;
        default:
          return false;
      }
    }
  };

  // deduction guide for trait_impl
  // (target_enum::trait<Tag> -> target_enum_ext::trait<Tag>)
  auto trait_impl(std::type_identity<target_enum>)
    -> std::type_identity<target_enum_ext>;

} // namespace client

TEST_CASE("Trait from the adl bridge", "[trait]")
{
  using namespace std::string_view_literals;
  using namespace client;
  using namespace domain;

  static_assert(extra::has_trait<target_enum, validate>);
  static_assert(not extra::has_trait<ignorant, validate>);

  constexpr target_enum e = target_enum::first;
  static_assert(extra::trait<validate>(e));

  constexpr auto str = extra::trait<to_string>(e);
  static_assert("first"sv == str);
}
