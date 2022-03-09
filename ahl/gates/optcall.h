#pragma once
#include "common.h"

namespace ahl::detail::wrapper{
    
    namespace OptcallOpt{
        struct tagEllidedType{};
        struct tagStackPadding{};


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
        
        template<typename ParamList>
        struct PreparePadding;

        template<typename ParamList>
        using PreparePadding_t = typename PreparePadding<ParamList>::type;
        
        template<typename... Params>
        struct PreparePadding<TL::TypeList<Params...>>{
            using type = TL::TypeList<tagEllidedType, tagEllidedType, tagStackPadding, tagStackPadding, Params...>;
        };
        
        template<typename TList>
        struct RemoveStackPadding{
            using type = TL::EraseAll_t<TList, tagStackPadding>;
        };
        
        template<typename TList>
        using RemoveStackPadding_t = typename RemoveStackPadding<TList>::type;

        template<typename TList>
        struct RemoveArgs{
        private:
            using trimmed   = TL::EraseN_t<TList,4>;
            using params2T4 = TL::ExtractSublist_t<TL::EraseN_t<TList,2>,2>;
        public:  
            using type = TL::Append_t<TL::EraseIf_t<params2T4, std::is_floating_point>, trimmed>;
        };

        template<typename TList>
        using RemoveArgs_t = typename RemoveArgs<TList>::type;

        template<typename ParamList, typename EllidedT, size_t index, typename=void>
        struct ReintroduceEllidedArgs;
        
        template<typename ParamList, typename EllidedT, size_t index, typename=void>
        using ReintroduceEllidedArgs_t = typename ReintroduceEllidedArgs<ParamList, EllidedT, index>::type;
        
        template<typename ParamList, typename EllidedT, size_t index>
        struct ReintroduceEllidedArgs<
            ParamList, 
            EllidedT, 
            index, 
            std::enable_if_t<std::is_floating_point_v<EllidedT>>>
        {
            using type = ParamList;
        };
        
        template<typename ParamList, typename EllidedT, size_t index>
        struct ReintroduceEllidedArgs<
            ParamList, 
            EllidedT, 
            index, 
            std::enable_if_t<!std::is_floating_point_v<EllidedT> && (sizeof(EllidedT) <= 4) >>
        {
            using type = TL::ReplaceTypeAt_t<ParamList, index, EllidedT>;
        };

        template<typename ParamList, typename EllidedT, size_t index>
        struct ReintroduceEllidedArgs<
            ParamList, 
            EllidedT, 
            index, 
            std::enable_if_t<!std::is_floating_point_v<EllidedT> && (sizeof(EllidedT) > 4) >>
        {
            using type = TL::ReplaceTypeAt_t<ParamList,2+index,EllidedT>;
        };

        template<typename TargetParamList, typename OrigParamList, size_t index>
        struct OptimizeFunctionImpl;
        
        template<typename TargetParamList, typename OrigParamList, size_t index>
        using OptimizeFunctionImpl_t = typename OptimizeFunctionImpl<TargetParamList, OrigParamList, index>::type;
        
        template<typename TargetParamList, typename OrigParamList, size_t index>
        struct OptimizeFunctionImpl{
            using type = OptimizeFunctionImpl_t<ReintroduceEllidedArgs_t<TargetParamList, TL::TypeAt_t<OrigParamList, index>,index>, OrigParamList, index-1>;
        };

        template<typename TargetParamList, typename OrigParamList>
        struct OptimizeFunctionImpl<TargetParamList, OrigParamList, 0>{
            using type = ReintroduceEllidedArgs_t<TargetParamList, TL::TypeAt_t<OrigParamList,0>,0>;
        };

        template<typename ParamList>
        struct OptimizeFunction{
        private: 
            static constexpr size_t argCount = std::clamp(TL::Length_v<ParamList>, 0u, 2u);//TL::Length_v<ParamList>;
            using PaddedArgs = PreparePadding_t<RemoveArgs_t<ParamList>>;
        public:  
            using type = RemoveStackPadding_t<OptimizeFunctionImpl_t<PaddedArgs, ParamList, argCount-1>>;//std::conditional_t<argCount != 0, OptimizeFunctionImpl_t<PaddedArgs, ParamList argCount-1>, ParamList>;
        };
        
        template<>
        struct OptimizeFunction<TL::TypeList<>>{
            using type = TL::TypeList<>;
        };

        template<typename ParamList>
        using OptimizeFunction_t = typename OptimizeFunction<ParamList>::type;
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
    /*template<typename OriginalTypes, typename WrapperSignature, typename Is, bool CCER>
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
    */

   
}