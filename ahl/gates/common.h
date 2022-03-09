#pragma once
#include "../traits.h"
#include "../type_list.h"
#include "../call_convention_tags.h"

namespace ahl{
    //forwrard- declaration
    template<typename T, typename CallConv>
    struct BaseHook;

    namespace detail{
        
        template<typename CallConv, typename T>
        struct WrapWithCC;
        
        template<typename CallConv, typename T>
        using WrapWithCC_t = typename WrapWithCC<CallConv, T>::type;

        //Specialization: Wrap member functions
        template<typename CallConv, typename R, typename T, typename... Args>
        struct WrapWithCC<CallConv, R(T::*)(Args...)>{
            using type = BaseHook<R, CallConv>(T::*)(Args...);
        };
        
        //Specialization: Wrap nonmember functions
        template<typename CallConv, typename R, typename... Args>
        struct WrapWithCC<CallConv, R(Args...)>{
            using type = BaseHook<R, CallConv>(Args...);
        };

        template<bool Condition, typename CallConv, typename T>
        struct ConditionalWrap{
            using type = std::conditional_t<Condition, typename WrapWithCC<CallConv,T>::type, T>;
        };
        
        template<bool Condition, typename CallConv, typename T>
        using ConditionalWrap_t = typename ConditionalWrap<Condition, CallConv, T>::type;
        
        //Varidic Template Indexed Accesss
        template<std::size_t index, typename Head, typename... Tail>
        constexpr auto getNthElement(Head head, Tail... args){
            if constexpr(index == 0) return std::forward<Head>(head);
            else if constexpr(sizeof...(args) > 0) return getNthElement<index-1>(args...);
        }
    }
}
