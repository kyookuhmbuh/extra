
#pragma once

// Main include for the `extra` utility library.
// This header aggregates all submodules.

#include <extra/args.hpp>
#include <extra/atomic_flags.hpp>
#include <extra/enum_flags.hpp>
#include <extra/hash_combine.hpp>
#include <extra/overloaded.hpp>
#include <extra/synch.hpp>
#include <extra/variant_alternative.hpp>
#include <extra/variant_visitor.hpp>

/// Root namespace for the Extra library utilities.
///
/// This namespace contains all helper modules:
/// - Argument parsing (`args.hpp`)
/// - Atomic and enum flags (`atomic_flags.hpp`, `enum_flags.hpp`)
/// - Hash combination utility (`hash_combine.hpp`)
/// - Visitor combinator for `std::variant` (`overloaded.hpp`)
/// - Synchronized data wrapper (`synch.hpp`)
/// - Variant concepts (`variant_alternative.hpp`, `variant_visitor.hpp`)
///
/// Example usage:
/// ```cpp
/// #include <extra/extra.hpp>
/// #include <iostream>
///
/// int main(int argc, char* argv[])
/// {
///     // Parse argument --value=42
///     int value = extra::get_arg<int>(argc, argv, "value", 0);
///     std::cout << "Value: " << value << "\n";
///
///     // Atomic flags
///     enum class Flags : unsigned { A = 1, B = 2, C = 4 };
///     extra::atomic_flags<Flags> af{Flags::A};
///     af.set(Flags::B);
///     if (af.has_any(Flags::B | Flags::C)) std::cout << "B or C is set\n";
///
///     // Synchronized data
///     extra::synch<int> data(10);
///     {
///         auto guard = data.wlock();
///         *guard += 5;
///     }
///     auto read_guard = data.rlock();
///     std::cout << "Data: " << *read_guard << "\n";
/// }
/// ```
namespace extra
{}
