
#include <span>

#include <extra/hex_dump.hpp>
#include <gtest/gtest.h>

TEST(hex_dump, basic)
{
  char const test_data[] = "hello my friend";
  auto       hex_dump    = extra::make_hex_dump(
    std::as_bytes(std::span(test_data, sizeof(test_data))));
  EXPECT_EQ(
    hex_dump,
    "00000000: 68 65 6c 6c 6f 20 6d 79  20 66 72 69 65 6e 64 00  hello my friend.\n");
}
