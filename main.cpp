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
        using type = ReplaceTypeAt_t<ParamList, index, EllidedT>;
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

   using test1 = TypeList<int,short,float, bool, char>;
   using test2 = TypeList<long long,short,float, float, char>;
   using test3 = TypeList<int,long long,float, float, char>;
   using test4 = TypeList<int>;
   //using test5 = TypeList<long long>;

   printType<OptcallOpt::OptimizeFunction_t<test1>>();
   printType<OptcallOpt::OptimizeFunction_t<test2>>();
   printType<OptcallOpt::OptimizeFunction_t<test3>>();
   printType<OptcallOpt::OptimizeFunction_t<test4>>();
   //printType<OptcallOpt::OptimizeFunction_t<test5>>();
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
