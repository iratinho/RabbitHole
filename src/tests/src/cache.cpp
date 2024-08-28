#include <iostream>
#include "gtest/gtest.h"
#include "Core/Cache/Cache.hpp"

TEST(Caching, Insertion) {

    Cache<std::size_t, float> cache;
    cache.Put(0, 1.0f);
    cache.Put(1, 2.0f);
    cache.Put(2, 3.0f);
    
    EXPECT_EQ(cache.Get(0), 1.0f); // Check if key 0 has value 1.0f
    EXPECT_EQ(cache.Get(1), 2.0f); // Check if key 1 has value 2.0f
    EXPECT_EQ(cache.Get(2), 3.0f); // Check if key 2 has value 3.0f

    // Check if the size of the cache is correct
    EXPECT_EQ(cache.Size(), 3); // There should be 3 elements in the cache
}

TEST(Caching, Update) {
    Cache<std::size_t, float> cache;

    // Insert initial value
    cache.Put(1, 1.0f);
    EXPECT_EQ(cache.Get(1), 1.0f);

    // Update the value for the same key
    cache.Put(1, 2.0f);
    EXPECT_EQ(cache.Get(1), 2.0f); // Check if the updated value is correct

    // Ensure the size of the cache remains unchanged
    EXPECT_EQ(cache.Size(), 1);
}

TEST(Caching, Remove) {
    Cache<std::size_t, float> cache;

    // Insert values into the cache
    cache.Put(0, 1.0f);
    cache.Put(1, 2.0f);
    cache.Put(2, 3.0f);
    
    // Remove a key
    cache.Remove(1);
    EXPECT_EQ(cache.Size(), 2); // Size should be 2 after removal

    // Check that the key was actually removed
    EXPECT_EQ(cache.Contains(1), false); // Expect an exception for a missing key
}

TEST(Caching, Clear) {
    Cache<std::size_t, float> cache;

    // Insert values into the cache
    cache.Put(0, 1.0f);
    cache.Put(1, 2.0f);
    cache.Put(2, 3.0f);

    // Clear the cache
    cache.Clear();

    // Check that the cache is empty
    EXPECT_EQ(cache.Size(), 0);

    // Check that trying to get any value throws an exception
    EXPECT_EQ(cache.Contains(0), false);
    EXPECT_EQ(cache.Contains(1), false);
    EXPECT_EQ(cache.Contains(2), false);
}

TEST(Caching, ExceptionSafety) {
    Cache<std::size_t, float> cache;

    // Insert a value
    cache.Put(0, 1.0f);

    // Check that getting a non-existing key throws an exception
    EXPECT_THROW(cache.Get(1), std::out_of_range);

    // Ensure that getting an existing key does not throw
    EXPECT_NO_THROW(cache.Get(0));
}

#include <thread>

TEST(Caching, Concurrency) {
    Cache<std::size_t, float> cache;

    auto insertFunction = [&cache]() {
        for (std::size_t i = 0; i < 100; ++i) {
            cache.Put(i, static_cast<float>(i));
        }
    };

    auto getFunction = [&cache]() {
        for (std::size_t i = 0; i < 100; ++i) {
            if (cache.Contains(i)) {
                try {
                    cache.Get(i);
                } catch (...) {
                    // Catch any exceptions to prevent the test from crashing
                }
            }
        }
    };

    std::thread t1(insertFunction);
    std::thread t2(getFunction);

    t1.join();
    t2.join();

    // At the end, all inserted elements should be in the cache
    EXPECT_EQ(cache.Size(), 100);
}

TEST(CacheTest, OnEraseCallback) {
    // Define variables to capture the callback invocation
    std::size_t erasedKey;
    float erasedValue;
    bool callbackCalled = false;

    // Define the callback
    auto onEraseCallback = [&](const std::size_t& key, const float value) {
        erasedKey = key;
        erasedValue = value;
        callbackCalled = true;
    };

    // Create a Cache instance with the custom callback
    Cache<std::size_t, float> cache(onEraseCallback);

    // Insert values into the cache
    cache.Put(1, 1.0f);
    cache.Put(2, 2.0f);

    // Remove a key
    cache.Remove(1);

    // Check that the callback was invoked
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(erasedKey, 1);
    EXPECT_EQ(erasedValue, 1.0f);
}
