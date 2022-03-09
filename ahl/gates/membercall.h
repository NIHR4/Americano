#pragma once
#include "common.h"

    
namespace ahl::detail::wrapper{
    
            template<typename T, typename RealArgsList>
            struct GetMembercallArgumentImpl{
                template<std::size_t index,typename... Args>
                static T getArg(Args...  stackargs){
                    constexpr std::size_t ellidedElements = CountEllidedTypesAtPos<RealArgsList, std::clamp(index,0u,3u)>;
                    constexpr std::size_t realIndex = index-ellidedElements;
                    return  getNthElement<realIndex>(stackargs...);
                }
            };
            
            //Rebase the index at "0", ECX is always used and is not optimizable so it isn't passed
            //to the optimization function
            template<typename RealArgsList>
            struct GetMembercallArgumentImpl<float, RealArgsList>{
                template<std::size_t index, typename... Args>
                static float getArg(Args... stackargs){
                    float val;
                    if constexpr(index == 0) __asm movss val, xmm1;
                    else if constexpr(index == 1) __asm movss val, xmm2;
                    else if constexpr(index == 2) __asm movss val, xmm3;
                    else
                    {
                        constexpr std::size_t ellidedElements = CountEllidedTypesAtPos<RealArgsList, std::clamp(index,0u,2u)>;
                        constexpr std::size_t realIndex = index-ellidedElements;
                        val = getNthElement<realIndex>(stackargs...);
                    }
                    return val;
                    
                }
            };
            
            template<typename RealArgsList>
            struct GetMembercallArgumentImpl<double, RealArgsList>{
                template<std::size_t index, typename... Args>
                static double getArg(Args... stackargs){
                    double val;
                    if constexpr(index == 0) __asm movsd val, xmm1;
                    else if constexpr(index == 1) __asm movsd val, xmm2;
                    else if constexpr(index == 2) __asm movsd val, xmm3;
                    else
                    {
                        constexpr std::size_t ellidedElements = CountEllidedTypesAtPos<RealArgsList, std::clamp(index,0u,2u)>;
                        constexpr std::size_t realIndex = index-ellidedElements;
                        val = getNthElement<realIndex>(stackargs...);
                    }
                    return val;
                    
                }
            };

            template<std::size_t index, typename RealArgsList>
            struct GetMembercallArgument{
                template<typename... Args>
                static auto getValue(Args... stackArgs){
                    return GetMembercallArgumentImpl<returnType,RealArgsList>::getArg<index>(stackArgs...);
                }
            private:
                using returnType = TL::TypeAt_t<RealArgsList, index>;

            };


            template<typename OriginalTypes, typename WrapperSignatures, typename Is, bool CCER>
            struct MembercallWrapperImpl;

            //Membercall Wrapper specialization for nonmember functions            
            template<
                typename OriginalTypes,
                typename R,
                typename T,
                typename... Args,
                template<typename I, I...> typename Is,
                std::size_t... Ix,
                bool CCER
            >
            struct MembercallWrapperImpl<OriginalTypes, R(T,Args...), Is<std::size_t, Ix...>, CCER>{
            private: 
            public:
                using P = BaseHook<R, Convention::Membercall>(T, Args...); //ConditionalWrap_t<CCER, Convention::Membercall, R(T,TL::TypeAt_t<OriginalTypes, Ix>...)>;
                static constexpr int argc = sizeof...(Args);
                
                template<P fn>
                static R __fastcall wrapper_fn(T instance, void*, Args... args){
                    auto v = fn(instance, GetMembercallArgument<Ix, OriginalTypes>::getValue(args...) ...);
                    if constexpr(!std::is_void_v<R>) return v;
                }
            };
            
            template<
                typename OriginalTypes,
                typename R,
                typename T,
                typename... Args,
                template<typename I, I...> typename Is,
                std::size_t... Ix,
                bool CCER
            >
            //Membercall Wrapper specialization for member functions
            struct MembercallWrapperImpl<OriginalTypes, R(T::*)(Args...), Is<std::size_t, Ix...>, CCER>{
            private: 
                using P = ConditionalWrap_t<CCER, Convention::Membercall, R(T::*)(TL::TypeAt_t<OriginalTypes, Ix>...)>;
                using LOL = int;
            public:
                template<P fn>
                static R __fastcall wrapper_fn(T* instance, void*, Args&&... args){
                    //TODO: Fwd ref
                    auto v = (instance->*fn)(GetMembercallArgument<Ix, OriginalTypes>::getValue(args...) ...);
                    if constexpr(!std::is_void_v<R>) return v;
                }
            };

            


            
            template<typename OriginalTypes, typename WrapperSignatures, bool CCER>
            struct MembercallWrapper : 
            MembercallWrapperImpl<
                OriginalTypes,
                WrapperSignatures,
                std::make_index_sequence<TL::Length_v<OriginalTypes>>,
                CCER
            >{};
}