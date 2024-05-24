#pragma once

namespace Core {
    struct NullStorage {
        static constexpr bool isNullStorage = true;
    };
    
    /*
     *  Class that holds the storage
     * this can be moved to a more generic place, we can serve pipelines and other resources with this type of caching
     */
    template <typename StorageType, typename StorageHashT>
    class StorageCache {
    public:
        using HashType = StorageHashT;
        using AllocContainer = std::unordered_map<StorageHashT, std::shared_ptr<StorageType>>;
        static constexpr bool isNullStorage = false;

    public:
        [[nodiscard]] static StorageCache& Get() {
            static StorageCache instance;
            return instance;
        }
        
        [[nodiscard]] const AllocContainer& GetAllEntries() {
            return _storage;
        }
                   
        template <typename ... Args>
        [[nodiscard]] std::shared_ptr<StorageType> Store(const HashType& hash, Args&&... args) {
            if(_storage.count(hash) == 0) {
                _storage[hash] = std::make_shared<StorageType>(std::forward<Args>(args)...);
                return _storage[hash];
            };
            
            return _storage[hash];
        };
        
        template <typename ... Args>
        [[nodiscard]] std::shared_ptr<StorageType> Get(const HashType& hash, Args&&... args) {
            if(_storage.count(hash) == 0) {
                assert(0 && "Trying to get something that do no exist in cahce");
            };
            
            return _storage[hash];
        };
        


    private:
        AllocContainer _storage;
    };
    
    /*
     * Factory to create new shaders
     */
    template <class Type, typename StorageCache = NullStorage>
    class Factory {
    public:
        /**
        * @brief Creates a new shader
        * @param path - The path for where the shader will be loaded from disk
        * @param stage - The shader stage. (Vertex shader or Fragment shader)
        */
        template <class T = Type /* unique needs dependent T */, typename ... Args>
        [[nodiscard]]  static std::enable_if_t<StorageCache::isNullStorage, std::unique_ptr<T>> Create(Args&&... args) {
            return std::make_unique<Type>(std::forward<Args>(args)...);
        };

        /**
        * @brief Creates a new shader
        * @param path - The path for where the shader will be loaded from disk
        * @param stage - The shader stage. (Vertex shader or Fragment shader)
        * @param hash - The hash used to storage this instance
        */
        template <typename ... Args>
        [[nodiscard]] static std::enable_if_t<!StorageCache::isNullStorage, std::shared_ptr<Type>> GetOrCreate(StorageCache::HashType hash, Args&&... args) {
            return StorageCache::Get().Store(hash, std::forward<Args>(args)...);
        };
        
        template <typename ... Args>
        [[nodiscard]] static std::enable_if_t<!StorageCache::isNullStorage, std::shared_ptr<Type>> Get(StorageCache::HashType hash, Args&&... args) {
            return StorageCache::Get().Get(hash, std::forward<Args>(args)...);
        };
    };
}
