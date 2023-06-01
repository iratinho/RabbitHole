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

}
