#pragma once

namespace Core {
    template <typename Key, typename... Args>
    class CacheDefaultPolicy {
    public:
        CacheDefaultPolicy() = default;

        void Insert(const Key& key) noexcept {};
        void Get(const Key& key) noexcept {};
        void Update(const Key& key) noexcept {};
        void Erase(const Key& key) noexcept {};
        void Clear() noexcept {};
    };

    template <typename Key, std::size_t MaxAccessCount = 100>
    class CacheLRUPolicy {

        using Container = std::unordered_map<Key, std::size_t>;

    public:
        CacheLRUPolicy() = default;

        void Insert(const Key& key) noexcept {
            _access[key] = 0;
        };

        void Get(const Key& key) noexcept {
            _access[key] = 0;
            Increment(key);
        };

        void Update(const Key& key) noexcept {
            Increment(key);
        };

        void Erase(const Key& key) noexcept {
            _access.erase(key);
        };

        void Clear() noexcept {
            _access.clear();
        };

    private:
        void Increment(const Key& key) noexcept {
            for (auto& [cKey, count] : _access) {
                if (cKey != key) {
                    count++;
                }
            }

            // After Increment evict if necessary
            Evict();
        };

        void Evict() noexcept {
            std::for_each(_access.begin(), _access.end(), [&](const Key& key, std::size_t count) {
                if (count > MaxAccessCount) {
                    _access.erase(key);
                }
            });
        };

    private:
        Container _access;
    };

    template <typename Key,  typename ValueType, typename InvictionPolicy = CacheDefaultPolicy<Key>>
    class Cache
    {
        template <typename T>
        struct ValueWrapped {
            using Type = std::conditional_t<std::is_trivial<T>::value, T, std::shared_ptr<T>>;
        };
            
        using Value = ValueWrapped<ValueType>::Type;
        using Container = std::unordered_map<Key, Value>;
        using Const_Iterator = typename Container::const_iterator;
        using Iterator = typename Container::iterator;
        using OnErase = std::function<void(const Key&, const Value)>;

    public:
        Cache(const OnErase& onErase = [](const Key&, const Value) {})
            : _onErase(onErase) {
        };

        ~Cache() noexcept {
            Clear();
        };
        
        void Put(const Key& key, const Value& value, bool bForceUpdate = false) noexcept {
            if (!Contains(key))
            {
                Insert(key, std::move(value));
                return;
            }

            if(bForceUpdate)
                Update(key, std::move(value));
        };

        Value Get(const Key& key) {
            return GetInternal(key);
        };

        bool Contains(const Key& key) const noexcept {
            return Find(key) != _cache.end();
        };

        void Remove(const Key& key)
        {
            Erase(key);
        };
        
        void Clear() noexcept {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            for (auto& [key, value] : _cache) {
                _onErase(key, value);
            }
            
            _cache.clear();
        };

        std::size_t Size() noexcept {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _cache.size();
        };

        Const_Iterator begin() const noexcept {
            return _cache.begin();
        };

        Const_Iterator end() const noexcept {
            return _cache.end();
        };

        Iterator begin() noexcept {
            return _cache.begin();
        };

        Iterator end() noexcept {
            return _cache.end();
        };

    protected:
        // TODO: Show i try to make this noexcept? Might be a problem in busy loops. Profile First
        Value GetInternal(const Key& key) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            // Check if the key exists in the cache
            if (_cache.find(key) == _cache.end()) {
                throw std::out_of_range("Key not found in the cache.");
            }

            return _cache[key];
        };

        void Insert(const Key& key, const Value&& value) noexcept {
            std::lock_guard<std::mutex> lock(m_mutex);
            _cache.emplace(key, std::move(value));
        };

        void Update(const Key& key, const Value&& value) noexcept {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (_cache.find(key) != _cache.end()) {
                _cache[key] = std::move(value);
            }
        };

        Iterator Find(const Key& key) noexcept {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _cache.find(key);
        };
        
        Const_Iterator Find(const Key& key) const noexcept {
            std::lock_guard<std::mutex> lock(m_mutex);
            return _cache.find(key);
        };

        void Erase(const Key& key) {
            std::lock_guard<std::mutex> lock(m_mutex);

            _onErase(key, _cache[key]);
            _cache.erase(key);
        };

    private:
        Container _cache;
        OnErase _onErase;
        InvictionPolicy _invictionPolicy;
        mutable std::mutex m_mutex;
    };

};
