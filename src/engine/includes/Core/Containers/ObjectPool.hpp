#pragma once 

template <typename Type>
class ObjectPool {
    using Queue = std::deque<Type>;
    using Container = std::vector<Type>;
        
public:
    void Insert(const Type& object) {
        _freeObjects.insert(object);
    };
    
    void Reset() {
        _freeObjects.insert(_freeObjects.end(), _objectsInUse.begin(), _objectsInUse.end());
        _objectsInUse.clear();
    };
    
    bool HasFreeObjects() {
        return _freeObjects.size() != 0;
    };
    
    template <typename InputIterator>
    void Insert(InputIterator begin, InputIterator end) {
        _freeObjects.insert(_freeObjects.end(), begin, end);
    };
    
    Type GetObject() {
        if(!HasFreeObjects())
            assert(0 && "No Free objects in the pool");
            return {};
        
        auto object = _freeObjects.front();
        _freeObjects.pop_front();
        
        _objectsInUse.push_back(object);
        
        return object;
    };
    
private:
    Queue _freeObjects;
    Container _objectsInUse;
};
