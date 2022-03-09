#pragma once
#include "traits.h"
#include "type_list.h"

#include "call_convention_tags.h"

namespace ahl::detail::invoker
{
    template<auto Fn, typename CallConv>
    struct OriginalInvoker;
    //================================================
    // Real Pointer
    //================================================

    /*template<auto Fn>
    struct RealFnPtr{
    private:
        using HookT =  ExtractUnderlyingHookType_t<std::remove_pointer_t<decltype(Fn)>>;
    public:
        using PtrT =  std::conditional_t<std::is_member_function_pointer_v<HookT>,  HookT,  std::add_pointer_t<HookT>>;
        inline static PtrT realPtr=nullptr; 
    };*/
    
    template<typename T, typename Enable=void>
    struct PointerImpl;

    template<typename T>
    struct PointerImpl<T, std::enable_if_t<!std::is_member_function_pointer_v<T>> >
    {
        using type = std::add_pointer_t<T>;
    };
    
    template<typename T>
    struct PointerImpl<T, std::enable_if_t<std::is_member_function_pointer_v<T>> >
    {
        using type = RemoveMemberFunctionPtr_t<T>;
    };


    template<auto Fn>
    struct Pointer{
    private: 
        
    
    public:  
        //using type   = std::conditional_t<std::is_member_function_pointer_v<decltype(Fn)>, RemoveMemberFunctionPtr_t<HookTy>,  std::add_pointer_t<HookTy>>;
        using HookTy = ExtractUnderlyingHookType_t<std::remove_pointer_t<decltype(Fn)>>;
        using type   = typename PointerImpl<HookTy>::type;
    };

    template<auto Fn>
    using Pointer_t = typename Pointer<Fn>::type;


    //================================================
    // Thiscall Invoker Class
    //================================================
    template<auto Fn>
    struct OriginalInvoker<Fn,Convention::Thiscall>{
    private:
        using basePtr   = typename Pointer<Fn>::type;
        using PointerTy = AddThiscall_t<basePtr>;
    public:
        inline static  PointerTy realPtr;

        template<typename... Args>
        static auto invoke(Args&&... args){
            return realPtr(std::forward<Args>(args)...);
            //invokeImpl<std::is_member_function_pointer_v<PtrT>>(std::forward<Args>(args)...);
        }

        /*template<bool IsMember, std::enable_if_t<IsMember,bool> = true, typename T, typename... Args>
        static auto invokeImpl(T* instance, Args&&... args){
            return (instance->*realPtr)(std::forward<Args>(args)...);
        }
                
        template<bool IsMember, std::enable_if_t<!IsMember,bool> = true, typename T, typename... Args>
        static auto invokeImpl(T instance, void* ,Args&&... args){
            realPtr(instance, std::forward<Args>(args)...);
        }*/
    };  

