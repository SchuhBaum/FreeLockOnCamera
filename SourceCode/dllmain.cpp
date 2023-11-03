#include <Windows.h>

#include "ModUtils.h"
#include "ini.h"

using namespace mINI;
using namespace ModUtils;

const std::string author = "SchuhBaum";
const std::string version = "0.0.1";

DWORD WINAPI MainThread(LPVOID lpParam) {
    Log("author " + author);
	Log("version " + version);
    
    std::vector<uint16_t> vanilla;
    std::vector<uint8_t> modded;
    uintptr_t assembly_location;
    
    // vanilla:
    // sets the variable that disables the free camera during lock-on to one;
    // c6 81 10030000 01    --  mov byte ptr [rcx+00000310],01
    //
    // modded:
    // sets the same variable to zero instead;
    vanilla = { 0xc6, 0x81, 0x10, 0x03, 0x00, 0x00, 0x01 };
    modded = { 0xc6, 0x81, 0x10, 0x03, 0x00, 0x00, 0x00 };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    // vanilla:
    // switching locked-on targets requires the mouse to be moved faster than a threshold speed;
    // 72 3a                --  jb <current_address+3a>
    // 0f2f 15 9abe2c02     --  comiss xmm2,<current_address+022cb39a>
    // 76 31                --  jna <current_address+31>                <-- nop this
    //
    // modded:
    // remove the jump when the threshold is not met; this is still bad since it 
    // reacts to moving the mouse rather than the exact camera position; too janky
    // for my taste;
    vanilla = { 0x72, 0x3a, 0x0f, 0x2f, 0x15, 0x9a, 0xbe, 0x2c, 0x02, 0x76, 0x31 };
    modded = { 0x72, 0x3a, 0x0f, 0x2f, 0x15, 0x9a, 0xbe, 0x2c, 0x02, 0x90, 0x90 };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);

    // vanilla:
    // initializes the lock-on angle to 0.7f (around 40? degrees); this makes many 
    // enemies lock-on candidates; switching targets just requires moving the mouse
    // rather than aiming at them; this can make things janky and you might switch
    // unintentionally;
    //
    // modded:
    // change this value to 0.25f (around 15? degrees) instead; this affects auto 
    // switching targets when they die; you lose lock-on more often;
    // 0.25f = (0)(011 1110 1)(000 0..) = 3e 80 00 00
    vanilla = { 0xc7, 0x83, 0x2c, 0x29, 0x00, 0x00, 0xc2, 0xb8, 0x32, 0x3f };
    modded = { 0xc7, 0x83, 0x2c, 0x29, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3e };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    // vanilla:
    // you have to press the lock-on key every time you lose it; this is not great in
    // combination with the change above;
    // 88 86 31 28 00 00        --  mov [rsi+00002831],al
    // 8b 0d 7a 52 ea 03        --  mov ecx,<address_offset>
    //
    // modded:
    // make it a toggle instead => prevent it from getting overriden; still not perfect
    // since you have to remember if it is toggled on or off;
    // 90 90 90 90 90 90        --  6x nop
    // 8b 0d 7a 52 ea 03        --  mov ecx,<address_offset>
    vanilla = { 0x88, 0x86, 0x31, 0x28, 0x00, 0x00, 0x8b, 0x0d, 0x7a, 0x52, 0xea, 0x03  };
    modded = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x8b, 0x0d, 0x7a, 0x52, 0xea, 0x03  };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    // vanilla:
    // this variable has to do with setting a range value; when you are in close range
    // then the lock-on relies less on the camera direction;
    // f3 0f 58 ca                  --  xmm1,XMM2       
    // f3 0f 11 8e 30 29 00 00      --  dword ptr [RSI + 0x2930],xmm1
    //
    // modded:
    // set this range value to zero;
    // c7 86 30290000 00000000      --  mov [RSI + 0x2930],0
    // 90 90                        --  2x nop
    vanilla = { 0xf3, 0x0f, 0x58, 0xca, 0xf3, 0x0f, 0x11, 0x8e, 0x30, 0x29, 0x00, 0x00 };
    modded = { 0xc7, 0x86, 0x30, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    // // vanilla:
    // // this function switches locked-on targets; it is called when you move the mouse;
    // // 48 89 5c 24 20           --  mov [rsp+20],rbx    <-- return + nops
    // // 55                       --  push rbp
    // // 56                       --  push rsi
    // // 41 57 48 8d 6c 24 90     --  lea rbp,[rsp-70]
    // //    
    // // modded:
    // // focus on the same locked-on target => skip this function by returning 
    // // immediately;
    // vanilla = { 0x48, 0x89, 0x5c, 0x24, 0x20, 0x55, 0x56, 0x41, 0x57, 0x48, 0x8d, 0x6c, 0x24, 0x90 };
    // modded = { 0xC3, 0x90, 0x90, 0x90, 0x90, 0x55, 0x56, 0x41, 0x57, 0x48, 0x8d, 0x6c, 0x24, 0x90 };
	// assembly_location = SigScan(vanilla);
    // if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    // vanilla:
    // removes the lock-on when you don't look at the target with the camera;
    // 40 0fb6 ff           --  movzx edi,dil       --  dil holds the value zero
    // 41 0f2f c3           --  comiss xmm0,xmm11
    //    
    // modded:
    // override the variable that tracks if the lock-on should be removed; there are 
    // three locations where this needs to be done;
    // 41 8b ff             --  mov edi,r15d        --  r15d holds the value one
    // 90                   --  nop
    // 41 0f2f c3           --  comiss xmm0,xmm11
    vanilla = { 0x40, 0x0f, 0xb6, 0xff, 0x41, 0x0f, 0x2f, 0xc3 };
    modded = { 0x41, 0x8b, 0xff, 0x90, 0x41, 0x0f, 0x2f, 0xc3 };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    vanilla = { 0x40, 0x0f, 0xb6, 0xff, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x43, 0xff };
    modded = { 0x41, 0x8b, 0xff, 0x90, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x43, 0xff };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    vanilla = { 0x40, 0x0f, 0xb6, 0xff, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x47, 0xff };
    modded = { 0x41, 0x8b, 0xff, 0x90, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x47, 0xff };
	assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
	CloseLog();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
	if (reason != DLL_PROCESS_ATTACH) return false;
	DisableThreadLibraryCalls(module);
	CreateThread(0, 0, &MainThread, 0, 0, NULL);
	return true;
}
