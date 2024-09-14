#pragma once

namespace Core {
    template <typename Ret>
    struct NullExtension {
        static constexpr bool isNullExtension = true;
        using RetType = std::unique_ptr<Ret>;
    };
    
    template <typename Ret>
    struct DefaultExtension {
        static constexpr bool isNullExtension = true;
        using RetType = std::shared_ptr<Ret>;
    };
    
    template <typename Ret>
    struct NullStorage {
        static constexpr bool isNullStorage = true;
        using AllocType = NullExtension<Ret>::RetType;
        using HashType = void*;
    };
    
    /*
     *  Class that holds the storage
     * this can be moved to a more generic place, we can serve pipelines and other resources with this type of caching
     
     * Create an extension to allow the creation of the resource to happen outside
     */
    template <typename Type, typename HashT, typename Extension = DefaultExtension<Type>> // TODO: enforce isNullExtension
    class StorageCache {
    public:
        using AllocType = Extension::RetType;
        using HashType = HashT;
        using AllocContainer = std::unordered_map<HashType, AllocType>;
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
        [[nodiscard]] AllocType Store(const HashType& hash, Args&&... args) {
            if(_storage.count(hash) == 0) {
                if constexpr(Extension::isNullExtension) {
                    _storage[hash] = std::make_shared<Type>(std::forward<Args>(args)...);
                }
                else {
                    _storage[hash] = Extension::MakeResource(std::forward<Args>(args)...);
                }
            };
            
            return _storage[hash];
        };
        
        template <typename ... Args>
        [[nodiscard]] AllocType Get(const HashType& hash, Args&&... args) {
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
    template <class Type, typename StorageCache = NullStorage<Type>>
    class Factory {
    public:
        /**
        * @brief Creates a new shader
        * @param path - The path for where the shader will be loaded from disk
        * @param stage - The shader stage. (Vertex shader or Fragment shader)
        */
//        template <typename ... Args>
//        [[nodiscard]]  static StorageCache::AllocType Create(Args&&... args) {
//            return std::make_unique<Type>(std::forward<Args>(args)...);
//        };
//
        /**
        * @brief Creates a new shader
        * @param path - The path for where the shader will be loaded from disk
        * @param stage - The shader stage. (Vertex shader or Fragment shader)
        * @param hash - The hash used to storage this instance
        */
//        template <typename ... Args>
//        [[nodiscard]] static StorageCache::AllocType GetOrCreate(StorageCache::HashType hash, Args&&... args) {
//            return StorageCache::Get().Store(hash, std::forward<Args>(args)...);
//        };
    };
}
