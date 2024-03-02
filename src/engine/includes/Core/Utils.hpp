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

class Range {
public:
    class ForwardIterator {
        friend class Range;
        
    public:
        using value_type = unsigned int;
        using reference = unsigned int&;
        
    protected:
        ForwardIterator(value_type start)
            : _value(start) {
        }
    
    public:
        reference operator*() { return _value; };
        
        const ForwardIterator& operator++() {
            ++_value;
            return *this;
        }
        
        bool operator==(const ForwardIterator& other) {
            return _value == other._value;
        }
        
        bool operator!=(const ForwardIterator& other) {
            return _value != other._value;
        }
        
    private:
        value_type _value;
    };
    
public:
    Range(unsigned int begin, unsigned int end)
        : _begin(begin)
        , _end(end) {
    }
    
    const ForwardIterator& begin() { return _begin; };
    const ForwardIterator& end() { return _end; };
    
    const ForwardIterator::value_type count() const {
        return _end._value;
    }
    
private:
    ForwardIterator _begin;
    ForwardIterator _end;
};

template <typename T>
static std::vector<char> MakePaddedGPUBuffer(const T& data) {
    std::vector<char> ret;
    auto writeData = [&ret](const auto& m){
        const size_t mSize = sizeof(m);
        size_t padding = (mSize % 8 != 0 && mSize % 8 != mSize) ? (mSize + (8 - mSize % 8)) - mSize : 0;
        
        if(mSize == 4) {
            padding = 12;
        }

        const char* mBytes = reinterpret_cast<const char*>(&m);
        for (size_t i = 0; i < mSize + padding; ++i) {
            if(i < mSize) {
                ret.push_back(mBytes[i]);
            } else {
                ret.push_back('\0');
            }
        }
    };

    if constexpr (std::is_aggregate_v<T>) {
        for_each_member(data, writeData);
    } else {
        const char* bytes = reinterpret_cast<const char*>(&data);
        const size_t size = sizeof(T);
        const T* data = reinterpret_cast<const T*>(bytes);
        size_t padding = (size % 8 != 0) ? (8 - size % 8) : 0;
        
        if(size == 4) {
            padding = 12;
        }

        ret = std::vector<char>(bytes, bytes + size);
        
        data = reinterpret_cast<const T*>(ret.data());
        for (size_t i = 0; i < padding; ++i) {
                ret.push_back('\0');
        }
        data = reinterpret_cast<const T*>(ret.data());
    }
    
    return ret;
};

template <typename T>
constexpr size_t CalculateGPUDStructSize() {
    T dummy {};
    
    size_t size = 0;
    
    auto writeData = [&size](const auto& m){
        const size_t mSize = sizeof(m);
        size_t padding = (mSize % 8 != 0 && mSize % 8 != mSize) ? (mSize + (8 - mSize % 8)) - mSize : 0;
        
        if(mSize == 4) {
            padding = 12;
        }
        
        size += mSize + padding;
    };

    if constexpr (std::is_aggregate_v<T>) {
        for_each_member(dummy, writeData);
    } else {
        const size_t mSize = sizeof(T);
        size_t padding = (mSize % 8 != 0) ? (8 - mSize % 8) : 0;

        if(mSize == 4) {
            padding = 12;
        }

        size += mSize + padding;
    }

    return size;
};


// Direct graph nodes should implement this and that
template <typename T>
class DirectGraph {
    using Hash = std::size_t;
    using Stack = std::deque<unsigned int>;
    /*
        Establish adjecency
        std::unordered_map<vertex, std::vector<vertex>> adjecency;
     
     *  Ex:
     *  NodeA {inputHash: x0, outputHash: x1 }
     *  NodeB {inputHash: x1, outputHash: x2 }
     *  NodeD {inputHash: x1, outputHash: x4 }
     *  NodeE {inputHash: x2, outputHash: x5}
     *
     *
     *
     *  for each currentNode
            output = currentNode.output
            for each node
                skip: currentNode
                input = node.input
                if(node.input == output)
                    adjecency[currentNode].push_back(node)
     *
                
     *
     *
     
     
        There is another problem where to synchronize?? we need to be able to sync before executing new stuff
        
     *
     */
    using Lookup = std::unordered_map<Hash, unsigned int>;
    using Deps = std::unordered_map<unsigned int /* node */, std::vector<unsigned int> /* dep node */>;
    using Node = T;
    using Storage = std::vector<Node>;
    
public:
    void AddNode(const Node& node) {
        _nodes.emplace_back(std::move(node));
        _lookup.emplace(node.GetInputHash(), _nodes.size() - 1);
        _bIsDirty = true;
    };
    
