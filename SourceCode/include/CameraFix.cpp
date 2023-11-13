// MIT License

// Copyright (c) 2022 Marius Storvik

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once 

#include "ModUtils.h"
#include <string>
#include <Windows.h>

using namespace ModUtils;

namespace CameraFix {
    inline static void Apply_CameraResetMod() {
        // this mod fixes a bug when the lock-on key is toggled; otherwise the camera
        // stutters in place and can move rapidly after some time to the nearest lock-on
        // target;

        // modded;
        Log("CameraFix::Apply_CameraResetMod");
        Log_Separator();
        
        std::string aob = "80 ? ? ? ? ? 00 74 ? ? 8b ? e8 ? ? ? ? eb ? 0f 28 ? ? ? ? ? ? 8d";
        std::string expectedBytes = "74";
        std::string newBytes = "eb";
        
        uintptr_t patchAddress = AobScan(aob);
        size_t offset = 7;
        
        if (patchAddress != 0) {
            patchAddress += offset;
            ReplaceExpectedBytesAtAddress(patchAddress, expectedBytes, newBytes);
        }
        
        // modded;
        Log_Separator();
        Log_Separator();
    }
}