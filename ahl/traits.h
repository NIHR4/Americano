#pragma once
#include <type_traits>

namespace ahl::detail{
    template<typename T>
    struct ExtractUnderlyingHookType;
    
    template<typename T>
    using ExtractUnderlyingHookType_t = typename ExtractUnderlyingHookType<T>::type;

    template<typename HookT, typename... Args>
    struct ExtractUnderlyingHookType<HookT(Args...)>{
        using type = typename HookT::value_type(Args...);
    };
    
    template<typename R, typename U, typename... Args>
    struct ExtractUnderlyingHookType<R(U::*)(Args...)>{
        using type = typename R::value_type(U::*)(Args...);
    };


    template<typename T, typename=void>
    struct ExtractCallConv;
    
    template<typename T>
    using ExtractCallConv_t = typename ExtractCallConv<T>::type;

    template<typename T, typename... Args>
    struct ExtractCallConv<T(Args...), std::void_t<typename T::callconv_type>>{
        using type = typename T::callconv_type;
    };

    template<typename T, typename U, typename... Args>
    struct ExtractCallConv<T(U::*)(Args...)>{
        using type = typename T::callconv_type;
    };

    template<typename T, typename=void>
    struct FnEncodesCallConv : std::false_type{};
    
    template<typename T>
    struct FnEncodesCallConv<T, std::void_t<ExtractCallConv_t<T>>> : std::true_type{};

    template<typename T>
    constexpr bool FnEncodesCallConv_v = FnEncodesCallConv<T>::value;

    template<typename T> 
    struct Identity{
        using type = T;
    };
    
    template<typename T> 
    using  Identity_t = typename Identity<T>::type;

    template<typename T>
    struct RemoveMemberFunctionPtr;/*{
        using type = T;
    };*/

    template<typename T>
    using RemoveMemberFunctionPtr_t = typename RemoveMemberFunctionPtr<T>::type;

    template<typename R, typename T, typename... Args>
    struct RemoveMemberFunctionPtr<R(T::*)(Args...)>{
        using type = R(*)(T*, Args...);
    };

    template<typename T>
    struct AddThiscall;
    
    template<typename T>
    using AddThiscall_t = typename AddThiscall<T>::type;

    template<typename R, typename T, typename... Args>
    struct AddThiscall<R(T, Args...)>{
        using type = R __thiscall(T, Args...);
    };
    
    template<typename R, typename... Args>
    struct AddThiscall<R(*)(Args...)>{
        using type = R (__thiscall*)( Args...);
    };

    template<typename T>
    struct AddFastall;
    
    template<typename T>
    using AddFastall_t = typename AddFastall<T>::type;

    template<typename R,typename... Args>
    struct AddFastall<R(*)(Args...)>{
        using type =  R(__fastcall*)(Args...);
    };

}
