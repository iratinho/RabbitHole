#pragma once

namespace {
    // Jenkins hash function
    uint32_t jenkins_hash(const uint8_t* key, size_t length) {
        uint32_t hash = 0;
        for (size_t i = 0; i < length; ++i) {
            hash += key[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        return hash;
    }

    template <typename T>
    void hash_combine(std::size_t& seed, const T& value) {
        seed ^= jenkins_hash(reinterpret_cast<const uint8_t*>(&value), sizeof(T)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T>
    std::size_t hash_value(const T& obj) {
        std::size_t seed = 0;
        std::apply([&seed](const auto&... members) {
            (hash_combine(seed, members), ...);
        }, std::tie(obj));
        return seed;
    }

    template <typename T>
    std::size_t hash_value(const std::vector<T>& vec) {
        std::size_t seed = 0;
        for (const auto& item : vec) {
            std::apply([&seed](const auto&... members) {
                (hash_combine(seed, members), ...);
            }, std::tie(item));
        }
        return seed;
    }

    template <typename... Args>
    std::size_t hash_value(const Args&... args) {
        std::size_t seed = 0;
        ((hash_combine(seed, hash_value(args))), ...);
        return seed;
    }
    
    // reference https://akrzemi1.wordpress.com/2020/10/01/reflection-for-aggregates/
    
    struct init {
        template <typename T>
        operator T(); // never defined
    };
    
    template <unsigned I>
    struct tag : tag<I - 1> {};
    
    template <>
    struct tag<0> {};
    
    template <typename T>
    constexpr auto size_(tag<4>) -> decltype(T{init{}, init{}, init{}, init{}}, 0u) { return 4u; }
    
    template <typename T>
    constexpr auto size_(tag<3>) -> decltype(T{init{}, init{}, init{}}, 0u) { return 3u; }
    
    template <typename T>
    constexpr auto size_(tag<2>) -> decltype(T{init{}, init{}}, 0u) { return 2u; }
    
    template <typename T>
    constexpr auto size_(tag<1>) -> decltype(T{init{}}, 0u) { return 1u; }
    
    template <typename T>
    constexpr auto size_(tag<0>) -> decltype(T{}, 0u) { return 0u; }
    
    template <typename T>
    constexpr size_t size() {
        static_assert(std::is_aggregate_v<T>);
        return size_<T>(tag<4>{}); // highest supported number
    }
    
    template <typename T, typename F>
    void for_each_member(T const& v, F f)
    {
        static_assert(std::is_aggregate_v<T>);
        
        if constexpr (size<T>() == 4u) {
            const auto& [m0, m1, m2, m3] = v;
            f(m0); f(m1); f(m2); f(m3);
        }
        else if constexpr (size<T>() == 3u) {
            const auto& [m0, m1, m2] = v;
            f(m0); f(m1); f(m2);
        }
        else if constexpr (size<T>() == 2u) {
            const auto& [m0, m1] = v;
            f(m0); f(m1);
        }
        else if constexpr (size<T>() == 1u) {
            const auto& [m0] = v;
            f(m0);
        }
    }
    
    template <typename T, typename F>
    void for_each_member_size(T const& v, F f)
    {
        static_assert(std::is_aggregate_v<T>);
        
        if constexpr (size<T>() == 4u) {
            const auto& [m0, m1, m2, m3] = v;
            f(m0, sizeof(m0)); f(m1, sizeof(m1)); f(m2, sizeof(m2)); f(m3, sizeof(m3));
        }
        else if constexpr (size<T>() == 3u) {
            const auto& [m0, m1, m2] = v;
            f(m0, sizeof(m0)); f(m1, sizeof(m1)); f(m2, sizeof(m2));
        }
        else if constexpr (size<T>() == 2u) {
            const auto& [m0, m1] = v;
            f(m0, sizeof(m0)); f(m1, sizeof(m1));
        }
        else if constexpr (size<T>() == 1u) {
            const auto& [m0] = v;
            f(m0, sizeof(m0));
        }
    }

    
    template<typename T, int N>
    class CircularBuffer {
    public:
        CircularBuffer() : currentIndex(0), size(0) {}

        void push(const T& value) {
            if (size < N) {
                buffer[size++] = value;
            } else {
                buffer[currentIndex] = value;
                currentIndex = (currentIndex + 1) % N;
            }
        }

        T& peek() {
            T& value = getCurrent();
            currentIndex = (currentIndex + 1) % size;
            return value;
        }
        
        T& peekAdvanced() {
            currentIndex = (currentIndex + 1) % size;
            return getCurrent();
        }

        T& getCurrent() {
            return buffer[currentIndex];
        }

    private:
        T buffer[N];
        int currentIndex;
        int size;
    };

}
