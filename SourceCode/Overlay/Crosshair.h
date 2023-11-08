#pragma once

#include "../packages/DirectXHook/IRenderCallback.h"
#include "../packages/DirectXHook/OverlayFramework.h"

class Crosshair : public IRenderCallback {
    public:
	    void Setup();
	    void Render();

    private:
	    OF::Box* box_1;
	    OF::Box* box_2;
	    OF::Box* box_3;
        int texture_id = 0;
};