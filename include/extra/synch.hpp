
#pragma once

#include <chrono>
#include <concepts>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>

namespace extra
{
  /// Synchronized data holder.
  ///
  /// Provides safe concurrent access to data with read/write guards.
  /// Class that owns both data and a mutex.
  /// Guarantees that access to data is performed only under lock.
  ///
  /// @tparam T Data type.
  /// @tparam M Mutex type.
  ///
  /// ## Common usage mistakes:
  ///
  /// ### Re-locking
  /// Re-locking the same `synch` in the same thread and scope.
  /// - In write mode (`wlock()`), this leads to deadlock.
  /// - In read mode (`rlock()`), this results in undefined behavior.
  ///
  /// ### Holding references to data for too long
  /// Storing a reference or pointer to protected data longer than the lifetime of the handle.
  /// ```cpp
  /// extra::synch<my::some_class> data();
  /// my::some_class* ref;
  /// {
  ///     auto handle = data.wlock();
  ///     ref = &(*handle);
  ///     ref->do_things();
  /// }
  /// // WARNING: handle is destroyed, further use of ref is UB
  /// ref->do_things();
  /// ```
  ///
  /// ### Mutating locks on multiple objects
  /// Locking two objects in different orders across threads
  /// may lead to a deadlock.
  /// ```cpp
  /// extra::synch<my::some_class> m1;
  /// extra::synch<my::whatever> m2;
  ///
  /// std::async([&] {
  ///     auto h1 = m1.rlock();
  ///     auto h2 = m2.wlock();
  /// });
  ///
  /// std::async([&] {
  ///     auto h2 = m2.rlock();
  ///     auto h1 = m1.wlock();
  /// });
  /// ```
  /// To avoid deadlock, use `extra::wlock`:
  /// ```cpp
  /// auto [h1, h2] = extra::wlock(m1, m2);
  /// ```
  ///
  /// ### Using a moved-from handle
  /// Using a moved-from handle is invalid.
  /// ```cpp
  /// auto handle = data.lock_shared();
  /// decltype(handle) handle2(std::move(handle));
  /// // WARNING: DO NOT use a moved-from handle
  /// int very_dangerous = *handle;
  /// ```
  template <typename T, typename M>
  class synch;

  /// @cond INTERNAL
  namespace detail
  {
    /// Concept: mutex satisfies
    /// [BasicLockable](https://en.cppreference.com/w/cpp/named_req/BasicLockable).
    ///
    /// @tparam T Checked mutex type.
    template <typename T>
    concept basic_lockable = requires(T locable) {
      locable.lock();
      locable.unlock();
    };

    /// Concept: mutex satisfies
    /// [Lockable](https://en.cppreference.com/w/cpp/named_req/Lockable).
    ///
    /// @tparam T Checked mutex type.
    template <typename T>
    concept lockable = basic_lockable<T> and requires(T locable) {
      { locable.try_lock() } -> std::same_as<bool>;
    };

    /// Concept: mutex satisfies
    /// [TimedLockable](https://en.cppreference.com/w/cpp/named_req/TimedLockable).
    ///
    /// @tparam T Checked mutex type.
    template <typename T>
    concept timed_lockable = lockable<T> and requires(T locable) {
      // try_lock_for
      {
        locable.try_lock_for(std::chrono::nanoseconds{ 1 })
      } -> std::same_as<bool>;

      // try_lock_until
      {
        locable.try_lock_until(std::chrono::high_resolution_clock::now())
      } -> std::same_as<bool>;
    };

    /// Concept: mutex satisfies BasicSharedLockable.
    /// Not standard, derived from BasicLockable/Lockable.
    ///
    /// @tparam T Checked mutex type.
    template <typename T>
    concept basic_shared_lockable = requires(T locable) {
      locable.lock_shared();
      locable.unlock_shared();
    };

    /// Concept: mutex satisfies
    /// [SharedLockable](https://en.cppreference.com/w/cpp/named_req/SharedLockable).
    ///
    /// @tparam T Checked mutex type.
    template <typename T>
    concept shared_lockable = basic_shared_lockable<T> and requires(T locable) {
      { locable.try_lock_shared() } -> std::same_as<bool>;
    };

