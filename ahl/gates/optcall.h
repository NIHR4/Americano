#pragma once
#include "common.h"

namespace ahl::detail::wrapper{
    
    namespace OptcallOpt{
    using namespace ahl::detail;
    using namespace ahl::detail::TL;
    
    
    template<typename T>
    struct SmallerOrEqToFour{
        static constexpr bool value = sizeof(T) <= 4;
    };

    
    template<typename TList>
    struct CountOptimizedParams{
    private:
        using twoParamGroup = TL::ExtractSublist_t<TList, 2>;
        using fourParamGroup = TL::ExtractSublist_t<TList, 4>;
    public:
    
        //Count type in list could be rewritten using CountElementsIf
        static constexpr int value = TL::CountTypeInList_v<fourParamGroup,float>  +  
                                     TL::CountTypeInList_v<fourParamGroup,double> +
                                     TL::CountElementsIf<SmallerOrEqToFour,twoParamGroup>::value;
    };
    

    template<typename TList>
    struct OptimizeCC{
    private: 
        using fourParamFirstHalf   = TL::ExtractSublist_t<TList,2>;
        using fourParamSecondtHalf = TL::ExtractSublist_t<TL::EraseN_t<TList,2>,2>;
        using firstHalfOpt  = TL::EraseIf_t<TL::EraseIf_t<fourParamFirstHalf, std::is_floating_point>, SmallerOrEqToFour>;
        using secondHalfOpt = TL::EraseIf_t<fourParamSecondtHalf, std::is_floating_point>;
        using trimmedList   = TL::EraseN_t<TList,4>;
    public:  
        using type = TL::Append_t<TL::Append_t<firstHalfOpt, secondHalfOpt>, trimmedList>;
    };
    
}

    

    
    /*template<typename T, typename RealArgsList>
    struct GetOptcallArgumentImpl{
        template<std::size_t index,typename... Args>
        static T getArg(Args...  stackargs){
            T val;
            if constexpr(sizeof(T) <= 4 && index == 0) __ asm mov val, ecx;
            else if constexpr(sizeof(T) <= 4 && index == 0) __asm mov val, edx;
            else
            {
                constexpr std::size_t ellidedElements = CountEllidedTypesAtPos<RealArgsList, std::clamp(index,0u,4u)>;
                constexpr std::size_t realIndex = index-ellidedElements;
                val = getNthElement<realIndex>(stackargs...);
            }
            return val;
        }
    };

    template<typename RealArgsList>
    struct GetOptcallArgumentImpl<float, RealArgsList>{
        template<std::size_t index, typename... Args>
        static float getArg(Args... stackargs){
            float val;
            if constexpr(index == 0) __asm movss val, xmm0;
            else if constexpr(index == 1) __asm movss val, xmm1;
            else if constexpr(index == 2) __asm movss val, xmm2;
            else if constexpr(index == 3) __asm movss val, xmm3;
            else
            {
                constexpr std::size_t ellidedElements = CountEllidedTypesAtPos<RealArgsList, std::clamp(index,0u,4u)>;
                constexpr std::size_t realIndex = index-ellidedElements;
                val = getNthElement<realIndex>(stackargs...);
            }
            return val;
            
        }
    };
    
    template<typename RealArgsList>
    struct GetOptcallArgumentImpl<double, RealArgsList>{
        template<std::size_t index, typename... Args>
        static double getArg(Args... stackargs){
            double val;
            if constexpr(index == 0) __asm movss val, xmm0;
            else if constexpr(index == 1) __asm movsd val, xmm1;
            else if constexpr(index == 2) __asm movsd val, xmm2;
            else if constexpr(index == 3) __asm movsd val, xmm3;
            else
            {
                constexpr std::size_t ellidedElements = CountEllidedTypesAtPos<RealArgsList, std::clamp(index,0u,4u)>;
                constexpr std::size_t realIndex = index-ellidedElements;
                val = getNthElement<realIndex>(stackargs...);
            }
            return val;
            
        }
    };

    template<std::size_t index, typename RealArgsList>
    struct GetOptcallArgument{
        template<typename... Args>
        static auto getValue(Args... stackArgs){
            return GetOptcallArgumentImpl<returnType,RealArgsList>::getArg<index>(stackArgs...);
        }
    private:
        using returnType = TL::TypeAt_t<RealArgsList, index>;

    };*/

    //=======================================
    //OptcallWrapper - Implementation
    //=======================================
    template<typename OriginalTypes, typename WrapperSignature, typename Is, bool CCER>
    struct OptcallWrapperImpl;
    
    template<
        typename OriginalTypes, 
        typename R, 
        typename... Args, 
        template<typename I, I...> typename Is, 
        std::size_t... Ix,
        bool CCER
    >
    struct OptcallWrapperImpl<OriginalTypes, R(Args...), Is<std::size_t, Ix...>, CCER>{
    private: 
        using P = ConditionalWrap_t<CCER, Convention::Optcall,R(TL::TypeAt_t<OriginalTypes,Ix>...)>;
    public:
        template<P fn>
        static R wrapper_fn(Args... args){
            return 0;
            //return fn(GetOptcallArgument<Ix, OriginalTypes>::getValue(args...) ... );
        }
    };

    template<typename OriginalTypes, typename WrapperSignature, bool CCER>
        struct OptcallWrapper : OptcallWrapperImpl<
            OriginalTypes, 
            WrapperSignature, 
            std::make_index_sequence<TL::Length_v<OriginalTypes>>,
            CCER 
            >{};

}