    //==================================
    // Optcall Invoker Class
    //==================================
    template<std::size_t MaxArgs, std::size_t ArgIndex,typename Callable, typename Head, typename... Args >
    constexpr auto invokeWithOptcallImpl(Callable fn, Head&& headArg, Args&&... args){
        if constexpr(ArgIndex < 4 &&  std::is_floating_point_v<Head>){
            //Floating point removal branch
            if constexpr(std::is_same_v<std::remove_cv_t<Head>, float>){
                if constexpr(ArgIndex == 0) __asm movss xmm0, headArg;
                if constexpr(ArgIndex == 1) __asm movss xmm1, headArg;
                if constexpr(ArgIndex == 2) __asm movss xmm2, headArg;
                if constexpr(ArgIndex == 3) __asm movss xmm3, headArg;
                if constexpr(ArgIndex == MaxArgs) return fn( std::forward<args>(args...));
                else return invokeWithOptcallImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)...);
            }else{
                //Double fp removal branch
                if constexpr(ArgIndex == 0) __asm movsd xmm0, headArg;
                if constexpr(ArgIndex == 1) __asm movsd xmm1, headArg;
                if constexpr(ArgIndex == 2) __asm movsd xmm2, headArg;
                if constexpr(ArgIndex == 3) __asm movsd xmm3, headArg;
                if constexpr(ArgIndex == MaxArgs) return fn( std::forward<args>(args...));
                else return invokeWithOptcallImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)...);
            }
        }

        if constexpr(ArgIndex < 2 && sizeof(Head) <= 4){
            if constexpr (ArgIndex == 0) __asm mov ecx, headArg;
            if constexpr (ArgIndex == 1) __asm mov edx, headArg;
            if constexpr(ArgIndex == MaxArgs) return fn( std::forward<args>(args...));
            else return invokeWithOptcallImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)...);
        }


        if constexpr(ArgIndex == MaxArgs) return fn(std::forward<Head>(headArg), std::forward<Args>(args)...);
        else return invokeWithOptcallImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)..., std::forward<Head>(headArg));
    }

    template<typename Callable, typename... Args>
    auto invokeWithOptcall(Callable fn, Args... args){
        return invokeWithOptcallImpl<sizeof...(args),0>(fn, std::forward<Args>(args)...);
    }

    template<auto Fn>
    struct OriginalInvoker<Fn,Convention::Optcall> {
    private:
        using PointerTy   = typename Pointer<Fn>::type;
    public:
        inline static  PointerTy realPtr;
        template<typename... Args>
        static auto invoke(Args&&... args){
            return invokeWithOptcall([&](auto&&... fwArgs){return realPtr(args...);}, std::forward<Args>(args)... );
        }
    };

    //==================================
    // Membercall Invoker Class
    //==================================
    template<std::size_t MaxArgs, std::size_t ArgIndex,typename Callable,  typename Head, typename... Args >
    auto invokeWithMemberImpl( Callable fn, Head&& headArg, Args&&... args){
        if constexpr(ArgIndex < 3 &&  std::is_floating_point_v<Head>){
            //Floating point removal branch
            if constexpr(std::is_same_v<std::remove_cv_t<Head>, float){
                if constexpr(ArgIndex == 0) __asm movss xmm1, headArg;
                if constexpr(ArgIndex == 1) __asm movss xmm2, headArg;
                if constexpr(ArgIndex == 2) __asm movss xmm3, headArg;
                if constexpr(ArgIndex == MaxArgs) return fn( std::forward<args>(args...));
                else return invokeWithMemberImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)...);
            }else{
                //Double fp removal branch
                if constexpr(ArgIndex == 0) __asm movsd xmm1, headArg;
                if constexpr(ArgIndex == 1) __asm movsd xmm2, headArg;
                if constexpr(ArgIndex == 2) __asm movsd xmm3, headArg;
                if constexpr(ArgIndex == MaxArgs) return fn( std::forward<args>(args...));
                else return invokeWithMemberImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)...);
            }
        }
        if constexpr(ArgIndex == MaxArgs) return fn(std::forward<Head>(headArg), std::forward<Args>(args)...);
        else return invokeWithMemberImpl<MaxArgs, ArgIndex+1>(fn, std::forward<Args>(args)..., std::forward<Head>(headArg));
    }

    template<typename Callable, typename... Args>
    auto invokeWithMembercall(Callable fn, Args... args){
        return invokeWithMembercallImpl<sizeof...(args),0>(fn, std::forward<Args>(args)...);
    }

    template<auto Fn>
    struct OriginalInvoker<Fn,Convention::Membercall>{
    private:
        using basePtr   = typename Pointer<Fn>::type;
        using PointerTy = AddThiscall_t<basePtr>;
    public:
        inline static PointerTy realPtr;
        template<typename Instance, typename... Args>
        static auto invoke(Instance instance, Args&&... args){
            return invokeWithMembercall([&](auto&&... fwArgs){return realPtr(instance, args...);});
        }
    };

} // namespace ahl::detail::invoker