    /// Concept: mutex satisfies
    /// [SharedTimedLockable](https://en.cppreference.com/w/cpp/named_req/SharedTimedLockable).
    ///
    /// @tparam T Checked mutex type.
    template <typename T>
    concept shared_timed_lockable = shared_lockable<T> and requires(T locable) {
      // try_lock_shared_for
      {
        locable.try_lock_shared_for(std::chrono::nanoseconds{ 1 })
      } -> std::same_as<bool>;

      // try_lock_shared_until
      {
        locable.try_lock_shared_until(std::chrono::high_resolution_clock::now())
      } -> std::same_as<bool>;
    };

    /// Proxy methods for read/write operations depending on mutex capabilities
    namespace proxy_mutex
    {
      template <basic_shared_lockable M>
      inline void lock_read(M& mutex)
      {
        mutex.lock_shared();
      }

      template <typename M>
      inline void lock_read(M& mutex)
        requires(not basic_shared_lockable<M>)
      {
        mutex.lock();
      }

      template <typename M>
      inline void lock_write(M& mutex)
      {
        mutex.lock();
      }

      template <shared_lockable M>
      inline bool try_lock_read(M& mutex)
      {
        return mutex.try_lock_shared();
      }

      template <typename M>
      inline bool try_lock_read(M& mutex)
        requires(not shared_lockable<M>)
      {
        return mutex.try_lock();
      }

      template <shared_timed_lockable M, typename Rep, typename Period>
      inline bool try_lock_read_for(
        M&                                        mutex,
        std::chrono::duration<Rep, Period> const& duration)
      {
        return mutex.try_lock_shared_for(duration);
      }

      template <typename M, typename Rep, typename Period>
      inline bool try_lock_read_for(
        M&                                        mutex,
        std::chrono::duration<Rep, Period> const& duration)
        requires(not shared_timed_lockable<M>)
      {
        return mutex.try_lock_for(duration);
      }

      template <shared_timed_lockable M, typename Clock, typename Duration>
      inline bool try_lock_read_until(
        M&                                              mutex,
        std::chrono::time_point<Clock, Duration> const& time)
      {
        return mutex.try_lock_shared_until(time);
      }

      template <typename M, typename Clock, typename Duration>
      inline bool try_lock_read_until(
        M&                                              mutex,
        std::chrono::time_point<Clock, Duration> const& time)
        requires(not shared_timed_lockable<M>)
      {
        return mutex.try_lock_until(time);
      }

      template <typename M, typename Rep, typename Period>
      inline bool try_lock_write_for(
        M&                                        mutex,
        std::chrono::duration<Rep, Period> const& duration)
      {
        return mutex.try_lock_for(duration);
      }

      template <typename M, typename Clock, typename Duration>
      inline bool try_lock_write_until(
        M&                                              mutex,
        std::chrono::time_point<Clock, Duration> const& time)
      {
        return mutex.try_lock_until(time);
      }

      template <typename M>
      inline bool try_lock_write(M& mutex)
      {
        return mutex.try_lock();
      }

      template <basic_shared_lockable M>
      inline void unlock_read(M& mutex)
      {
        mutex.unlock_shared();
      }

      template <typename M>
      inline void unlock_read(M& mutex)
        requires(not basic_shared_lockable<M>)
      {
        mutex.unlock();
      }

      template <typename M>
      inline void unlock_write(M& mutex)
      {
        mutex.unlock();
      }
    } // namespace proxy_mutex

    /// Token allowing access to private members of `synch`.
    struct synch_access
    {
      /// Adopt a lock on `synch`.
      template <typename T, typename M>
      static auto adopt_lock(synch<T, M>& mutex) ->
        typename synch<T, M>::wlock_guard
      {
        return mutex.wlock(std::adopt_lock);
      }

      /// Get reference to the mutex inside `synch`.
      template <typename T, typename M>
      static auto get_mutex_ref(synch<T, M>& mutex) ->
        typename synch<T, M>::mutex_type&
      {
        return mutex.mutex_;
      }
    };

    /// Get reference to a raw mutex.
    template <typename U>
    auto get_mutex_ref(U& mutex) -> U&
    {
      return mutex;
    }

