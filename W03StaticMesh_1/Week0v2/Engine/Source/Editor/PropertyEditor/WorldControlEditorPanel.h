#pragma once
#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

#define ICON_FA_PLAY "\xE2\x96\xB6"   // ▶
#define ICON_FA_STOP "\xE2\x97\xBB"   // ■
#define ICON_PAUSE "\xE2\x8F\xB8"     // ⏸

// World로 해야될지 Level로 해야될지 헷갈림.
class WorldControlEditorPanel : public UEditorPanel
{
public:
    WorldControlEditorPanel() = default;
    
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreateLevelEditorPlayButton(ImVec2 ButtonSize, ImFont* IconFont);
    
private:
    float Width = 300;
    float Height = 100;
    ImFont* IconFont;
};