    void Clear() {
        _stack.clear();
        _nodes.clear();
        _lookup.clear();
        _deps.clear();
    };
    
    // Returns nullptr when we reached the end
    [[nodiscard]] Node* GetNextNode() {
        if(_bIsDirty) {
            _stack.clear();
            EstablishDependencies();
            TopologicalSorting();
        }
        
        if(_stack.size() > 0) {
            unsigned int index = _stack.size() -1;
            _stack.pop_back();
            return &_nodes[index];
        }
        
        return nullptr;
    }
    
    [[nodiscard]] const Storage& GetNodes() const {
        return _nodes;
    }
    
private:
    void EstablishDependencies() {
        for(size_t i = 0; i < _nodes.size(); i++) {
            const Hash& output = _nodes[i].GetOutputHash();
            auto it = _lookup.find(output);
            if(it != _lookup.end()) {
                _deps[i].emplace_back(it->second);
            }
        }
    };
    
    void TopologicalSorting() {
        std::unordered_set<unsigned int> visited;
        for(size_t i = 0; i < _nodes.size(); i++) {
            DFS(i, visited);
        }
        
        std::reverse(_stack.begin(), _stack.end());
    };
    
    void DFS(unsigned int idx, std::unordered_set<unsigned int>& visited) {
        visited.emplace(idx);
        for (auto dIdx : _deps[idx]) {
            if(!visited.contains(dIdx)) {
                DFS(dIdx, visited);
            }
        }
        
        _stack.emplace_back(idx);
    };
    
private:
    Stack _stack;
    Lookup _lookup;
    Deps _deps;
    Storage _nodes;
    bool _bIsDirty;
};

class DAG {
public:
    using Vertex = size_t;
    using VertexDegree = uint8_t;
    using Edge = std::pair<Vertex, Vertex>;
    using Stack = std::vector<Vertex>;
    using Degrees = std::unordered_map<Vertex, uint8_t>;
    
    void MakeVertex(Vertex v) {
        _adjecency[v];
        _degress[v] = 0;
    }
    
    void MakeEdge(Edge edge) {
        _adjecency[edge.first].emplace(edge.second);
        _degress[edge.second] += 1;
    }
    
    void Sort() {
        std::vector<Vertex> tmpStack;
        
        // Gather root vertex's
        std::unordered_set<Vertex> rootVertexs;
        for(auto [vStart, connections] : _adjecency) {
            if(_degress[vStart] == 0) {
//                tmpStack.push_back(vStart);
                rootVertexs.emplace(vStart);
            }
        }
          
        // For each root
        for(auto& [key, value] : _adjecency) {
            SortDFS(key);
        }
                
        std::reverse(_stack.begin(), _stack.end());
        tmpStack.insert(tmpStack.end(), _stack.begin(), _stack.end());
        _stack = tmpStack;
    }
    
    int GetVertexDegree(Vertex v) {
        if(_degress.count(v) > 0) {
            return _degress[v];
        }
        
        return 0;
    }
    
    void ForEachSorted(std::function<void(Vertex vertex)> func) {
        for (Vertex vertex : _stack) {
            func(vertex);
        }
    }
    
private:
    void SortDFS(Vertex v) {
        // We wont process the dfs chain if we still have unprocessed connections
//        if(_degress[v] > 1) {
//            _degress[v] -= 1;
//            return;
//        }
        
        if(_visitedVertexs.count(v) > 0)
            return;
        
        _visitedVertexs.emplace(v);
        
        // Going over each vertex connection
        for(auto vConnection : _adjecency[v]) {
            // Go deep
            if(!_visitedVertexs.count(vConnection)) {
                SortDFS(vConnection);
            }
        }
        
        _stack.push_back(v);
    }
    
private:
    Stack _stack;
    Degrees _degress;
    std::unordered_set<Vertex> _visitedVertexs;
    // vertex -> (connected vertices)
    std::unordered_map<Vertex, std::unordered_set<unsigned int>> _adjecency;
};