    /// Get reference to a mutex inside `synch`.
    template <typename T, typename M>
    auto get_mutex_ref(synch<T, M>& mutex) -> typename synch<T, M>::mutex_type&
    {
      return synch_access::get_mutex_ref(mutex);
    }

    /// Adopt lock on a raw mutex.
    template <template <typename> typename L, typename T>
    auto adopt_lock(T& mutex) -> L<T>
    {
      return L<T>(mutex, std::adopt_lock);
    }

    /// Adopt lock on a `synch`.
    template <template <typename> typename L, typename T, typename M>
    auto adopt_lock(synch<T, M>& mutex) -> typename synch<T, M>::wlock_guard
    {
      return synch_access::adopt_lock(mutex);
    }

    template <typename Tuple, std::size_t... I>
    void lock_all_impl(Tuple& mutexes, std::index_sequence<I...>)
    {
      std::lock(std::get<I>(mutexes)...);
    }

    template <typename Tuple>
    void lock_all(Tuple& mutexes)
    {
      constexpr std::size_t Size = std::tuple_size_v<Tuple>;
      lock_all_impl(mutexes, std::make_index_sequence<Size>{});
    }

    template <typename Tuple, std::size_t... I>
    bool try_lock_all_impl(Tuple& mutexes, std::index_sequence<I...>)
    {
      // Yes, -1 means success. https://en.cppreference.com/w/cpp/thread/try_lock
      return std::try_lock(std::get<I>(mutexes)...) == -1;
    }

    /// Attempt to lock all provided mutexes.
    ///
    /// @return true on success.
    template <typename Tuple>
    bool try_lock_all(Tuple& mutexes)
    {
      constexpr std::size_t Size = std::tuple_size_v<Tuple>;
      return try_lock_all_impl(mutexes, std::make_index_sequence<Size>{});
    }

    template <template <typename> typename L, typename... Args>
    auto wlock_impl(Args&... args)
      -> std::tuple<decltype(detail::adopt_lock<L>(args))...>
    {
      auto mutex_refs = std::tie(detail::get_mutex_ref(args)...);
      detail::lock_all(mutex_refs);
      return std::make_tuple(detail::adopt_lock<L>(args)...);
    }

    template <template <typename> typename L, typename... Args>
    auto try_wlock_impl(Args&... args)
      -> std::optional<std::tuple<decltype(detail::adopt_lock<L>(args))...>>
    {
      auto mutex_refs = std::tie(detail::get_mutex_ref(args)...);
      if (not detail::try_lock_all(mutex_refs))
      {
        return {};
      }
      auto tuple = std::make_tuple(detail::adopt_lock<L>(args)...);
      return std::move(tuple);
    }

  } // namespace detail
  /// @endcond

  /// Acquire mutable access to any number of `synch` or raw mutexes
  /// with deadlock prevention.
  ///
  /// @tparam L Lock type for raw mutexes.
  /// @tparam Args Argument types.
  ///
  /// @param args Any number of `synch` or raw mutex objects.
  ///
  /// @return A tuple: `synch::wlock_guard` for each `synch`
  /// and L<T> locks for raw mutexes.
  ///
  /// Based on the standard `std::lock()`.
  template <template <typename> typename L = std::unique_lock, typename... Args>
  [[nodiscard]] inline auto wlock(Args&... args)
    -> decltype(detail::wlock_impl<L>(args...))
  {
    return detail::wlock_impl<L>(args...);
  }

  /// Attempt to acquire mutable access to multiple `synch`/mutexes.
  ///
  /// @tparam L Lock type for raw mutexes.
  /// @tparam Args Argument types.
  ///
  /// @param args Any number of `synch` or raw mutexes.
  ///
  /// @return std::optional with tuple. Empty if any lock fails.
  ///
  /// Based on the standard `std::try_lock()`.
  template <template <typename> typename L = std::unique_lock, typename... Args>
  [[nodiscard]] inline auto try_wlock(Args&... args)
    -> decltype(detail::try_wlock_impl<L>(args...))
  {
    return detail::try_wlock_impl<L>(args...);
  }

