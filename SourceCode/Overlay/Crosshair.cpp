#include "Crosshair.h"
#include "../ModUtils.h"

using namespace OF;

void Crosshair::Setup() {
    InitFramework(device, spriteBatch, window);
    // box = CreateBox(0, 0, 4, 4);
    box_1 = CreateBox(0, 0, 2, 4);
    box_2 = CreateBox(0, 0, 4, 2);
    box_3 = CreateBox(0, 0, 2, 2);
    
    box_1->draggable = false;
    box_2->draggable = false;
    box_3->draggable = false;
    // box->hasBeenRendered = true;
    // texture_id = LoadTexture("crosshair_texture.png");
    ModUtils::Log("Setup");
}

void Crosshair::Render() {
    // RECT overlay_rectangle;
    // GetClientRect(ofWindow, &overlay_rectangle);
    // ofWindowWidth = overlay_rectangle.right - overlay_rectangle.left;
    // ofWindowHeight = overlay_rectangle.bottom - overlay_rectangle.top;
    
    box_1->x = ofWindowWidth/2 - 1;
    box_1->y = ofWindowHeight/2 - 2;
    
    box_2->x = ofWindowWidth/2 - 2;
    box_2->y = ofWindowHeight/2 - 1;

    box_3->x = ofWindowWidth/2 - 1;
    box_3->y = ofWindowHeight/2 - 1;

    // CheckMouseEvents();
    // DrawBox(box, texture_id);
    DrawBox(box_1, 0, 0, 0, 255);
    DrawBox(box_2, 0, 0, 0, 255);
    DrawBox(box_3, 255, 255, 255, 255);
}