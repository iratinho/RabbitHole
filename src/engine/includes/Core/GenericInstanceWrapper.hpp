#pragma once

// A generic wrapper, this is very nice to create non heap allocated classes but still call them from base classes
template <typename Interface>
class GenericInstanceWrapper {
public:
    // TODO create enable if to create diff constructors for pointer and non pointer
    template <typename Implementation>
    GenericInstanceWrapper(Implementation&& concrete_render_pass)
        : storage(std::forward<Implementation>(concrete_render_pass))
        , getter([](std::any& storage) -> Interface& { return std::any_cast<Implementation&>(storage);} )
    {}

    bool Execute() {
        return (&this->getter(storage))->Execute();
    };
    
    Interface& GetRef() {
         return getter(storage); 
    }

    Interface* operator->() { return &getter(storage); }
    
    
private:
    std::any storage;
    Interface& (*getter)(std::any&);
};