  /// RAII object for accessing data inside `synch`.
  ///
  /// @tparam T Data type.
  /// @tparam M Mutex type.
  ///
  /// Releases access upon destruction.
  template <typename T, typename M>
  class [[nodiscard]] synch_lock_guard
  {
  private:
    void unlock()
    {
      if (mutex_ == nullptr)
      {
        return;
      }

      if constexpr (std::is_const_v<T>)
      {
        detail::proxy_mutex::unlock_read(*mutex_);
      }
      else
      {
        detail::proxy_mutex::unlock_write(*mutex_);
      }
    }

  public:
    // Only the parent `synch` may create this object.
    template <typename TT, typename MM>
    friend class synch;

    /// Type of stored value (const/volatile removed).
    using value_type = std::remove_cv_t<T>;

    synch_lock_guard()                        = delete;
    synch_lock_guard(synch_lock_guard const&) = delete;

    [[nodiscard]] synch_lock_guard(synch_lock_guard&& other) noexcept
      : mutex_{ std::exchange(other.mutex_, nullptr) }
      , value_{ std::exchange(other.value_, nullptr) }
    {}

    synch_lock_guard& operator=(synch_lock_guard const& other) = delete;

    synch_lock_guard& operator=(synch_lock_guard&& other) noexcept
    {
      if (std::addressof(other) != this)
      {
        unlock();
        mutex_ = std::exchange(other.mutex_, nullptr);
        value_ = std::exchange(other.value_, nullptr);
      }

      return *this;
    }

    ~synch_lock_guard()
    {
      unlock();
    }

    [[nodiscard]] T& operator*()
    {
      return *value_;
    }

    [[nodiscard]] T* operator->()
    {
      return value_;
    }

  private:
    M* mutex_;
    T* value_;

    /// Create guard on an ALREADY LOCKED mutex.
    [[nodiscard]] synch_lock_guard(M* mutex, T* value) noexcept
      : mutex_{ mutex }
      , value_{ value }
    {}
  };

  /// Default mutex type used in `synch`.
  using synch_default_mutex = std::shared_timed_mutex;

  template <typename T, typename M = synch_default_mutex>
  class [[nodiscard]] synch
  {
  public:
    /// Type of stored value (without const/volatile), provided for convenience.
    using value_type  = std::remove_cv_t<T>;
    /// Mutex type used, provided for convenience.
    using mutex_type  = M;
    /// Guard type for read-only access.
    using rlock_guard = synch_lock_guard<value_type const, M>;
    /// Guard type for mutable access.
    using wlock_guard = synch_lock_guard<value_type, M>;

    /// Create a new instance.
    ///
    /// @tparam Args Types of arguments for the protected value constructor.
    ///
    /// @param args Arguments for the protected value constructor.
    ///
    /// `requires` forbids this constructor if argument is a `synch`.
    template <typename... Args>
      requires(not std::is_same_v<synch, std::remove_cvref_t<Args>> and ...)
    [[nodiscard]] explicit synch(Args&&... args)
      : value_(std::forward<Args>(args)...)
    {}

    [[nodiscard]] synch(synch const& other)
      requires std::is_copy_constructible_v<T>
      : value_(*other.rlock())
    {}

    [[nodiscard]] synch(synch&& other) noexcept
      requires std::is_move_constructible_v<T>
      : value_(std::move(*other.wlock()))
    {}

    synch& operator=(synch const& other)
      requires std::is_assignable_v<T&, T const&>
    {
      if (std::addressof(other) != this)
      {
        std::lock_guard lock(mutex_);
        value_ = *other.rlock();
      }

      return *this;
    }

    synch& operator=(synch&& other) noexcept
      requires std::is_assignable_v<T&, T&&>
    {
      if (std::addressof(other) != this)
      {
        auto lock_and_handle = extra::wlock(mutex_, other);
        value_               = std::move(*std::get<1>(lock_and_handle));
      }

      return *this;
    }

    ~synch() = default;

  private:
    std::optional<rlock_guard> try_rlock_impl() const
    {
      if (detail::proxy_mutex::try_lock_read(mutex_))
      {
        return rlock_guard(&mutex_, &value_);
      }

      return {};
    }

