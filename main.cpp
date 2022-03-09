#define AHL_ENABLE_AFTER_CREATE
#include <iostream>
#include <any>
#include <string>
#include <algorithm>
#include "ahl/americano.h"
#include <Windows.h>





ahl::Membercall<int> memberfun(int ecx){
    return 5;
}
ahl::Optcall<int> optfun(float a1, float a2, std::string, int a3, bool a4){
    std::cout << a1 << " " << a2 << " " << a3 << " " << a4; 
    return 6;
}


struct Tracker{
    Tracker(){std::cout << "Tracker Default Constructor\n";}
    Tracker(const Tracker&){std::cout << "Tracker Copy Constructor\n";}
    Tracker(Tracker&&){std::cout << "Tracker Move Constructor\n";}
};

class OriginalClass{
    int x=1234;
public:
    void method(Tracker t){
        std::cout << "I am original class! I won't let anyone see my secret variable X!!!\n";
        return;
    }
};

struct EvilClass{
    int x;
    ahl::Thiscall<void> method(Tracker t){
        std::cout << "I am Evil Class and I have hacked Original Class, his variable X holds the value " << x << ". Do drugs\n";
        ahl::orig<&EvilClass::method>(this, t);
        return {};
    }
};


ahl::Thiscall<void> thisfun(OriginalClass*, Tracker t){
    std::cout << "Lmaoooo!\n";
    return {};
}

ahl::Membercall<int> memfun(void* ecx, void* e, void*, float, float, float){
    std::cout << "Detoured to membercall";
    return 5;
}


struct MC{
    ahl::Membercall<void> memfun2(float, float, float, float, float, float, float, int, float){
        return {};
    }
};



template<typename T, typename R, typename... Args>
void* void_cast(R(T::*f)(Args...))
{
    union
    {
        R(T::*pf)(Args...);
        void* p;
    };
    pf = f;
    return p;
}

ahl::Optcall<void*> createSrc(int targetdup, int menuSelector){
    std::cout << "wrong func :(\n";
    return nullptr;
}

ahl::Optcall<void*> createHook(int sprite, int target, int targetdup, int menuSelector){
    //return ahl::orig<&createHook>(sprite, target, targetdup, menuSelector);
    std::cout << "Sprite: " << sprite << "\n";
    std::cout << "target: " << target << "\n";
    std::cout << "targetdup: " << targetdup << "\n";
    std::cout << "menuSelector: " << menuSelector << "\n";
    return nullptr;
}


namespace OptcallOpt{
    using namespace ahl::detail;
    using namespace ahl::detail::TL;
    
    
    struct tagEllidedType{};
    struct tagStackPadding{};


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
    
    template<typename ParamList>
    struct PreparePadding;

    template<typename ParamList>
    using PreparePadding_t = typename PreparePadding<ParamList>::type;
    
    template<typename Head, typename... Tail>
    struct PreparePadding<TL::TypeList<Head, Tail...>>{
        using type = TL::TypeList<tagEllidedType, tagEllidedType, tagStackPadding, tagStackPadding, Head, Tail...>;
    };

    
    template<typename TList>
    struct RemoveStackPadding{
        using type = TL::EraseAll_t<TList, tagStackPadding>;
    };
    
    template<typename TList>
    using RemoveStackPadding_t = typename RemoveStackPadding<TList>::type;

    template<typename TList>
    struct RemoveArgs{
    //private: 
        using fourParamFirstHalf   = TL::ExtractSublist_t<TList,2>;
        using fourParamSecondtHalf = TL::ExtractSublist_t<TL::EraseN_t<TList,2>,2>;
        using firstHalfOpt  = TL::EraseIf_t<TL::EraseIf_t<fourParamFirstHalf, std::is_floating_point>, SmallerOrEqToFour>;
        using secondHalfOpt = TL::EraseIf_t<fourParamSecondtHalf, std::is_floating_point>;
        using trimmedList   = TL::EraseN_t<TList,4>;
    public:  
        using type = TL::Append_t<TL::Append_t<firstHalfOpt, secondHalfOpt>, trimmedList>;
        
        //using xmmopt = TL::EraseIf_t<subgroup, std::is_floating_point>;
        //using regopt = TL::EraseIf_t<subgroup, SmallerOrEqToFour>;
        //using optargs = TL::EraseIf_t<TL::EraseIf_t<fourParamFirstHalf, std::is_floating_point>, SmallerOrEqToFour>;
        //using type = TL::Append_t<
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
        using type = ReplaceTypeAt_t<ParamList,index,EllidedT>;
    };

