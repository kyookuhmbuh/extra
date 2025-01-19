
#include <cassert>
#include <chrono>
#include <future>
#include <thread>

#include <extra/synch.hpp>
#include <gtest/gtest.h>

class basic_locable_mock
{
public:
  void lock()
  {
    mutex_.lock();
  }
  void unlock()
  {
    mutex_.unlock();
  }

private:
  std::mutex mutex_;
};

class locable_mock
{
public:
  void lock()
  {
    mutex_.lock();
  }

  bool try_lock()
  {
    return mutex_.try_lock();
  }

  void unlock()
  {
    mutex_.unlock();
  }

private:
  std::mutex mutex_;
};

class data_wrapper
{
public:
  explicit data_wrapper(int data)
    : data_{ data }
  {}

  int do_things() const // NOLINT(modernize-use-nodiscard)
  {
    return data_ * 2;
  };

  int do_things_mut()
  {
    data_ *= 3;
    return data_;
  };

  [[nodiscard]] int get_data() const
  {
    return data_;
  }

private:
  int data_;
};

// NOLINTBEGIN(readability-magic-numbers)

TEST(synch, construct_synch)
{
  EXPECT_NO_THROW({ [[maybe_unused]] auto cell = extra::synch<int>(int{}); });
}

TEST(synch, lock_synch)
{
  extra::synch<int> cell(42);
  auto              handle = cell.rlock();
  EXPECT_TRUE(*handle == 42);
}

TEST(synch, lock_synch_mutably)
{
  extra::synch<int> cell(42);
  auto              handle = cell.wlock();
  EXPECT_TRUE(*handle == 42);
  *handle = 8;
  EXPECT_TRUE(*handle == 8);
}

TEST(synch, access_synch_readonly)
{
  extra::synch<data_wrapper> cell(data_wrapper(111));
  auto                       handle = cell.rlock();
  EXPECT_TRUE(handle->do_things() == 222);
}

TEST(synch, access_synch_mutably)
{
  extra::synch<data_wrapper> cell(data_wrapper(111));
  auto                       handle = cell.wlock();
  EXPECT_TRUE(handle->do_things() == 222);
  EXPECT_TRUE(handle->do_things_mut() == 333);
  EXPECT_TRUE(handle->do_things() == 666);
}

// Without shared mutex there can only be one reader at a time and this code would deadlock.
TEST(synch, lock_synch_readonly_without_deadlock)
{
  extra::synch<std::string> name("Batman");
  auto                      handle = name.rlock();
  auto                      future = std::async(
    [&name]
    {
      auto handle                                = name.rlock();
      [[maybe_unused]] std::string volatile copy = *handle;
    });

  [[maybe_unused]] std::string volatile copy = *handle;
  future.wait();
  // The simple fact that this test ends is a proof of simultaneous read-access.
  EXPECT_TRUE(true);
}

