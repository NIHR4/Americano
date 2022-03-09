#pragma once
#include <algorithm>
#include <utility>
#include <MinHook.h>

#include "traits.h"
#include "type_list.h"
#include "gate_wrapper.h"
#include "original_wrapper.h"
#include "call_convention_tags.h"
namespace ahl{
 
    template<typename, typename>
    struct BaseHook;

    
    template<typename T, typename CallConv>
    struct BaseHook{
        using value_type = T;
        using callconv_type = CallConv;
        
        operator T&() {return _wrappee;}
        BaseHook(const T& val) : _wrappee(val){}
        BaseHook(T&& val) : _wrappee(std::move(val)) {};
        BaseHook(BaseHook&&) = default;
        BaseHook(const BaseHook&) = default;
        T& get() {return _wrappee;}
    private:
        T _wrappee;
    };

    template<typename CallConv>
    struct BaseHook<void, CallConv>{
        using value_type = void;
        using callconv_type = CallConv;
        
        operator value_type() {}
        BaseHook()=default;
        BaseHook(BaseHook&&) = default;
        BaseHook(const BaseHook&) = default;
        value_type get() {}
    private:
    };

    template<typename T> using Thiscall = BaseHook<T, Convention::Thiscall>;    
    template<typename T> using Optcall = BaseHook<T, Convention::Optcall>;
    template<typename T> using Membercall = BaseHook<T, Convention::Membercall>;


    template<auto DetourFunc, typename CallConv>
    void addHook(uint32_t hookedAddress){
        auto hookAddressPtr = reinterpret_cast<void*>(hookedAddress);
        MH_CreateHook(
            hookAddressPtr,
            detail::Detour<std::remove_pointer_t<decltype(DetourFunc)>, CallConv>::wrapper_fn<DetourFunc>,
            reinterpret_cast<void**>(&detail::invoker::OriginalInvoker<DetourFunc,CallConv>::realPtr)
        );

        #ifdef AHL_ENABLE_AFTER_CREATE
            MH_EnableHook(hookAddressPtr);
        #endif
    }

    template<auto DetourFunc>
    void addHook(uint32_t hookedAddress){
        using DeTy = std::remove_pointer_t<decltype(DetourFunc)>;
        static_assert(detail::FnEncodesCallConv_v<DeTy> , "Could not deduce calling convention from function");
        addHook<DetourFunc, detail::ExtractCallConv_t<DeTy>>(hookedAddress);
    };

    template<auto DetourFunc, typename... Args>
    auto orig(Args... args){
        using callconv = detail::ExtractCallConv_t<std::remove_pointer_t<decltype(DetourFunc)>>;
        return detail::invoker::OriginalInvoker<DetourFunc,callconv>::invoke(args...);
    }
}