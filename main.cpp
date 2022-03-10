#define AHL_ENABLE_AFTER_CREATE
#include <iostream>
#include <any>
#include <string>
#include <algorithm>
#include "ahl/americano.h"
#include <Windows.h>
#include "ahl/gates/optcall.h"




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




ahl::Optcall<void*> createHook(int sprite, int, int target, int menuSelector){
    //return ahl::orig<&createHook>(sprite, target, targetdup, menuSelector);
    std::cout << "Sprite: " << sprite << "\n";
    std::cout << "target: " << target << "\n";
    //std::cout << "targetdup: " << targetdup << "\n";
    std::cout << "menuSelector: " << menuSelector << "\n";

    auto ptr = ahl::detail::invoker::OriginalInvoker<&createHook, ahl::Convention::Optcall>::realPtr;
    auto val = reinterpret_cast<void* (__thiscall*)(int,int, int)>(ptr)(sprite,target,menuSelector);
    __asm add esp, 8;




    return val;
}




using namespace ahl::detail;
using namespace ahl::detail::TL;




template <typename T>
void printType() {
    std::cout << __FUNCSIG__ << '\n';
}

int _main(){
    MH_Initialize();
    ahl::addHook<&createHook>((uint32_t) createSrc);
    /*__asm push ebp;
    __asm mov ecx, 1337; //Sprite
    __asm mov edx, 555; //Target
    __asm push 456;
    __asm push 123;
    __asm call createSrc;
    __asm add esp, 16*/
    //printType<ahl::detail::Detour<decltype(createHook),ahl::Convention::Optcall>::ty>();
    
    //createSrc(123, 456); //Targetdup, Menuselector
    return 0;
   /*using test1 = TypeList<int,short,float, bool, char>;
   using test2 = TypeList<long long,short,float, float, char>;
   using test3 = TypeList<int,long long,float, float, char>;
   using test4 = TypeList<int>;
   //using test5 = TypeList<long long>;

   printType<wrapper::OptcallOpt::OptimizeFunction_t<test1>>();
   printType<wrapper::OptcallOpt::OptimizeFunction_t<test2>>();
   printType<wrapper::OptcallOpt::OptimizeFunction_t<test3>>();
   printType<wrapper::OptcallOpt::OptimizeFunction_t<test4>>();*/
   //printType<OptcallOpt::OptimizeFunction_t<test5>>();
}
DWORD WINAPI main_thread(void* hModule) {
    MH_Initialize();
    AllocConsole();
    FILE *fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
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
}
