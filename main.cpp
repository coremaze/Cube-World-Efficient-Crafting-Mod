#include "main.h"
#include <iostream>
#include <fstream>

UINT_PTR base;
float multiplier = 1.0;
float current;
float amount_to_add;
float maxcrafting = 1.0;

void DLL_EXPORT CraftingMath(){
    asm("movss _current, xmm0");
    asm("movss _amount_to_add, xmm1");

    if (multiplier <= 1.3){
        multiplier *= 1.001;
    }

    amount_to_add *= multiplier;

    asm("movss xmm1, [_amount_to_add]");
    asm("movss xmm0, [_current]");

}
DWORD CraftingMathPtr = (DWORD)&CraftingMath;

_declspec(naked) void DLL_EXPORT CraftingProgressInjection(){

    asm("call [_CraftingMathPtr]");
    asm("addss xmm0, xmm1");

    asm("mov ecx, [_base]"); //jump back
    asm("add ecx, 0x8F20E");
    asm("comiss xmm0, [_maxcrafting]");//original comparison
    asm("jmp ecx");
}

_declspec(naked) void DLL_EXPORT CraftingDone(){
    asm("mov dword ptr _multiplier, 0x3F800000"); //reset multiplier
    asm("mov byte ptr [edi + 0x3C4], 0x0");

    asm("mov esi, [_base]"); //jump back
    asm("add esi, 0x34711");
    asm("jmp esi");

}

void WriteJMP(BYTE* location, BYTE* newFunction){
	DWORD dwOldProtection;
	VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(( (unsigned INT32)newFunction - (unsigned INT32)location ) - 5);
	VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    switch (fdwReason)
    {

        case DLL_PROCESS_ATTACH:

            WriteJMP((BYTE*)(base + 0x8F203), (BYTE*)&CraftingProgressInjection);
            WriteJMP((BYTE*)(base + 0x3470A), (BYTE*)&CraftingDone);

            break;
;
    }
    return TRUE;
}
