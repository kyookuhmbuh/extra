
#include <extra/enum_flags.hpp>
#include <gtest/gtest.h>

enum class test_flags : unsigned int // NOLINT(performance-enum-size)
{
  none = 0,
  a    = 1u << 0,
  b    = 1u << 1,
  c    = 1u << 2,
  ab   = a | b,
  all  = a | b | c
};

template <>
struct extra::flags::is_enum_flags<test_flags> : std::true_type
{};

TEST(enum_flags_traits, is_enum_flags_true)
{
  EXPECT_TRUE(extra::flags::is_enum_flags_v<test_flags>);
}

TEST(enum_flags_traits, enum_flags_concept)
{
  EXPECT_TRUE(extra::flags::enum_flags<test_flags>);
}

TEST(enum_flags_bitwise_operators, or_operator)
{
  test_flags value = test_flags::a | test_flags::b;
  EXPECT_EQ(value, test_flags::ab);
}

TEST(enum_flags_bitwise_operators, and_operator)
{
  test_flags value = test_flags::ab & test_flags::a;
  EXPECT_EQ(value, test_flags::a);
}

TEST(enum_flags_bitwise_operators, xor_operator)
{
  test_flags value = test_flags::ab ^ test_flags::a;
  EXPECT_EQ(value, test_flags::b);
}

TEST(enum_flags_bitwise_operators, not_operator)
{
  test_flags value = ~test_flags::none;
  EXPECT_EQ(value & test_flags::all, test_flags::all);
}

TEST(enum_flags_bitwise_operators, or_assign_operator)
{
  test_flags value  = test_flags::a;
  value            |= test_flags::b;
  EXPECT_EQ(value, test_flags::ab);
}

TEST(enum_flags_bitwise_operators, and_assign_operator)
{
  test_flags value  = test_flags::ab;
  value            &= test_flags::a;
  EXPECT_EQ(value, test_flags::a);
}

TEST(enum_flags_bitwise_operators, xor_assign_operator)
{
  test_flags value  = test_flags::ab;
  value            ^= test_flags::b;
  EXPECT_EQ(value, test_flags::a);
}

TEST(enum_flags_queries, is_empty)
{
  EXPECT_TRUE(extra::flags::is_empty(test_flags::none));
  EXPECT_FALSE(extra::flags::is_empty(test_flags::a));
}

TEST(enum_flags_queries, is_empty_with_mask)
{
  EXPECT_TRUE(extra::flags::is_empty(test_flags::a, test_flags::b));
  EXPECT_FALSE(extra::flags::is_empty(test_flags::ab, test_flags::b));
}

TEST(enum_flags_queries, has_all)
{
  EXPECT_TRUE(extra::flags::has_all(test_flags::ab, test_flags::a));
  EXPECT_TRUE(extra::flags::has_all(test_flags::ab, test_flags::ab));
  EXPECT_FALSE(extra::flags::has_all(test_flags::a, test_flags::ab));
}

TEST(enum_flags_queries, has_any)
{
  EXPECT_TRUE(extra::flags::has_any(test_flags::ab, test_flags::b));
  EXPECT_FALSE(extra::flags::has_any(test_flags::a, test_flags::b));
}

TEST(enum_flags_modification, set)
{
  auto value = extra::flags::set(test_flags::a, test_flags::b);
  EXPECT_EQ(value, test_flags::ab);
}

TEST(enum_flags_modification, set_inplace)
{
  test_flags value = test_flags::a;
  extra::flags::set_inplace(value, test_flags::b);
  EXPECT_EQ(value, test_flags::ab);
}

TEST(enum_flags_modification, clear)
{
  auto value = extra::flags::clear(test_flags::ab, test_flags::a);
  EXPECT_EQ(value, test_flags::b);
}

TEST(enum_flags_modification, clear_inplace)
{
  test_flags value = test_flags::ab;
  extra::flags::clear_inplace(value, test_flags::a);
  EXPECT_EQ(value, test_flags::b);
}

TEST(enum_flags_modification, apply_true)
{
  auto value = extra::flags::apply(test_flags::a, test_flags::b, true);
  EXPECT_EQ(value, test_flags::ab);
}

TEST(enum_flags_modification, apply_false)
{
  auto value = extra::flags::apply(test_flags::ab, test_flags::a, false);
  EXPECT_EQ(value, test_flags::b);
}

TEST(enum_flags_modification, apply_inplace)
{
  test_flags value = test_flags::a;
  extra::flags::apply_inplace(value, test_flags::b, true);
  EXPECT_EQ(value, test_flags::ab);
}

TEST(enum_flags_modification, toggle)
{
  auto value = extra::flags::toggle(test_flags::ab, test_flags::a);
  EXPECT_EQ(value, test_flags::b);
}

TEST(enum_flags_modification, toggle_inplace)
{
  test_flags value = test_flags::a;
  extra::flags::toggle_inplace(value, test_flags::a);
  EXPECT_EQ(value, test_flags::none);
}
