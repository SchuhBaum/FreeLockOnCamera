#include "Custom.h"
#include "Include/CameraFix.cpp"
#include "Include/Ini.h"
#include "Include/ModUtils.h"
#include <chrono>
#include <sstream>
#include <thread>

using namespace Custom;
using namespace mINI;
using namespace ModUtils;

const std::string author = "SchuhBaum";
const std::string version = "0.2.1";

// NOTE: Patches might also introduce cases where the searched array of bytes
//       is not unique anymore. Check if matches are unique.

//
// config
//

bool is_free_lock_on_camera_enabled = true;
bool is_health_bar_hidden           = true;
bool is_lock_on_camera_zoom_enabled = true;
bool is_only_using_camera_yaw       = true;
bool is_toggle                      = true;

float angle_range                      = 0.7F;
float angle_to_camera_score_multiplier = 6000.0F;
float camera_height                    = 1.45F;
float additional_range                 = 0.0F;

std::string target_switching_mode = "modded_switch";

//
//
//

std::string Get_HexString(float f) {
    // f = 0.0F produces an empty string; not sure why;
    std::string hex_string = Add_Spaces_To_HexString(Swap_HexString_Endian(Convert_Float_To_LowercaseHexString(f)));
    if (hex_string.empty()) return "00 00 00 00";
    return hex_string;
}

void Log_Parameters() {
    Log("additional_range");
    Log("  float: ", additional_range, "  hex: ", Get_HexString(additional_range));

    Log("angle_range");
    Log("  float: ", angle_range, "  hex: ", Get_HexString(angle_range));

    Log("angle_to_camera_score_multiplier");
    Log("  float: ", angle_to_camera_score_multiplier, "  hex: ", Get_HexString(angle_to_camera_score_multiplier));

    Log("camera_height");
    Log("  float: ", camera_height, "  hex: ", Get_HexString(camera_height));

    Log("is_free_lock_on_camera_enabled: ", is_free_lock_on_camera_enabled ? "true" : "false");
    Log("is_health_bar_hidden: ", is_health_bar_hidden ? "true" : "false");
    Log("is_lock_on_camera_zoom_enabled: ", is_lock_on_camera_zoom_enabled ? "true" : "false");
    Log("is_only_using_camera_yaw: ", is_only_using_camera_yaw ? "true" : "false");
    Log("is_toggle: ", is_toggle ? "true" : "false");

    Log("target_switching_mode: ", target_switching_mode);

    Log_Separator();
    Log_Separator();
}

void ReadAndLog_Config() {
    Log("ReadAndLog_Config");
    Log_Separator();
    INIFile config(GetModFolderPath() + "\\config.ini");
    INIStructure ini;

    try {
        if (!config.read(ini)) {
            Log("The config file was not found. Create a new one.");

            ini["FreeLockOnCamera"]["additional_range"]                 = std::to_string(additional_range);
            ini["FreeLockOnCamera"]["angle_range"]                      = std::to_string(angle_range);
            ini["FreeLockOnCamera"]["angle_to_camera_score_multiplier"] = std::to_string(static_cast<int>(angle_to_camera_score_multiplier));
            ini["FreeLockOnCamera"]["camera_height"]                    = std::to_string(camera_height);

            ini["FreeLockOnCamera"]["is_free_lock_on_camera_enabled"] = is_free_lock_on_camera_enabled ? "true" : "false";
            ini["FreeLockOnCamera"]["is_health_bar_hidden"]           = is_health_bar_hidden ? "true" : "false";
            ini["FreeLockOnCamera"]["is_lock_on_camera_zoom_enabled"] = is_lock_on_camera_zoom_enabled ? "true" : "false";
            ini["FreeLockOnCamera"]["is_only_using_camera_yaw"]       = is_only_using_camera_yaw ? "true" : "false";

            ini["FreeLockOnCamera"]["is_toggle"]             = is_toggle ? "true" : "false";
            ini["FreeLockOnCamera"]["target_switching_mode"] = target_switching_mode;

            config.write(ini, true);

            Log_Parameters();
            return;
        }

        additional_range                 = stof(ini["FreeLockOnCamera"]["additional_range"]);
        angle_range                      = stof(ini["FreeLockOnCamera"]["angle_range"]);
        angle_to_camera_score_multiplier = stoi(ini["FreeLockOnCamera"]["angle_to_camera_score_multiplier"]);
        camera_height                    = stof(ini["FreeLockOnCamera"]["camera_height"]);

        std::string str;
        str = ini["FreeLockOnCamera"]["is_free_lock_on_camera_enabled"];
        std::istringstream(str) >> std::boolalpha >> is_free_lock_on_camera_enabled;
        str = ini["FreeLockOnCamera"]["is_health_bar_hidden"];
        std::istringstream(str) >> std::boolalpha >> is_health_bar_hidden;
        str = ini["FreeLockOnCamera"]["is_lock_on_camera_zoom_enabled"];
        std::istringstream(str) >> std::boolalpha >> is_lock_on_camera_zoom_enabled;
        str = ini["FreeLockOnCamera"]["is_only_using_camera_yaw"];
        std::istringstream(str) >> std::boolalpha >> is_only_using_camera_yaw;
        str = ini["FreeLockOnCamera"]["is_toggle"];
        std::istringstream(str) >> std::boolalpha >> is_toggle;

        str = ini["FreeLockOnCamera"]["target_switching_mode"];
        if (str == "vanilla_switch" || str == "modded_keep" || str == "modded_switch") {
            target_switching_mode = str;
        }

        Log_Parameters();
        return;

    } catch(const std::exception& exception) {
        Log("Could not read or create the config file. Use defaults.");
        Log_Parameters();
        return;
    }
}