    template<typename ParamList, typename EllidedT, size_t index>
    struct ReintroduceEllidedArgs<
        ParamList, 
        EllidedT, 
        index, 
        std::enable_if_t<!std::is_floating_point_v<EllidedT> && (sizeof(EllidedT) > 4) >>
    {
        using type = ReplaceTypeAt_t<ParamList,2+index,EllidedT>;
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
        static constexpr size_t argCount = TL::Length_v<ParamList>;
        using PaddedArgs = PreparePadding_t<RemoveArgs_t<ParamList>>;
    public:  
        using type = RemoveStackPadding_t<OptimizeFunctionImpl_t<PaddedArgs, ParamList, argCount-1>>;//std::conditional_t<argCount != 0, OptimizeFunctionImpl_t<PaddedArgs, ParamList argCount-1>, ParamList>;
    };
    
}

using namespace ahl::detail;
using namespace ahl::detail::TL;




template <typename T>
void printType() {
    std::cout << __FUNCSIG__ << '\n';
}

int main(){
    MH_Initialize();
    ahl::addHook<&createHook>((uint32_t) createSrc);
    __asm mov ecx, 1337; //Sprite
    __asm mov edx, 555; //Target
    //createSrc(123, 456); //Targetdup, Menuselector

    using tl = typename ahl::detail::TL::TypeList<int,int,float,float, int>;
    using tr = typename OptcallOpt::ExtractSublist<tl, 3>::type;
    OptcallOpt::CountElementsIf<std::is_floating_point, tl>::value;
    //std::cout << OptcallOpt::CountOptimizedParams<tl>::value;
    
    using tl2 = typename TL::TypeList<int,long long,float,bool, double, float>;
    using result =  typename OptcallOpt::RemoveArgs<tl2>::type;

    using amogus = typename TL::TypeList<int,float,bool>;
    using sus = ReplaceTypeAt_t<amogus, 2, short>;
    using gaeus = InsertTypeAt_t<amogus,1,double>;
    //printType<sus>();
    //printType<gaeus>();
    using namespace OptcallOpt;
    using target = typename TypeList<OptcallOpt::tagEllidedType, OptcallOpt::tagEllidedType, OptcallOpt::tagStackPadding, OptcallOpt::tagStackPadding,bool>;
    using step1 = OptcallOpt::RemoveStackPadding_t<OptcallOpt::ReintroduceEllidedArgs_t<target, int, 0>>;
    using step2 = OptcallOpt::RemoveStackPadding_t<OptcallOpt::ReintroduceEllidedArgs_t<target, long long, 1>>;
    using finalstep = typename OptcallOpt::OptimizeFunction<amogus>::type;
    using padamogus = OptcallOpt::RemoveArgs<amogus>::type;
    using ff = PreparePadding_t<RemoveArgs_t<amogus>>;
    printType<finalstep>();
    //printType<step1>();
    printType<ff>();
    //OptcallOpt::ReintroduceEllidedArgs<a
    //printType< typename OptcallOpt::RemoveArgs<tl2>::firstHalfOpt>();
    //printType< typename OptcallOpt::RemoveArgs<tl2>::secondHalfOpt>();
    
    /*using e1 = typename TypeList<int>;
    using e2 = typename TypeList<>;
    using e3 = typename TypeList<bool>;
    using j = Append_t<e1,e2>;
    using k = Append_t<j, e3>;
    
    using slist = typename TypeList<int,long long, float, float, short>;
    using res = EraseIf_t<slist, std::is_integral>;  
    std::cout << TL::Length_v<res> << "\n";
    std::cout << TL::CountTypeInList_v<res, float> << "\n";
    printType<res>();*/
}
/*DWORD WINAPI main_thread(void* hModule) {
    MH_Initialize();
    auto base =  (uint32_t)GetModuleHandle(0);
    //ahl::detail::ExtractUnderlyingHookType<decltype(createHook)>::type;
    MessageBoxA(0,"Injected",nullptr, MB_OK);
    //ahl::detail::invoker::OriginalInvoker<createHook, ahl::Convention::Optcall>::invoke;
    
    ahl::addHook<&createHook>(base+0x18EE0);
    //ahl::detail::Detour<decltype(createHook), ahl::Convention::Optcall>::wrapper_fn
    return 0;

}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, main_thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}*/
