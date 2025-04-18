﻿#pragma once
#include "ImGUI/imgui.h"
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class ViewportControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateFlagButton() const;
    void CreateSRTButton(ImVec2 ButtonSize) const;

    uint64 ConvertSelectionToFlags(const bool selected[]) const;
    
private:
    float PanelPosX;
    float PanelPosY;
    
    float Width = 300, Height = 100;
    bool bOpenMenu = false;

    float FOV;
    float CameraSpeed = 0.0f;
    float GridScale = 1.0f;
};

