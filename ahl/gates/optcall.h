#pragma once
#include "common.h"
#include <utility>
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
    

    template<typename OriginalTypesTL, std::size_t index, typename... StackArgs>
    auto extractRealArgument(StackArgs... stackArgs){
        using targetType = TL::TypeAt_t<OriginalTypesTL, index>;
        
        //Float branch
        if constexpr(std::is_floating_point_v<targetType> && index <= 4){
            if constexpr(std::is_same_v<targetType, float>){
                float val=0;
                if constexpr(index == 0) __asm movss val, xmm0;
                else if constexpr(index == 1) __asm movss val, xmm1;
                else if constexpr(index == 2) __asm movss val, xmm2;
                else if constexpr(index == 3) __asm movss val, xmm3;
                return val;

            }else if constexpr(std::is_same_v<targetType, double>){
                double val=0;
                if constexpr(index == 0) __asm movsd val, xmm0;
                else if constexpr(index == 1) __asm movsd val, xmm1;
                else if constexpr(index == 2) __asm movsd val, xmm2;
                else if constexpr(index == 3) __asm movsd val, xmm3;
                return val;
            }
        }

        //Optimizable branch, non-floats
        if constexpr(index < 2 && sizeof(TL::TypeAt_t<OriginalTypesTL,index>)  <= 4){
            return getNthElement<index>(stackArgs...);
        }else if constexpr(index < 2 && sizeof(TL::TypeAt_t<OriginalTypesTL,index>)  > 4){
            if constexpr(index == 1 && sizeof(TL::TypeAt_t<OriginalTypesTL, index-1>) <= 4 ) return getNthElement<2>(stackArgs...);
            else if constexpr(index == 1 && sizeof(TL::TypeAt_t<OriginalTypesTL, index-1>) > 4) return getNthElement<3>(stackArgs...);
            else return getNthElement<index+2>(stackArgs...);
        }
        
        //Everything else
        //((std::cout << (stackArgs) << ", "), ...);
        //std::cout << std::endl;
        return getNthElement<index>(stackArgs...);
        //return 300;

    }





    template<typename OriginalParameters,typename FilteredParameters,typename ReturnType, typename ParamIs>
    struct OptcallGateGenerator;

    template<typename OriginalParameters,typename... FilteredParameters,typename ReturnType, std::size_t... Is>
    struct OptcallGateGenerator<OriginalParameters, TL::TypeList<FilteredParameters...>, ReturnType, std::index_sequence<Is...>>{
        template<auto fn>
        static ReturnType __fastcall wrapper_fn(FilteredParameters... args){

        };
    };

   template<typename OriginalParametersTL, typename FilteredParametersTL, typename ReturnType, typename OriginalParametersIs, bool CCER>
   struct OptcallWrapper;
   

   template<typename OriginalParametersTL, typename... FilteredParameters, typename ReturnType, std::size_t... OriginalParametersIs, bool CCER>
   struct OptcallWrapper<OriginalParametersTL, TL::TypeList<FilteredParameters...>, ReturnType, std::index_sequence<OriginalParametersIs...>, CCER>{
    private:
       using P = ConditionalWrap_t<CCER, Convention::Optcall, ReturnType(FilteredParameters...)>;
    public:
        template<P fn>
        static ReturnType __cdecl _wrapper_fn(FilteredParameters... args){
            return fn(extractRealArgument<OriginalParametersTL, OriginalParametersIs>(args...) ...);
        }
                
   };
 
}