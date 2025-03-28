#pragma once
#include "UnrealEd/EditorPanel.h"

class FPSEditorPanel : public UEditorPanel
{
public:
    virtual ~FPSEditorPanel() = default;
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    static double elapsedTime;

    static void SetPickElapsedTime(double pickElapsedTime);

private:
    float Width = 300;
    float Height = 125;

    static double pickElapsedTime;
    static uint32 numAttempts;
    static double accumulatedTime;

};

