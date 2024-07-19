
#include <catch2/catch_test_macros.hpp>
#include <extra/trait.hpp>

struct get_twelve
{};

struct get_seven : extra::trait<get_seven>
{
  template <typename...>
  struct trait_for;

  template <typename T>
  struct trait_for<T>
  {
    constexpr int operator()(T const&) const noexcept
    {
      return 7;
    }
  };
};

struct empty
{
  template <typename...>
  struct trait;
};

template <>
struct empty::trait<get_twelve>
{
  constexpr int operator()(empty) const noexcept
  {
    return 12;
  }
};

TEST_CASE("Check trait implemenations from transparent call", "[trait]")
{
  empty value{};

  SECTION("Invoke without deducing")
  {
    auto twelve = extra::trait_v<get_twelve, empty>(value);
    REQUIRE(12 == twelve);
  }

  SECTION("Invoke with deducing of target type")
  {
    auto twelve = extra::trait_v<get_twelve>(value);
    REQUIRE(12 == twelve);
  }

  SECTION("Invoke with deducing of tag and target types")
  {
    auto twelve = extra::trait_v<>(get_twelve{}, value);
    REQUIRE(12 == twelve);
  }

  SECTION("Invoke via a special object")
  {
    auto seven = extra::trait_v<>(get_seven{}, value);
    REQUIRE(7 == seven);
  }

  SECTION("Invoke via a special object")
  {
    auto seven = get_seven{}(value);
    REQUIRE(7 == seven);
  }
}
