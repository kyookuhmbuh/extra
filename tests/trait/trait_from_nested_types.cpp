
#include <catch2/catch_test_macros.hpp>
#include <extra/trait.hpp>

namespace domain
{
  struct get_value
  {
    // inject name
    template <typename...>
    struct trait_for;

    // default impl
    template <typename T>
    struct trait_for<T>
    {
      constexpr auto operator()(T const&) const noexcept
      {
        return 0;
      }
    };

    // delegating impl
    template <extra::has_trait<get_value> T>
    struct trait_for<std::optional<T>>
    {
      constexpr auto operator()(std::optional<T> const& opt) const noexcept
      {
        if (opt)
        {
          return extra::trait<get_value, T>(*opt);
        }

        return 0;
      }
    };
  };

  struct set_value
  {};
} // namespace domain

namespace client
{
  struct ignorant
  {};

  struct target
  {
    int value;

    // inject name
    template <typename...>
    struct trait;

    template <>
    struct trait<domain::get_value>
    {
      constexpr auto operator()(target const& t) const noexcept
      {
        return t.value;
      }
    };
  };

  // impl get_value for target
  template <>
  struct target::trait<domain::set_value>
  {
    constexpr void operator()(target& t, int value) const noexcept
    {
      t.value = value;
    }
  };

} // namespace client

TEST_CASE("Check trait implemenations", "[trait]")
{
  using namespace client;
  using namespace domain;

  extra::has_trait<get_value> auto instance = target{ 12 };

  SECTION("Invoke the trait")
  {
    auto trait = extra::trait<get_value, target>;
    auto value = trait(instance);
    REQUIRE(12 == value);
  }

  SECTION("Invoke the trait with a target type deducing")
  {
    extra::trait<set_value>(instance, 881);
    auto value = extra::trait<get_value>(instance);
    REQUIRE(881 == value);
  }

  SECTION("Invoke the default implementation")
  {
    REQUIRE(extra::has_trait<ignorant, get_value>);
    REQUIRE(not extra::has_trait<ignorant, set_value>);

    auto value = extra::trait<get_value>(ignorant{});
    REQUIRE(0 == value);
  }

  SECTION("Invoke the delegating implementation")
  {
    REQUIRE(extra::has_trait<target, get_value>);
    REQUIRE(extra::has_trait<std::optional<target>, get_value>);

    std::optional<target> opt_null{};
    auto value_from_opt_null = extra::trait<get_value>(opt_null);
    REQUIRE(0 == value_from_opt_null);

    auto value1 = extra::trait<get_value>(std::as_const(instance));
    auto value2 = extra::trait<get_value>(std::optional(instance));
    REQUIRE(12 == value1);
    REQUIRE(12 == value2);
  }
}
