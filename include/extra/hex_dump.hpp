
#pragma once

#include <cstdint>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <span>
#include <sstream>
#include <string>

namespace extra
{
  /// Hexadecimal memory dump formatter.
  ///
  /// Formats a byte sequence into a classic hex dump representation containing:
  /// - byte offsets,
  /// - hexadecimal byte values,
  /// - ASCII representation.
  ///
  /// Example output:
  /// @code
  /// 00000000: 48 65 6c 6c 6f 20 77 6f  72 6c 64              Hello world
  /// @endcode
  struct hex_dump
  {
    /// Input byte sequence to format.
    std::span<std::byte const> input;

    /// Number of bytes displayed per line.
    std::size_t bytes_per_line = 16;

    /// Write formatted hex dump into an output stream.
    ///
    /// @param output Output stream.
    /// @param manip Hex dump formatter instance.
    ///
    /// @return Reference to the output stream.
    friend std::ostream& operator<<(std::ostream& output, hex_dump const& manip)
    {
      auto old_flags = output.flags();
      output << std::hex << std::setfill('0');

      for (std::size_t offset  = 0; offset < manip.input.size();
           offset             += manip.bytes_per_line)
      {
        // Print line offset.
        output << std::setw(8) << offset << ": ";

        // Print hexadecimal byte values.
        for (size_t i = 0; i < manip.bytes_per_line; ++i)
        {
          if (offset + i < manip.input.size())
          {
            output << std::setw(2)
                   << std::to_integer<std::uint32_t>(manip.input[offset + i])
                   << ' ';
          }
          else
          {
            output << "   "; // Padding for incomplete lines.
          }

          if (i == (manip.bytes_per_line / 2) - 1)
          {
            output << ' '; // Separator in the middle of the line.
          }
        }

        output << ' ';

        // Print ASCII representation.
        for (std::size_t i = 0;
             i < manip.bytes_per_line && offset + i < manip.input.size();
             ++i)
        {
          auto byte = std::to_integer<std::int32_t>(manip.input[offset + i]);

          output << (std::isprint(byte) != 0 ? static_cast<char>(byte) : '.');
        }

        output << '\n';
      }

      output.flags(old_flags);
      return output;
    }

    /// Convert formatted hex dump into a string.
    ///
    /// @return String containing the formatted hex dump.
    [[nodiscard]] std::string operator()() const
    {
      std::ostringstream output;
      output << *this;
      return output.str();
    }
  };

  /// Create a formatted hexadecimal dump string.
  ///
  /// @param input Input byte sequence.
  /// @param bytes_per_line Number of bytes displayed per line.
  ///
  /// @return String containing the formatted hex dump.
  [[nodiscard]] inline std::string make_hex_dump(
    std::span<std::byte const> input,
    std::size_t                bytes_per_line = 16)
  {
    return std::invoke(
      hex_dump{ .input = input, .bytes_per_line = bytes_per_line });
  }
} // namespace extra