TEST(synch, lock_synch_readonly_twice)
{
  extra::synch<int> cell(42);

  auto tic = std::chrono::high_resolution_clock::now();

  auto future = std::async(
    [&cell]
    {
      auto handle = cell.rlock();
      EXPECT_TRUE(*handle == 42);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  auto future2 = std::async(
    [&cell]
    {
      auto handle = cell.rlock();
      EXPECT_TRUE(*handle == 42);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
  future.wait();
  future2.wait();

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(150));
}

TEST(synch, lock_synch_mutably_while_locked_readonly)
{
  extra::synch<int> cell(42);

  auto tic = std::chrono::high_resolution_clock::now();

  std::atomic<bool> started{ false };
  auto              future = std::async(
    [&cell, &started]
    {
      auto handle = cell.rlock();
      started     = true;
      EXPECT_TRUE(*handle == 42);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  auto future2 = std::async(
    [&cell]
    {
      auto handle = cell.wlock();
      EXPECT_TRUE(*handle == 42);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
  future.wait();
  future2.wait();

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(200));
}

TEST(synch, lock_synch_readonly_while_locked_mutably)
{
  extra::synch<int> cell(42);

  auto tic = std::chrono::high_resolution_clock::now();

  std::atomic<bool> started{ false };
  auto              future = std::async(
    [&cell, &started]
    {
      auto handle = cell.wlock();
      started     = true;
      EXPECT_TRUE(*handle == 42);
      *handle = 15;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  auto future2 = std::async(
    [&cell]
    {
      auto handle = cell.rlock();
      EXPECT_TRUE(*handle == 15);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
  future.wait();
  future2.wait();

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(200));
}

TEST(synch, lock_synch_mutably_while_locked_mutably)
{
  extra::synch<int> cell(42);

  auto tic = std::chrono::high_resolution_clock::now();

  std::atomic<bool> started{ false };
  auto              future = std::async(
    [&cell, &started]
    {
      auto handle = cell.wlock();
      started     = true;
      EXPECT_TRUE(*handle == 42);
      *handle = 15;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  auto future2 = std::async(
    [&cell]
    {
      auto handle = cell.wlock();
      EXPECT_TRUE(*handle == 15);
      *handle = 12;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
  future.wait();
  future2.wait();

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(200));
}

TEST(synch, try_lock_mutably)
{
  extra::synch<int> cell(42);
  std::atomic<bool> started{ false };

  auto future = std::async(
    [&cell, &started]
    {
      auto opt_handle = cell.try_wlock();
      started         = true;
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
      **opt_handle = 45;
      EXPECT_TRUE(**opt_handle == 45);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  while (not started)
  {
    // BUSY WAITING
  }

  auto opt_handle2 = cell.wlock(std::try_to_lock);
  EXPECT_FALSE(opt_handle2);
  future.wait();
}

TEST(synch, try_lock)
{
  extra::synch<int> cell(42);
  std::atomic<bool> started{ false };

  auto future = std::async(
    [&cell, &started]
    {
      auto opt_handle = cell.try_rlock();
      started         = true;
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  while (not started)
  {
    // BUSY WAITING
  }

  auto future2 = std::async(
    [&cell]
    {
      auto opt_handle = cell.rlock(std::try_to_lock);
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
    });

  auto opt_handle = cell.try_wlock();
  EXPECT_FALSE(opt_handle);
  future.wait();
  future2.wait();
}

TEST(synch, try_lock_mutably_for)
{
  extra::synch<int>                                           cell(42);
  std::atomic<std::chrono::high_resolution_clock::time_point> sleep_start{
    std::chrono::high_resolution_clock::time_point::min()
  };

  auto future = std::async(
    [&cell, &sleep_start]
    {
      auto opt_handle = cell.try_wlock_for(std::chrono::nanoseconds(1));
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
      **opt_handle = 45;
      EXPECT_TRUE(**opt_handle == 45);
      sleep_start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  while (sleep_start.load() ==
         std::chrono::high_resolution_clock::time_point::min())
  {
    // BUSY WAITING
  }

  {
    auto opt_handle = cell.try_wlock_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(opt_handle);
  }
  {
    // Unfortunately on windows, on CI this sometimes takes way more than 100 ms to be acquired.
    auto       opt_handle = cell.try_wlock_for(std::chrono::milliseconds(1000));
    auto const tac        = std::chrono::high_resolution_clock::now();

    EXPECT_TRUE(opt_handle);
    EXPECT_TRUE(**opt_handle == 45);
    EXPECT_TRUE(tac - sleep_start.load() >= std::chrono::milliseconds(100));
  }
  future.wait();
}

TEST(synch, try_lock_for)
{
  extra::synch<int>                                           cell(42);
  std::atomic<std::chrono::high_resolution_clock::time_point> sleep_start{
    std::chrono::high_resolution_clock::time_point::min()
  };

  auto future = std::async(
    [&cell, &sleep_start]
    {
      auto opt_handle = cell.try_rlock_for(std::chrono::nanoseconds(1));
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
      sleep_start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  while (sleep_start.load() ==
         std::chrono::high_resolution_clock::time_point::min())
  {
    // BUSY WAITING
  }

  {
    auto opt_handle = cell.try_rlock_for(std::chrono::nanoseconds(1));

    EXPECT_TRUE(opt_handle);
    EXPECT_TRUE(**opt_handle == 42);
  }
  {
    auto opt_handle = cell.try_wlock_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(opt_handle);
  }
  {
    // Unfortunately on windows, on CI this sometimes takes way more than 100 ms to be acquired.
    auto       opt_handle = cell.try_wlock_for(std::chrono::milliseconds(1000));
    auto const tac        = std::chrono::high_resolution_clock::now();

    EXPECT_TRUE(opt_handle);
    EXPECT_TRUE(**opt_handle == 42);
    EXPECT_TRUE(tac - sleep_start.load() >= std::chrono::milliseconds(100));
  }
  future.wait();
}

TEST(synch, try_lock_mutably_until)
{
  extra::synch<int> cell(42);

  std::atomic<std::chrono::high_resolution_clock::time_point> sleep_start{
    std::chrono::high_resolution_clock::time_point::min()
  };
  auto future = std::async(
    [&cell, &sleep_start]
    {
      auto opt_handle = cell.try_wlock_until(
        std::chrono::high_resolution_clock::time_point::max());
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
      **opt_handle = 45;
      EXPECT_TRUE(**opt_handle == 45);
      sleep_start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  while (sleep_start.load() ==
         std::chrono::high_resolution_clock::time_point::min())
  {
    // BUSY WAITING
  }

  {
    auto opt_handle =
      cell.try_wlock_until(sleep_start.load() + std::chrono::milliseconds(20));
    EXPECT_FALSE(opt_handle);
  }
  {
    // Unfortunately on windows, on CI this sometimes takes way more than 100 ms to be acquired.
    auto opt_handle = cell.try_wlock_until(
      sleep_start.load() + std::chrono::milliseconds(1000));
    auto const tac = std::chrono::high_resolution_clock::now();

    EXPECT_TRUE(opt_handle);
    EXPECT_TRUE(**opt_handle == 45);
    EXPECT_TRUE(tac - sleep_start.load() >= std::chrono::milliseconds(100));
  }
  future.wait();
}

TEST(synch, try_lock_until)
{
  extra::synch<int>                                           cell(42);
  std::atomic<std::chrono::high_resolution_clock::time_point> sleep_start{
    std::chrono::high_resolution_clock::time_point::min()
  };

  auto future = std::async(
    [&cell, &sleep_start]
    {
      auto opt_handle = cell.try_rlock_until(
        std::chrono::high_resolution_clock::time_point::max());
      EXPECT_TRUE(opt_handle);
      EXPECT_TRUE(**opt_handle == 42);
      sleep_start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  while (sleep_start.load() ==
         std::chrono::high_resolution_clock::time_point::min())
  {
    // BUSY WAITING
  }

  {
    auto opt_handle =
      cell.try_rlock_until(sleep_start.load() + std::chrono::nanoseconds(1));

    EXPECT_TRUE(opt_handle);
    EXPECT_TRUE(**opt_handle == 42);
  }
  {
    auto opt_handle =
      cell.try_wlock_until(sleep_start.load() + std::chrono::milliseconds(20));
    EXPECT_FALSE(opt_handle);
  }
  {
    // Unfortunately on windows, on CI this sometimes takes way more than 100 ms to be acquired.
    auto opt_handle = cell.try_wlock_until(
      sleep_start.load() + std::chrono::milliseconds(1000));
    auto const tac = std::chrono::high_resolution_clock::now();

    EXPECT_TRUE(opt_handle);
    EXPECT_TRUE(**opt_handle == 42);
    EXPECT_TRUE(tac - sleep_start.load() >= std::chrono::milliseconds(100));
  }
  future.wait();
}

TEST(synch, move_handle_mutably_construct)
{
  extra::synch<int> cell(42);

  auto             handle = cell.wlock();
  decltype(handle) handle2(std::move(handle));

  EXPECT_TRUE(*handle2 == 42);
}

TEST(synch, move_handle_construct)
{
  extra::synch<int> cell(42);

  auto             handle = cell.rlock();
  decltype(handle) handle2(std::move(handle));

  EXPECT_TRUE(*handle2 == 42);
}

TEST(synch, move_handle_mutably_assign)
{
  extra::synch<int> cell1(1);
  extra::synch<int> cell2(2);

  {
    auto cell1_guard = cell1.wlock();
    auto cell2_guard = cell2.wlock();
    cell2_guard      = std::move(cell1_guard);
    *cell2_guard     = 3;
  }

  EXPECT_TRUE(*cell1.rlock() == 3);
  EXPECT_TRUE(*cell2.rlock() == 2);
}

TEST(synch, move_handle_assign)
{
  extra::synch<int> cell1(1);
  extra::synch<int> cell2(2);

  {
    auto cell1_guard = cell1.rlock();
    auto cell2_guard = cell2.rlock();
    cell2_guard      = std::move(cell1_guard);
  }

  EXPECT_TRUE(*cell1.rlock() == 1);
  EXPECT_TRUE(*cell2.rlock() == 2);
}

TEST(synch, copy_synch_unused_construct)
{
  extra::synch<int> cell(42);

  auto           tic = std::chrono::high_resolution_clock::now();
  decltype(cell) cell2(cell);
  auto           tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(10));

  EXPECT_TRUE(*cell.rlock() == 42);
  EXPECT_TRUE(*cell2.rlock() == 42);
}

TEST(synch, copy_synch_used_mutably_construct)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell(42);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell, &started]
    {
      auto handle = cell.wlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  decltype(cell) cell2(cell);

  EXPECT_TRUE(*cell2.rlock() == 42);

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, copy_synch_used_readonly_construct)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell(42);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell, &started]
    {
      auto handle = cell.rlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  decltype(cell) cell2(cell);

  EXPECT_TRUE(*cell2.rlock() == 42);

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(80));
}

TEST(synch, copy_synch_unused_assign)
{
  extra::synch<int> cell1(42);
  extra::synch<int> cell2(3);

  auto tic = std::chrono::high_resolution_clock::now();
  cell2    = cell1;
  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(10));

  EXPECT_TRUE(*cell1.rlock() == 42);
  EXPECT_TRUE(*cell2.rlock() == 42);
}

TEST(synch, copy_synch_used_mutably_assign)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell1(42);
  extra::synch<int> cell2(2);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell1, &started]
    {
      auto handle = cell1.wlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  cell2 = cell1;

  auto tac = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(*cell2.rlock() == 42);
  EXPECT_TRUE(*cell1.rlock() == 42);
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, copy_synch_used_readonly_assign)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell1(42);
  extra::synch<int> cell2(2);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell1, &started]
    {
      auto handle = cell1.rlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  cell2 = cell1;

  auto tac = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(*cell2.rlock() == 42);
  EXPECT_TRUE(*cell1.rlock() == 42);
  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(80));
}

TEST(synch, move_synch_unused_construct)
{
  extra::synch<int> cell1(42);

  auto            tic = std::chrono::high_resolution_clock::now();
  decltype(cell1) cell2(std::move(cell1));
  auto            tac = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(10));
  EXPECT_TRUE(*cell2.rlock() == 42);
}

TEST(synch, move_synch_used_mutably_construct)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell(42);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell, &started]
    {
      auto handle = cell.wlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  decltype(cell) cell2(std::move(cell));

  EXPECT_TRUE(*cell2.rlock() == 42);

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, move_synch_used_readonly_construct)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell(42);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell, &started]
    {
      auto handle = cell.rlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  decltype(cell) cell2(std::move(cell));

  EXPECT_TRUE(*cell2.rlock() == 42);

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, move_synch_unused_assign)
{
  extra::synch<int> cell1(42);
  decltype(cell1)   cell2(5);

  auto tic = std::chrono::high_resolution_clock::now();
  cell2    = std::move(cell1);
  auto tac = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(tac - tic < std::chrono::milliseconds(10));
  EXPECT_TRUE(*cell2.rlock() == 42);
}

TEST(synch, move_synch_used_mutably_assign)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell1(42);
  extra::synch<int> cell2(2);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell1, &started]
    {
      auto handle = cell1.wlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  cell2 = std::move(cell1);

  auto tac = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(*cell2.rlock() == 42);
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, move_synch_used_readonly_assign)
{
  auto tic = std::chrono::high_resolution_clock::now();

  extra::synch<int> cell1(42);
  extra::synch<int> cell2(2);

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&cell1, &started]
    {
      auto handle = cell1.rlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  cell2 = std::move(cell1);

  auto tac = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(*cell2.rlock() == 42);
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, synchronous_lock_unused)
{
  extra::synch<data_wrapper> shared1(1);
  extra::synch<float>        shared2(2.f);
  std::mutex                 mutex;

  auto locks = wlock(shared1, shared2, mutex);

  auto& [handle1, handle2, lock] = locks;

  EXPECT_TRUE(handle1->get_data() == 1);
  EXPECT_TRUE(*handle2 == 2.f);
  handle1->do_things_mut();
  *handle2 += 8.f;
  EXPECT_TRUE(handle1->get_data() == 3);
  EXPECT_TRUE(*handle2 == 10.f);
  EXPECT_TRUE(lock.owns_lock());
}

TEST(synch, synchronous_lock_used_by_synch)
{
  extra::synch<data_wrapper> shared1(1);
  extra::synch<float>        shared2(2.f);
  std::mutex                 mutex;

  auto tic = std::chrono::high_resolution_clock::now();

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&shared1, &started]
    {
      auto handle = shared1.rlock();
      started     = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  auto locks = wlock(shared1, shared2, mutex);

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, synchronous_lock_used_by_mutex)
{
  extra::synch<data_wrapper> shared1(1);
  extra::synch<float>        shared2(2.f);
  std::mutex                 mutex;

  auto tic = std::chrono::high_resolution_clock::now();

  std::atomic<bool>     started{ false };
  [[maybe_unused]] auto future = std::async(
    [&mutex, &started]
    {
      std::lock_guard<decltype(mutex)> lock(mutex);
      started = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

  // Make sure the future starts.
  while (not started)
  {
    // BUSY WAITING
  }

  auto locks = wlock(shared1, shared2, mutex);

  auto tac = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(tac - tic >= std::chrono::milliseconds(100));
}

TEST(synch, synchronous_try_lock)
{
  extra::synch<data_wrapper> shared1(1);
  extra::synch<float>        shared2(2.f);
  std::mutex                 mutex;

  {
    // Try lock while nothing is used.
    auto locks = try_wlock(shared1, shared2, mutex);

    EXPECT_TRUE(locks);
    auto& [handle1, handle2, lock] = *locks;

    EXPECT_TRUE(handle1->get_data() == 1);
    EXPECT_TRUE(*handle2 == 2.f);
    handle1->do_things_mut();
    *handle2 += 8.f;
    EXPECT_TRUE(handle1->get_data() == 3);
    EXPECT_TRUE(*handle2 == 10.f);
    EXPECT_TRUE(lock.owns_lock());
  }
  {
    // Now try lock while a synch handle exists.
    std::atomic<bool> started{ false };
    auto              future = std::async(
      [&shared1, &started]
      {
        auto handle1 = shared1.rlock();
        started      = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      });

    // Make sure the future starts.
    while (not started)
    {
      // BUSY WAITING
    }

    auto locks = try_wlock(shared1, shared2, mutex);
    EXPECT_FALSE(locks);
    future.wait();
  }
  {
    // Now try lock while a mutex lock_guard exists.
    std::atomic<bool> started{ false };
    auto              future = std::async(
      [&mutex, &started]
      {
        std::lock_guard<decltype(mutex)> lock(mutex);
        started = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      });

    // Make sure the future starts.
    while (not started)
    {
      // BUSY WAITING
    }

    auto locks = try_wlock(shared1, shared2, mutex);
    EXPECT_FALSE(locks);
    future.wait();
  }
}

TEST(synch, synch_with_basic_lockable_only)
{
  extra::synch<float, basic_locable_mock> cell(2.f);

  auto handle = cell.rlock();
  EXPECT_TRUE(*handle == 2.f);
}

TEST(synch, synch_with_lockable_only)
{
  extra::synch<float, locable_mock> cell(2.f);

  {
    auto handle = cell.rlock();
    EXPECT_TRUE(*handle == 2.f);
  }
  {
    auto handle = cell.try_rlock();
    EXPECT_TRUE(handle);
  }
}

// NOLINTEND(readability-magic-numbers)
