#pragma once
#include "UnrealEd/EditorPanel.h"

class FPSEditorPanel : public UEditorPanel
{
public:
    virtual ~FPSEditorPanel() = default;
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;
private:
    float Width = 100;
    float Height = 50;

};