void Apply_AngleRangeMod() {
    Log("Apply_AngleRangeMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    // vanilla:
    // you switch or acquire targets that are within ~0.7f radians(?) (around 40 degrees)
    // to where you aim at; maybe it's half of that (+- 20 degrees);
    //
    // modded:
    // change this value to angle_range;
    vanilla = "c7 83 2c 29 00 00 c2 b8 32 3f";
    modded = "c7 83 2c 29 00 00 " + Get_HexString(angle_range);
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_AngleToCameraMod() {
    Log("Apply_AngleToCameraMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    // vanilla:
    // uses the normalized camera rotation to determine cos(angle_to_camera);
    // 0f 28 40 30      --  movaps xmm0,[rax+30]
    // 0f 28 48 40      --  movaps xmm1,[rax+40]
    // 0f 29 45 d0      --  movaps [rbp-30],xmm0
    //
    // modded:
    // use a normalized variable that ignores the height (y = 0); however this variable
    // is rotated; (x, z) = (0, 1) means west instead of north; cos(angle_to_camera) =
    // dot_product => the dot product needs to use (-z, x) instead of (x, z) later;
    // 0f 28 40 10      --  movaps xmm0,[rax+10]
    // 0f 28 48 40      --  movaps xmm1,[rax+40]
    // 0f 29 45 d0      --  movaps [rbp-30],xmm0

    // it can be a bit time consuming to do it like this; it is more general; I think
    // I will wait and see how much patches can mess these up;
    vanilla = "0f 28 ? 30 0f 28 ? 40 0f 29 ? d0";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location + 3, "30", "10");

    Log_Separator();

    // vanilla:
    // this is part of the dot product calculation; cos(angle_to_camera) = dot(v_1, v_2)
    // where v_1 = candidate_position - camera_position and v_2 = camera_rotation;
    // v_2 is modded above; v_1.y (height difference) is modded below; the dot product
    // is modded here;
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
    vanilla = "0f 28 f2 f3 0f 59 75 d0 0f 28 ca 0f c6 ca 55 f3 0f 59 4d d4 f3 0f 58 f1 0f c6 d2 aa f3 0f 59 55 d8 f3 0f 58 f2";
    modded = "0f 28 ca f3 0f 59 4d d8 0f 28 f2 0f c6 f2 55 f3 0f 59 75 d4 f3 0f 5c f1 0f c6 d2 aa f3 0f 59 55 d0 f3 0f 58 f2";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();

    // // vanilla:
    // // uses the height difference from the candidate to the camera in angle_to_camera;
    // // setting it to zero causes issues with locking onto targets behind cover;
    // // f3 44 0f 5c 45 54        --  subss xmm8,[rbp+54]
    // // f3 44 0f 11 45 44        --  movss [rbp+44],xmm8
    // //
    // // modded:
    // // use the height difference to the player instead; otherwise the lock-on target
    // // might change simply by moving the camera up and down;
    // // f3 44 0f 10 45 74        --  movss xmm8,[rbp+74]
    // // f3 44 0f 11 45 44        --  movss [rbp+44],xmm8
    // vanilla = "f3 44 0f 5c 45 54 f3 44 0f 11 45 44";
    // modded = "f3 44 0f 10 45 74 f3 44 0f 11 45 44";
    // assembly_location = AobScan(vanilla);
    // if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    // Log_Separator();

    // vanilla:
    // uses the height difference between the candidate and the camera;
    // 48 8d 55 40                              --  lea rdx,[rbp+40]
    // 48 8d 4d 80                              --  lea rcx,[rbp-80]
    // e8 02 58 a7 ff                           --  call NormalizeVector(...)
    // 0f 28 10                                 --  xmm2,[rax]
    //
    // modded:
    // instead of modding it I can change the value before it is normalized (function
    // call) and restore it afterwards; this way the height difference is completely
    // ignored but only for the score; way less janky;
    // f3 0f 10 75 44                           --  movss xmm6,[rbp+44]
    // f3 0f 11 55 44                           --  movss [rbp+44],xmm2     -- xmm2 is a function parameter as well and set to zero
    // 48 8d 55 40                              --  lea rdx,[rbp+40]
    // 48 8d 4d 80                              --  lea rcx,[rbp-80]
    // ff 15 02000000 eb 08 new_call_address    --  call NormalizeVector(...)
    // f3 0f 11 75 44                           --  movss [rbp+44],xmm6
    // 0f 28 10                                 --  xmm2,[rax]

    // Search for the exact offset "82 56 a7 ff". The game crashes if this does
    // not line up with new_call_address later.
    vanilla = "48 8d 55 40 48 8d 4d 80 e8 82 56 a7 ff 0f 28 10";
    assembly_location = AobScan(vanilla);

    if (assembly_location != 0) {
        // https://stackoverflow.com/questions/40936534/how-to-alloc-a-executable-memory-buffer
        MODULEINFO module_info;
        GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("eldenring.exe"), &module_info, sizeof(module_info));
        LPVOID eldenring_assembly_base = module_info.lpBaseOfDll;

        int memory_block_size_in_bytes = 64;
        SYSTEM_INFO system_info;
        GetSystemInfo(&system_info);
        auto const page_size = system_info.dwPageSize;

        // prepare the memory in which the machine code will be put (it's not executable yet):
        auto const buffer = VirtualAlloc(nullptr, page_size, MEM_COMMIT, PAGE_READWRITE);
        auto const new_assembly_location = reinterpret_cast<std::uintptr_t>(buffer);

        // removes 14 + 2 bytes from assembly_location => jump-back-address is assembly_location + 16;
        Hook(assembly_location, new_assembly_location, 2);
        vanilla = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

        // The absolute address offset depends on the relative address offset
        // in the searched array of bytes. This means that this part is difficult
        // to make robust against game updates.
        // Update: This time only the local offset in the vanilla string changed.
        // The offset from the eldenring.exe base address remained at 0x18c460.
        // Update: +1
        // Update: +1. If this stays like this then I try to use wildcards.
        std::string new_call_address = Add_Spaces_To_HexString(Swap_HexString_Endian(NumberToHexString((ULONGLONG)eldenring_assembly_base + 0x18c460)));
        modded = "f3 0f 10 75 44 f3 0f 11 55 44 48 8d 55 40 48 8d 4d 80 ff 15 02 00 00 00 eb 08 " + new_call_address + " f3 0f 11 75 44 0f 28 10";
        ReplaceExpectedBytesAtAddress((uintptr_t)buffer, vanilla, modded);
        Hook(new_assembly_location + (modded.size() + 1)/3, assembly_location + 16);

        // mark the memory as executable:
        DWORD dummy;
        VirtualProtect(buffer, memory_block_size_in_bytes, PAGE_EXECUTE_READ, &dummy);
    }

    Log_Separator();
    Log_Separator();
}

void Apply_CameraHeightMod() {
    Log("Apply_CameraHeightMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

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
    // set it to camera_height; it is not ideal to do it here since it is read here
    // but not set; I have not found the assembly location where it is set;
    // b8 xx xx xx xx   --  mov eax,camera_height
    // 66 0f6e c0       --  movd xmm0,eax
    // 90 90 90 90      --  4x nop
    vanilla = "48 8b 01 48 85 c0 74 06 f3 0f 10 40 0c";
    modded = "b8 " + Get_HexString(camera_height) + " 66 0f 6e c0 90 90 90 90";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_FreeLockOnCameraMod() {
    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    if (is_free_lock_on_camera_enabled) {
        Log("Apply_FreeLockOnCameraMod");
        Log_Separator();

        // vanilla:
        // sets the variable that disables the free camera during lock-on to one;
        // c6 81 10030000 01    --  mov byte ptr [rcx+00000310],01
        //
        // modded:
        // sets the same variable to zero instead;
        vanilla = "c6 81 10 03 00 00 01";
        modded = "c6 81 10 03 00 00 00";
        assembly_location = AobScan(vanilla);
        if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

        if (is_lock_on_camera_zoom_enabled) {
            Log_Separator();

            // vanilla:
            // checks if the free lock-on camera is disabled; if yes then some camera paramters
            // get changed;
            // 38 93 10030000       --  cmp [rbx+00000310],dl   <-- dl is always zero(?)
            // 74 26                --  je <+26>                <-- nop this
            //
            // modded:
            // changing the lock-on variable at the beginning has some side effects; the camera
            // zooms out a bit when locking on certain large enemies; this change here tries to
            // enable it again;
            vanilla = "38 93 10 03 00 00 74 26";
            modded = "38 93 10 03 00 00 90 90";
            assembly_location = AobScan(vanilla);
            if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);
        }

        Log_Separator();
        Log_Separator();
        return;
    }

    if (!is_lock_on_camera_zoom_enabled) {
        Log("Apply_FreeLockOnCameraMod");
        Log_Separator();

        // vanilla:
        // same as before;
        // 38 93 10030000       --  cmp [rbx+00000310],dl   <-- dl is always zero(?)
        // 74 26                --  je <+26>                <-- change to jmp
        //
        // modded:
        // this time jump always to skip the zoom out; there still might be side effects;
        // not sure what else is affected by skipping this;
        vanilla = "38 93 10 03 00 00 74 26";
        modded = "38 93 10 03 00 00 eb 26";
        assembly_location = AobScan(vanilla);
        if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

        Log_Separator();
        Log_Separator();
    }
}

void Apply_KeepLockOnMod() {
    Log("Apply_KeepLockOnMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

//    // vanilla:
//    // removes the lock-on when you don't look at the target with the camera;
//    // 40 0fb6 ff           --  movzx edi,dil       --  dil holds the value zero
//    // 41 0f2f c3           --  comiss xmm0,xmm11
//    //
//    // modded:
//    // override the variable that tracks if the lock-on should be removed;
//    // there are three locations where this needs to be done;
//    // 41 8b ff             --  mov edi,r15d        --  r15d holds the value one
//    // 90                   --  nop
//    // 41 0f2f c3           --  comiss xmm0,xmm11

//    // Relative location: eldenring.exe+716ed9 in v1.16.
//    // Absolute location: 00007ff653cd6ed9.
//    vanilla = "40 0f b6 ff 41 0f 2f c3";
//    modded = "41 8b ff 90 41 0f 2f c3";
//    assembly_location = AobScan(vanilla);
//    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

//    Log_Separator();

//    // Relative location: eldenring.exe+716f19 in v1.16.
//    // Absolute location: 00007ff653cd6f19.
//    vanilla = "40 0f b6 ff 0f 2f c8 41 0f 43 ff";
//    modded = "41 8b ff 90 0f 2f c8 41 0f 43 ff";
//    assembly_location = AobScan(vanilla);
//    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

//    Log_Separator();

//    // There is a weird bug. If anything is scanned before this then it will
//    // not find this location. Even when vanilla was "replaced" by vanilla. For
//    // all I know, this worked before v1.16. For now use the alternative
//    // implementation which one needs to replace one array of bytes.
//    // Relative location: eldenring.exe+716ffa in v1.16.
//    // Absolute location: 00007ff653cd6ffa.
//    vanilla = "40 0f b6 ff 0f 2f c8 41 0f 47 ff";
//    modded = "41 8b ff 90 0f 2f c8 41 0f 47 ff";
//    assembly_location = AobScan(vanilla);
//    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    // This seems to have the same effect as the difference already in place
    // (see above), i.e. any new target needs to be within a certain range to
    // where you aim at; you can change how much using the function
    // Apply_AngleRangeMod().
    //
    // vanilla:
    // Excludes candidates that you don't look at. The same goes for the
    // current lock-on target.
    // target; in that case it waits for a short duration before it removes the lock-on;
    // f3 0f 10 44 24 34            --  movss xmm0,[rsp+34]         <-- time difference
    // f3 0f 58 86 78 29 00 00      --  addss xmm0,[rsi+00002978]   <-- current timer
    //
    // modded:
    // Keep the first part but don't remove the lock-on, i.e. keep the timer at
    // zero.
    // f3 0f 10 44 24 34            --  movss xmm0,[rsp+34]     <-- ignored
    // 0f 57 c0                     --  xorps xmm0,xmm0         <-- set xmm0 to zero
    // 90 90 90 90 90               --  5x nop
    vanilla = "f3 0f 10 44 24 34 f3 0f 58 86 78 29 00 00";
    modded = "f3 0f 10 44 24 34 0f 57 c0 90 90 90 90 90";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_LockOnRangeMod() {
    Log("Apply_LockOnRangeMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    // vanilla:
    // The absolute lock-on range is saved in rsi+2930 (and possibly other
    // locations). That value is checked and reset via code. Changing it
    // directly will not work. Plus, it has side effects. Changing only that
    // value prevents you from changing targets at long range.
    //
    // c7 83 30 29 00 00 00 00 70 41 -- mov [RSI + 0x2930], 15F
    //
    // modded:
    // Override the additional-range value instead (stored at rsi+2924).
    // In vanilla, this one is zero initialized and never touched. This has not
    // the side effect compared to changing the range directly.
    //
    // I assume that the range is zero initialized as well. The reset function
    // does a smooth transition. If it is not zero initialized it might take
    // some time before it reaches 15f again.
    //
    // c7 83 24 29 00 00 ** ** ** ** -- mov [RSI + 0x2924], additional_range

    vanilla = "c7 83 30 29 00 00 00 00 70 41";
    modded = "c7 83 24 29 00 00 " + Get_HexString(additional_range);
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_LockOnHealthBarMod() {
    Log("Apply_LockOnHealthBarMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    // vanilla:
    // shows health bars over the currently locked-on target;
    // 75 18            --  jne <+18>
    // 49 8b 5e 08      --  mov rbx,[r14+08]
    //
    // modded:
    // don't show it by skipping the if-block;
    // eb 18            --  jmp <+18>
    // 49 8b 5e 08      --  mov rbx,[r14+08]
    vanilla = "75 18 49 8b 5e 08";
    modded = "eb 18 49 8b 5e 08";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_LockOnScoreMod() {
    Log("Apply_LockOnScoreMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    // vanilla:
    // uses angle_to_player for the score; this variable is saved at [rbx+64];
    // f3 0f 10 43 64       --  movss xmm0,[rbx+64]
    // f3 0f 10 4b 60       --  movss xmm1,[rbx+60]
    //
    // modded:
    // use angle_to_camera instead of angle_to_player; this means that you lock onto
    // and switch to candidates that you look at with the camera;
    // f3 0f 10 43 6c       --  movss xmm0,[rbx+6c]
    // f3 0f 10 4b 60       --  movss xmm1,[rbx+60]
    vanilla = "f3 0f 10 43 64 f3 0f 10 4b 60";
    modded = "f3 0f 10 43 6c f3 0f 10 4b 60";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    if (angle_to_camera_score_multiplier != 30.0F) {
        Log_Separator();

        // vanilla:
        // the score applies a multiplier on how well the player is facing the candidate;
        // 48 c7 83 4c 29 00 00 00 00 f0 41     --  mov [rbx+294c],(float)30
        //
        // increase the score multiplier for angle_to_player; this means that the range
        // has less effect and the angle has more on the final score;
        // 48 c7 83 4c 29 00 00 xx xx xx xx     --  mov [rbx+294c],angle_to_camera_score_multiplier
        vanilla = "48 c7 83 4c 29 00 00 00 00 f0 41";
        modded = "48 c7 83 4c 29 00 00 " + Get_HexString(angle_to_camera_score_multiplier);
        assembly_location = AobScan(vanilla);
        if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);
    }

    Log_Separator();
    Log_Separator();
}

void Apply_LockOnSensitivityMod() {
    Log("Apply_LockOnSensitivityMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

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
    vanilla = "72 3a 0f 2f 15 9a be 2c 02 76 31";
    modded = "72 3a 0f 2f 15 9a be 2c 02 90 90";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_LockOnToggleMod() {
    Log("Apply_LockOnToggleMod");
    Log_Separator();

    std::string search_string;
    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

    // vanilla:
    // you have to press the lock-on key every time you lose it; this is not great when
    // you can move the camera freely;
    // 88 86 31 28 00 00        --  mov [rsi+00002831],al
    // 8b 0d 63 fd 13 04        --  mov ecx,<address_offset>
    //
    // modded:
    // make it a toggle instead => prevent it from getting overriden; still not perfect
    // since you have to remember if it is toggled on or off;
    // 90 90 90 90 90 90        --  6x nop
    // 8b 0d 63 fd 13 04        --  mov ecx,<address_offset>
    vanilla = "88 86 31 28 00 00 8b 0d";
    modded = "90 90 90 90 90 90 8b 0d";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();

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
    vanilla = "80 b9 30 28 00 00 00 0f 94 c0";
    modded = "80 b9 31 28 00 00 00 0f 94 c0";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    Log_Separator();

    // vanilla:
    // you lose your toggle after performing a critical hit;
    // 48 8b cf                 --  mov rcx,rdi
    // e8 89 32 8f ff           --  call <offset>
    // 84 c0                    --  test al,al
    // 74 41                    --  je <+41>
    //
    // modded:
    // skip the if-block by jumping always;
    // 48 8b cf                 --  mov rcx,rdi
    // e8 89 32 8f ff           --  call <offset>
    // 84 c0                    --  test al,al
    // eb 41                    --  jmp <+41>
    search_string = "48 8b cf e8 ? ? ? ? 84 c0 74 41";
    vanilla = "84 c0 74 41";
    modded = "84 c0 eb 41";
    assembly_location = AobScan(search_string);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location+8, vanilla, modded);

    Log_Separator();
    Log_Separator();
}

void Apply_SwitchLockOnMod() {
    Log("Apply_SwitchLockOnMod");
    Log_Separator();

    std::string vanilla;
    std::string modded;
    uintptr_t assembly_location;

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
    vanilla = "48 89 5c 24 20 55 56 41 57 48 8d 6c 24 90";
    modded = "c3 90 90 90 90 55 56 41 57 48 8d 6c 24 90";
    assembly_location = AobScan(vanilla);
    if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);

    if (target_switching_mode == "modded_switch") {
        Log_Separator();

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
        vanilla = "a8 20 74 10 80 be";
        modded = "a8 20 eb 10 80 be";
        assembly_location = AobScan(vanilla);
        if (assembly_location != 0) ReplaceExpectedBytesAtAddress(assembly_location, vanilla, modded);
    }

    Log_Separator();
    Log_Separator();
}

//
// main
//

DWORD WINAPI MainThread(LPVOID lpParam) {
    // this is for ModEngine2-only users; apparently the first change is not applied
    // otherwise; it reaches the end of scannable memory;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    Log("author " + author);
    Log("version " + version);

    Log_Separator();
    Log_Separator();
    ReadAndLog_Config();

    if (angle_range != 0.7F)      Apply_AngleRangeMod();
    if (is_only_using_camera_yaw) Apply_AngleToCameraMod();
    if (camera_height != 1.45F)   Apply_CameraHeightMod();
    Apply_FreeLockOnCameraMod();

    Apply_KeepLockOnMod();
    if (is_health_bar_hidden)     Apply_LockOnHealthBarMod();
    if (additional_range != 0.0F) Apply_LockOnRangeMod();
    Apply_LockOnScoreMod();

    if (is_toggle)                                 Apply_LockOnToggleMod();
    if (target_switching_mode != "vanilla_switch") Apply_SwitchLockOnMod();

    // this can fail when using the original CameraFix mod; in that case it can take
    // a while and the logs might misalign or get spammed otherwise;
    if (is_toggle) CameraFix::Apply_CameraResetMod();

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