    template <typename Rep, typename Period>
    std::optional<rlock_guard> try_rlock_for_impl(
      std::chrono::duration<Rep, Period> const& duration) const
    {
      if (detail::proxy_mutex::try_lock_read_for(mutex_, duration))
      {
        return rlock_guard(&mutex_, &value_);
      }

      return {};
    }

    template <typename Clock, typename Duration>
    std::optional<rlock_guard> try_rlock_until_impl(
      std::chrono::time_point<Clock, Duration> const& time) const
    {
      if (detail::proxy_mutex::try_lock_read_until(mutex_, time))
      {
        return rlock_guard(&mutex_, &value_);
      }

      return {};
    }

    std::optional<wlock_guard> try_wlock_impl()
    {
      if (detail::proxy_mutex::try_lock_write(mutex_))
      {
        return wlock_guard(&mutex_, &value_);
      }

      return {};
    }

    template <typename Rep, typename Period>
    std::optional<wlock_guard> try_wlock_for_impl(
      std::chrono::duration<Rep, Period> const& duration)
    {
      if (detail::proxy_mutex::try_lock_write_for(mutex_, duration))
      {
        return wlock_guard(&mutex_, &value_);
      }

      return {};
    }

    template <typename Clock, typename Duration>
    std::optional<wlock_guard> try_wlock_until_impl(
      std::chrono::time_point<Clock, Duration> const& time)
    {
      if (detail::proxy_mutex::try_lock_write_until(mutex_, time))
      {
        return wlock_guard(&mutex_, &value_);
      }

      return {};
    }

  public:
    /// Acquire data for reading.
    [[nodiscard]] rlock_guard rlock() const
    {
      detail::proxy_mutex::lock_read(mutex_);
      return rlock_guard(&mutex_, &value_);
    }

    /// Try to acquire read access.
    [[nodiscard]] std::optional<rlock_guard> try_rlock() const
    {
      return try_rlock_impl();
    }

    /// Try to acquire read access (try_to_lock).
    [[nodiscard]] std::optional<rlock_guard> rlock(std::try_to_lock_t) const
    {
      return try_rlock_impl();
    }

    /// Attempt to acquire read access for the specified duration.
    template <typename Rep, typename Period>
    [[nodiscard]] std::optional<rlock_guard> try_rlock_for(
      std::chrono::duration<Rep, Period> const& duration) const
    {
      return try_rlock_for_impl(duration);
    }

    /// Attempt to acquire read access until the specified time point.
    template <typename Clock, typename Duration>
    [[nodiscard]] std::optional<rlock_guard> try_rlock_until(
      std::chrono::time_point<Clock, Duration> const& time) const
    {
      return try_rlock_until_impl(time);
    }

    /// Acquire data for writing.
    [[nodiscard]] wlock_guard wlock()
    {
      detail::proxy_mutex::lock_write(mutex_);
      return wlock_guard(&mutex_, &value_);
    }

    /// Try to acquire write access.
    [[nodiscard]] std::optional<wlock_guard> try_wlock()
    {
      return try_wlock_impl();
    }

    /// Try to acquire write access (try_to_lock).
    [[nodiscard]] std::optional<wlock_guard> wlock(std::try_to_lock_t)
    {
      return try_wlock_impl();
    }

    /// Attempt to acquire write access for a duration.
    template <typename Rep, typename Period>
    [[nodiscard]] std::optional<wlock_guard> try_wlock_for(
      std::chrono::duration<Rep, Period> const& duration)
    {
      return try_wlock_for_impl(duration);
    }

    /// Attempt to acquire write access until a time point.
    template <typename Clock, typename Duration>
    [[nodiscard]] std::optional<wlock_guard> try_wlock_until(
      std::chrono::time_point<Clock, Duration> const& time)
    {
      return try_wlock_until_impl(time);
    }

  private:
    T         value_;
    mutable M mutex_{};

    // Required so that `extra::wlock` can work.
    friend struct detail::synch_access;

    /// Create guard on an ALREADY LOCKED mutex.
    [[nodiscard]] wlock_guard wlock(std::adopt_lock_t)
    {
      return wlock_guard(&mutex_, &value_);
    }
  };
} // namespace extra
