#pragma once
#include "traits.h"
#include "type_list.h"
#include "call_convention_tags.h"
#include "gates/thiscall.h"
#include "gates/optcall.h"
#include "gates/membercall.h"

namespace ahl{
    
    
    //forwrard- declaration
    template<typename T, typename CallConv>
    struct BaseHook;

    namespace detail{
        
            template<typename... Args>
            using msvc_optimize_args = typename TL::EraseFloatsUpToN<TL::TypeList<Args...>, 4>::result_type;

        namespace wrapper{
            
            template<typename CallConv, typename T, bool CCER=true>
            struct DetourWrapper;
            
            //=======================================
            //ThiscallWrapper - Implementer
            //=======================================

            //Thiscall Wrapper Specialization  for nonmbember functions
            template<typename R, typename T, typename... Args, bool CCER>
            struct DetourWrapper<Convention::Thiscall, R(T, Args...), CCER>{
                using  P = ConditionalWrap_t<CCER, Convention::Thiscall,R(T, Args...)>; 
                template<P fn>
                static R __fastcall wrapper_fn(T instance, void*, Args&&... args){
                    auto v = fn(instance, std::forward<Args>(args)...);
                    if constexpr(!std::is_void_v<R>) return v;
                }
            };

            //Thiscall Wrapper Specialization for member functions
            template<typename R, typename T, typename... Args, bool CCER>
            struct DetourWrapper<Convention::Thiscall, R(T::*)(Args...), CCER>{
                using  P = ConditionalWrap_t<CCER, Convention::Thiscall,  R(T::*)(Args...)>;

                template<P fn>
                static R __fastcall wrapper_fn(T* instance, void* ,Args&&... args){
                    auto v = (instance->*fn)(std::forward<Args>(args)...);
                    if constexpr(!std::is_void_v<R>) return v;
                }
            };
            
            //=======================================
            //OptcallWrapper - Implementer
            //=======================================


            template<typename R, typename... Args, bool CCER>
            struct DetourWrapper<Convention::Optcall, R(Args...), CCER> :  
                OptcallWrapper<
                    TL::TypeList<Args...>,  
                    OptcallOpt::OptimizeFunction_t<TL::TypeList<Args...>>,
                    R,
                    std::make_index_sequence<sizeof...(Args)>,
                    CCER
                    >{};

            
            //=======================================
            //MemberWrapper Implementer
            //=======================================
            
            //Non-member
            template<typename R, typename T, typename... Args, bool CCER>
            struct DetourWrapper<Convention::Membercall, R(T, Args...), CCER> :
            MembercallWrapper<
                TL::TypeList<Args...>,
                TypeListToFn_t<R, TL::Append_t<T, msvc_optimize_args<Args...> > >,
                CCER
            >{};
            
            //Member
            template<typename R, typename T, typename... Args, bool CCER>
            struct DetourWrapper<Convention::Membercall, R(T::*)(Args...), CCER> :
            MembercallWrapper<
                TL::TypeList<Args...>,
                TypeListToMemFn_t<R, T, msvc_optimize_args<Args...>>,
                CCER
            >{};
            
        }
        

        //===============================
        // Detour Class
        //===============================
        template<class, class, class=void>
        struct Detour;

        template<typename T, typename CallConv>
        struct Detour<T,CallConv, std::enable_if_t<!detail::FnEncodesCallConv_v<T>> >
         : wrapper::DetourWrapper<CallConv, typename detail::Identity_t<T>, false>{};
        
        template<typename T, typename CallConv >
        struct Detour<T,CallConv, std::enable_if_t<detail::FnEncodesCallConv_v<T>>>
         : wrapper::DetourWrapper<CallConv , typename ahl::detail::ExtractUnderlyingHookType<T>::type, true>{};

        //template<typename T, typename CallConv >
        //struct Detour<T,CallConv, std::enable_if_t<true> > : wrapper::DetourWrapper<CallConv , typename ahl::detail::ExtractUnderlyingHookType<T>::type>{};

        //template<typename T, typename CallConv>
        //Manually Specified calling convention
       /*template<typename T>
       struct Detour{

       };*/
    }
}


