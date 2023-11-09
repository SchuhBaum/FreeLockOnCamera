#include <Windows.h>

#include "ModUtils.h"
#include "ini.h"

using namespace mINI;
using namespace ModUtils;

const std::string author = "SchuhBaum";
const std::string version = "0.0.4";

void LogSeparator() {
    Log("----------");
}

void ApplyFreeLockOnCameraMod() {
    Log("ApplyFreeLockOnCameraMod");
    
    // vanilla:
    // sets the variable that disables the free camera during lock-on to one;
    // c6 81 10030000 01    --  mov byte ptr [rcx+00000310],01
    //
    // modded:
    // sets the same variable to zero instead;
    std::vector<uint16_t> vanilla = { 0xc6, 0x81, 0x10, 0x03, 0x00, 0x00, 0x01 };
    std::vector<uint8_t> modded = { 0xc6, 0x81, 0x10, 0x03, 0x00, 0x00, 0x00 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplyLockOnSensitivityMod() {
    Log("ApplyLockOnSensitivityMod");
    
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
    std::vector<uint16_t> vanilla = { 0x72, 0x3a, 0x0f, 0x2f, 0x15, 0x9a, 0xbe, 0x2c, 0x02, 0x76, 0x31 };
    std::vector<uint8_t> modded = { 0x72, 0x3a, 0x0f, 0x2f, 0x15, 0x9a, 0xbe, 0x2c, 0x02, 0x90, 0x90 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplyReduceLockOnAngleMod() {
    Log("ApplyReduceLockOnAngleMod");
    
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
    std::vector<uint16_t> vanilla = { 0xc7, 0x83, 0x2c, 0x29, 0x00, 0x00, 0xc2, 0xb8, 0x32, 0x3f };
    std::vector<uint8_t> modded = { 0xc7, 0x83, 0x2c, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplyLockOnToggleMod() {
    Log("ApplyLockOnToggleMod");
    
    // vanilla:
    // you have to press the lock-on key every time you lose it; this is not great when
    // you can move the camera freely;
    // 88 86 31 28 00 00        --  mov [rsi+00002831],al
    // 8b 0d 7a 52 ea 03        --  mov ecx,<address_offset>
    //
    // modded:
    // make it a toggle instead => prevent it from getting overriden; still not perfect
    // since you have to remember if it is toggled on or off;
    // 90 90 90 90 90 90        --  6x nop
    // 8b 0d 7a 52 ea 03        --  mov ecx,<address_offset>
    std::vector<uint16_t> vanilla = { 0x88, 0x86, 0x31, 0x28, 0x00, 0x00, 0x8b, 0x0d, 0x7a, 0x52, 0xea, 0x03  };
    std::vector<uint8_t> modded = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x8b, 0x0d, 0x7a, 0x52, 0xea, 0x03  };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();

    // vanilla:
    // you can only toggle the lock-on off when you currently are locked-on;
    // 80 b9 30 28 00 00 00     --  cmp byte ptr [rcx+00002830],00
    // 0f 94 c0                 --  sete al
    //
    // modded:
    // ignore the variable that checks if you are currently locked-on and
    // instead check the toggle variable;
    // 80 b9 31 28 00 00 00     --  cmp byte ptr [rcx+00002831],00
    // 0f 94 c0                 --  sete al
    vanilla = { 0x80, 0xb9, 0x30, 0x28, 0x00, 0x00, 0x00, 0x0f, 0x94, 0xc0 };
    modded = { 0x80, 0xb9, 0x31, 0x28, 0x00, 0x00, 0x00, 0x0f, 0x94, 0xc0 };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();

    // vanilla:
    // you lose your toggle after performing a critical hit;
    // e8 59 e8 91 ff           --  call <+ff91e859>
    // 84 c0                    --  test al,al
    // 74 41                    --  je <+41>
    //
    // modded:
    // skip the if-block by jumping always;
    // e8 59 e8 91 ff           --  call <+ff91e859>
    // 84 c0                    --  test al,al
    // eb 41                    --  jmp <+41>
    vanilla = { 0xe8, 0x59, 0xe8, 0x91, 0xff, 0x84, 0xc0, 0x74, 0x41 };
    modded = { 0xe8, 0x59, 0xe8, 0x91, 0xff, 0x84, 0xc0, 0xeb, 0x41 };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplyLockOnCloseRangeMod() {
    Log("ApplyLockOnCloseRangeMod");
    
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
    std::vector<uint16_t> vanilla = { 0xf3, 0x0f, 0x58, 0xca, 0xf3, 0x0f, 0x11, 0x8e, 0x30, 0x29, 0x00, 0x00 };
    std::vector<uint8_t> modded = { 0xc7, 0x86, 0x30, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
   
    LogSeparator();
    LogSeparator();
}

void ApplyCameraHeightMod() {
    Log("ApplyCameraHeightMod");
    
    // vanilla:
    // the height of the camera aims at the center of the player; for aiming it makes 
    // more sense that the camera aims at the head of the character; the offset is
    // stored in rax+0c and is equal to 1.45f;
    // 48 8b 01         --  mov rax,[rcx]
    // 48 85 c0         --  test rax,rax
    // 74 06            --  je <+06>
    // f3 0f10 40 0c    --  movss xmm0,[rax+0C]
    //
    // modded:
    // increase it to a constant of 2f (0x40000000);
    // b8 00000040      --  mov eax,40000000
    // 66 0f6e c0       --  movd xmm0,eax
    // 90 90 90 90      --  4x nop
    // 0 100 0000 0
    std::vector<uint16_t> vanilla = { 0x48, 0x8b, 0x01, 0x48, 0x85, 0xc0, 0x74, 0x06, 0xf3, 0x0f, 0x10, 0x40, 0x0c };
    std::vector<uint8_t> modded = { 0xb8, 0x00, 0x00, 0x00, 0x40, 0x66, 0x0f, 0x6e, 0xc0, 0x90, 0x90, 0x90, 0x90 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplyLockOnScoreMod() {
    Log("ApplyLogOnScoreMod");
    
    // vanilla:
    // the score for selecting a lock-on target relies heavily on the distance to 
    // the player;
    // f3 0f 58 ce  --  addss xmm1,xmm6
    // f3 0f 58 cf  --  addss xmm1,xmm7
    //
    // modded:
    // make score = cos(angle_to_camera), i.e. one if you aim exactly at it and lower
    // otherwise;
    // f3 0f10 4b 6c    --  movss xmm1,[rbx+6c]
    // 90 90 90         --  3x nop
    std::vector<uint16_t> vanilla = { 0xf3, 0x0f, 0x58, 0xce, 0xf3, 0x0f, 0x58, 0xcf };
    std::vector<uint8_t> modded = { 0xf3, 0x0f, 0x10, 0x4b, 0x6c, 0x90, 0x90, 0x90 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);

    LogSeparator();

    // vanilla:
    // uses the normalized camera rotation to determine cos(angle_to_camera);
    // 0f 28 40 30      --  movaps xmm0,[rax+30] 
    // 0f 28 48 40      --  movaps xmm1,[rax+40]
    // 0f 29 45 d0      --  movaps [rbp-30],xmm0
    //
    // modded:
    // use a normalized variable that ignores the height (y = 0); however this variable is 
    // rotated; (x, z) = (0, 1) means west and not north; score = cos(angle) = dot_product
    // => the dot product needs to use (-z, x) instead of (x, z);
    // 0f 28 40 30      --  movaps xmm0,[rax+10] 
    // 0f 28 48 40      --  movaps xmm1,[rax+40]
    // 0f 29 45 d0      --  movaps [rbp-30],xmm0
    vanilla = { 0x0f, 0x28, 0x40, 0x30, 0x0f, 0x28, 0x48, 0x40, 0x0f, 0x29, 0x45, 0xd0 };
    modded = { 0x0f, 0x28, 0x40, 0x10, 0x0f, 0x28, 0x48, 0x40, 0x0f, 0x29, 0x45, 0xd0 };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();

    // vanilla:
    // this is part of the dot product calculation; score = dot(v_1, v_2) where 
    // v_1 = candidate_position - camera_position and v_2 = camera_rotation; v_2 is
    // modded above; v_1.y (height difference) is modded below; the dot product is
    // modded here; 
    // 0f 28 f2         --  movaps xmm6,xmm2
    // f3 0f 59 75 d0   --  mulss xmm6,[rbp-30]
    // 0f 28 ca         --  movaps xmm1,xmm2
    // 0f c6 ca 55      --  shufps xmm1,xmm2,55
    // f3 0f 59 4d d4   --  mulss xmm1,[rbp-2C]
    // f3 0f 58 f1      --  addss xmm6,xmm1
    // 0f c6 d2 aa      --  shufps xmm2,xmm2,-56
    // f3 0f 59 55 d8   --  mulss xmm2,[rbp-28]
    // f3 0f 58 f2      --  addss xmm6,xmm2
    //
    // modded:
    // rotate v_2_modded = (x, 0, z) to v_2_modded_rotated = (-z, 0, x); use
    // v_2_modded_rotated for the score;
    // 0f 28 ca         --  movaps xmm1,xmm2
    // f3 0f 59 4d d8   --  mulss xmm1,[rbp-28]      --  x_new = -z since subss later;
    // 0f 28 f2         --  movaps xmm6,xmm2
    // 0f c6 f2 55      --  shufps xmm6,xmm2,55
    // f3 0f 59 75 d4   --  mulss xmm6,[rbp-2c]      --  y_new is zero;
    // f3 0f 5c f1      --  subss xmm6,xmm1
    // 0f c6 d2 aa      --  shufps xmm2,xmm2,-56
    // f3 0f 59 55 d0   --  mulss xmm2,[rbp-30]      --  z_new = x;
    // f3 0f 58 f2      --  addss xmm6,xmm2
    vanilla = { 0x0f, 0x28, 0xf2, 0xf3, 0x0f, 0x59, 0x75, 0xd0, 0x0f, 0x28, 0xca, 0x0f, 0xc6, 0xca, 0x55, 0xf3, 0x0f, 0x59, 0x4d, 0xd4, 0xf3, 0x0f, 0x58, 0xf1, 0x0f, 0xc6, 0xd2, 0xaa, 0xf3, 0x0f, 0x59, 0x55, 0xd8, 0xf3, 0x0f, 0x58, 0xf2 };
    modded = { 0x0f, 0x28, 0xca, 0xf3, 0x0f, 0x59, 0x4d, 0xd8, 0x0f, 0x28, 0xf2, 0x0f, 0xc6, 0xf2, 0x55, 0xf3, 0x0f, 0x59, 0x75, 0xd4, 0xf3, 0x0f, 0x5c, 0xf1, 0x0f, 0xc6, 0xd2, 0xaa, 0xf3, 0x0f, 0x59, 0x55, 0xd0, 0xf3, 0x0f, 0x58, 0xf2 };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();

    // vanilla:
    // uses the height difference from the candidate to the camera in the score; setting
    // it to zero causes issues with locking onto targets behind cover;
    // f3 44 0f 5c 45 54        --  subss xmm8,[rbp+54]
    // f3 44 0f 11 45 44        --  movss [rbp+44],xmm8
    //
    // modded:
    // use the height difference to the player instead; otherwise the lock-on target 
    // might change simply by moving the camera up and down;
    // f3 44 0f 10 45 74        --  movss xmm8,[rbp+74]
    // f3 44 0f 11 45 44        --  movss [rbp+44],xmm8
    vanilla = { 0xf3, 0x44, 0x0f, 0x5c, 0x45, 0x54, 0xf3, 0x44, 0x0f, 0x11, 0x45, 0x44 };
    modded = { 0xf3, 0x44, 0x0f, 0x10, 0x45, 0x74, 0xf3, 0x44, 0x0f, 0x11, 0x45, 0x44 };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplySwitchLockOnMod() {
    Log("ApplySwitchLockOnMod");
    
    // vanilla:
    // this function switches locked-on targets; it is called when you move the mouse;
    // 48 89 5c 24 20           --  mov [rsp+20],rbx    <-- return + nops
    // 55                       --  push rbp
    // 56                       --  push rsi
    // 41 57 48 8d 6c 24 90     --  lea rbp,[rsp-70]
    //    
    // modded:
    // focus on the same locked-on target => skip this function by returning 
    // immediately;
    std::vector<uint16_t> vanilla = { 0x48, 0x89, 0x5c, 0x24, 0x20, 0x55, 0x56, 0x41, 0x57, 0x48, 0x8d, 0x6c, 0x24, 0x90 };
    std::vector<uint8_t> modded = { 0xC3, 0x90, 0x90, 0x90, 0x90, 0x55, 0x56, 0x41, 0x57, 0x48, 0x8d, 0x6c, 0x24, 0x90 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);

    LogSeparator();

    // vanilla:
    // the score is only used when initiating the lock-on; after that separate switch
    // function(s) are used; every candidate becomes the lowest score after initiation;
    // a8 20                --  test al,20
    // 74 10                --  je <+10>
    // 80 be 30280000 00    --  cmp byte ptr [rsi+00002830],00
    //
    // modded:
    // skip this and leave everyone as a viable candidate; this means that this function
    // now switches lock-on targets as well; the other switch function(s) seem to me more
    // concerned with you moving the mouse rather than aiming directly;
    // a8 20                --  test al,20
    // EB 10                --  jmp <+10>
    // 80 be 30280000 00    --  cmp byte ptr [rsi+00002830],00
    vanilla = { 0xa8, 0x20, 0x74, 0x10, 0x80, 0xbe };
    modded = { 0xa8, 0x20, 0xeb, 0x10, 0x80, 0xbe };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();
    LogSeparator();
}

void ApplyKeepLockOnMod() {
    Log("ApplyKeepLockOnMod");
    
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
    std::vector<uint16_t> vanilla = { 0x40, 0x0f, 0xb6, 0xff, 0x41, 0x0f, 0x2f, 0xc3 };
    std::vector<uint8_t> modded = { 0x41, 0x8b, 0xff, 0x90, 0x41, 0x0f, 0x2f, 0xc3 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();

    vanilla = { 0x40, 0x0f, 0xb6, 0xff, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x43, 0xff };
    modded = { 0x41, 0x8b, 0xff, 0x90, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x43, 0xff };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);
    
    LogSeparator();

    vanilla = { 0x40, 0x0f, 0xb6, 0xff, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x47, 0xff };
    modded = { 0x41, 0x8b, 0xff, 0x90, 0x0f, 0x2f, 0xc8, 0x41, 0x0f, 0x47, 0xff };
    assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);

    LogSeparator();
    LogSeparator();
}

void ApplyLockOnHealthBarMod() {
    Log("ApplyLockOnHealthBarMod");
    
    // vanilla:
    // shows health bars over the currently locked-on target;
    // 75 18            --  jne <+18>
    // 49 8b 5e 08      --  mov rbx,[r14+08]
    //
    // modded:
    // don't show it by skipping the if-block;
    // eb 18            --  jmp <+18>
    // 49 8b 5e 08      --  mov rbx,[r14+08]
    std::vector<uint16_t> vanilla = { 0x75, 0x18, 0x49, 0x8b, 0x5e, 0x08 };
    std::vector<uint8_t> modded = { 0xeb, 0x18, 0x49, 0x8b, 0x5e, 0x08 };
    uintptr_t assembly_location = SigScan(vanilla);
    if (assembly_location != 0) Replace(assembly_location, vanilla, modded);

    LogSeparator();
    LogSeparator();
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    Log("author " + author);
    Log("version " + version);
    LogSeparator();
    LogSeparator();

    // ApplyLockOnCloseRangeMod();
    // ApplyLockOnSensitivityMod();
    // ApplyReduceLockOnAngleMod();
    // ApplyCameraHeightMod();
    
    // makes LockOnCloseRangeMod useless;
    ApplyLockOnScoreMod();
    
    // makes LockOnSensitivityMod useless;
    ApplySwitchLockOnMod();
    
    ApplyFreeLockOnCameraMod();
    ApplyLockOnToggleMod();
    ApplyKeepLockOnMod();
    ApplyLockOnHealthBarMod();
    
    CloseLog();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    // someone wrote online that some processes might crash when returning false;
    if (reason != DLL_PROCESS_ATTACH) return true;
    DisableThreadLibraryCalls(module);
    CreateThread(0, 0, &MainThread, 0, 0, NULL);
    return true;
}
