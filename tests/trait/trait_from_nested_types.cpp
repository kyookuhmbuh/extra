
#include <catch2/catch_test_macros.hpp>
#include <extra/trait.hpp>

namespace domain
{
  struct get_value
  {
    // inject name
    template <typename...>
    struct trait_impl;

    // default impl
    template <typename T>
    struct trait_impl<T>
    {
      constexpr auto operator()(T const&) const noexcept
      {
        return 0;
      }
    };

    // delegating impl
    template <extra::has_trait<get_value> T>
    struct trait_impl<std::optional<T>>
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
    struct trait_impl;

    template <>
    struct trait_impl<domain::get_value>
    {
      constexpr auto operator()(target const& t) const noexcept
      {
        return t.value;
      }
    };
  };

  // impl get_value for target
  template <>
  struct target::trait_impl<domain::set_value>
  {
    constexpr void operator()(target& t, int value) const noexcept
    {
      t.value = value;
    }
  };

} // namespace client

TEST_CASE("Check trait implemenations", "[trait]")
{
  auto instance = client::target{ 12 };

  SECTION("Invoke the trait")
  {
    auto trait = extra::trait<domain::get_value, client::target>;
    auto value = trait(instance);
    REQUIRE(12 == value);
  }

  SECTION("Invoke the trait with a target type deducing")
  {
    extra::trait<domain::set_value>(instance, 881);
    auto value = extra::trait<domain::get_value>(instance);
    REQUIRE(881 == value);
  }

  SECTION("Invoke the default implementation")
  {
    REQUIRE(extra::has_trait<client::ignorant, domain::get_value>);
    REQUIRE(not extra::has_trait<client::ignorant, domain::set_value>);

    auto value = extra::trait<domain::get_value>(client::ignorant{});
    REQUIRE(0 == value);
  }

  SECTION("Invoke the delegating implementation")
  {
    REQUIRE(extra::has_trait<client::target, domain::get_value>);
    REQUIRE(extra::has_trait<std::optional<client::target>, domain::get_value>);

    std::optional<client::target> opt_null{};
    auto value_from_opt_null = extra::trait<domain::get_value>(opt_null);
    REQUIRE(0 == value_from_opt_null);

    auto value1 = extra::trait<domain::get_value>(std::as_const(instance));
    auto value2 = extra::trait<domain::get_value>(std::optional(instance));
    REQUIRE(12 == value1);
    REQUIRE(12 == value2);
  }
}
