
#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace extra
{
  template <typename Tag, typename T>
  struct trait_impl;

  template <typename T, typename Tag>
  concept has_trait = requires { trait_impl<Tag, T>{}; };

  template <typename Tag, has_trait<Tag> T = void>
  inline constexpr auto trait = trait_impl<Tag, T>{};

  template <typename Tag>
  struct trait_impl<Tag, void>
  {
    using is_transparent = std::true_type;

    template <typename T,
              typename... U,
              typename Target = std::remove_cvref_t<T>>
      requires has_trait<Target, Tag> and
               std::invocable<trait_impl<Tag, Target>, T, U...>
    inline constexpr auto operator()(T&& target, U&&... args) const
      noexcept(std::is_nothrow_invocable_v<trait_impl<Tag, Target>, T, U...>)
        -> std::invoke_result_t<trait_impl<Tag, Target>, T, U...>
    {
      return trait<Tag, Target>(std::forward<T>(target),
                                std::forward<U>(args)...);
    }
  };

  template <typename T, typename Tag>
  concept has_trait_impl_from_target_nested_type =
    not std::is_void_v<T> and requires {
      {
        typename T::template trait<Tag>{}
      };
    };

  template <typename Tag, typename T>
    requires has_trait_impl_from_target_nested_type<T, Tag>
  struct trait_impl<Tag, T> : T::template trait<Tag>
  {};

  template <typename T, typename Tag>
  concept has_trait_impl_from_tag_nested_type =
    not std::is_void_v<T> and requires {
      {
        typename Tag::template trait_for<T>{}
      };
    };

  template <typename Tag, typename T>
    requires has_trait_impl_from_tag_nested_type<T, Tag> and
             (not has_trait_impl_from_target_nested_type<T, Tag>)
  struct trait_impl<Tag, T> : Tag::template trait_for<T>
  {};

  namespace internal::trait_impl_from_adl
  {
    template <typename... U>
    void trait_impl(U...) = delete;

    template <typename... U>
    auto adl_helper(U... args)
    {
      return trait_impl(args...);
    }

    struct type_identity_getter
    {
      template <typename... U>
      constexpr auto operator()(U... args) noexcept
      {
        return adl_helper(args...);
      }
    };

    template <typename T>
    using trait_impl_bridge =
      std::invoke_result_t<type_identity_getter, std::type_identity<T>>::type;

    template <typename Tag, typename T>
    using type = trait_impl_bridge<T>::template trait<Tag>;

    template <typename T, template <typename...> typename Template>
    inline constexpr bool is_specialization_v = false;

    template <template <typename...> typename T, typename... U>
    inline constexpr bool is_specialization_v<T<U...>, T> = true;

    template <typename T>
    concept specialization_of_type_identity =
      is_specialization_v<T, std::type_identity>;

    template <typename T, typename Tag>
    concept type_check = requires {
      {
        trait_impl(std::type_identity<T>{})
      } -> specialization_of_type_identity;
      {
        type<Tag, T>{}
      };
    };

  } // namespace internal::trait_impl_from_adl

  template <typename T, typename Tag>
  concept has_trait_impl_from_adl =
    internal::trait_impl_from_adl::type_check<T, Tag>;

  template <typename Tag, typename T>
    requires has_trait_impl_from_adl<T, Tag>
  struct trait_impl<Tag, T> : internal::trait_impl_from_adl::type<Tag, T>
  {};

} // namespace extra
