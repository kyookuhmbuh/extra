
#include <catch2/catch_test_macros.hpp>
#include <extra/trait.hpp>

namespace domain
{
  struct get_value
  {};
} // namespace domain

namespace client
{
  struct ignorant
  {};

  struct target
  {
    int value;
  };

} // namespace client

template <>
struct extra::trait_impl<domain::get_value, client::target>
{
  constexpr auto operator()(client::target const& target) const noexcept
  {
    return target.value;
  }
};

TEST_CASE("Invoke the trait from the regular specialization", "[trait]")
{
  using namespace client;
  using namespace domain;

  REQUIRE(extra::with_trait<target, get_value>);
  REQUIRE(not extra::with_trait<ignorant, get_value>);

  auto instance = target{ 12 };

  SECTION("Invoke the trait")
  {
    instance.value = 4;
    auto trait     = extra::trait<get_value, target>;
    auto value     = trait(instance);
    REQUIRE(4 == value);
  }

  SECTION("Invoke the trait with a target type deducing")
  {
    instance.value = 84;
    auto value     = extra::trait<get_value>(instance);
    REQUIRE(84 == value);
  }
}
