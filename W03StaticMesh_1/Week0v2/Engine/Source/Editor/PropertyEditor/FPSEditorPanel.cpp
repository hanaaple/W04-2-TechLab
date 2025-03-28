#include "HAL/WindowsPlatformTIme.h"
#include "FPSEditorPanel.h"
#include "ImGUI/imgui_internal.h"

double FPSEditorPanel::elapsedTime = 0;
double FPSEditorPanel::pickElapsedTime = 0;
uint32 FPSEditorPanel::numAttempts = 0;
double FPSEditorPanel::accumulatedTime = 0;

void FPSEditorPanel::Render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    float PanelWidth = Width;
    float PanelHeight = Height;

    float PanelPosX = 10.0f;
    float PanelPosY = 60.0f;

    ImVec2 MinSize(100, 30);
    ImVec2 MaxSize(400, FLT_MAX);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 255));

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    /* Render Start */
    ImGui::Begin("FPS Panel", nullptr, PanelFlags);

    double DeltaTimeMs = elapsedTime;
    double DeltaTimeSec = DeltaTimeMs / 1000.0;
    double FPS = 1.0 / DeltaTimeSec;
    ImGui::Text("DeltaTimeMs: %.4f", DeltaTimeMs);
    ImGui::Text("DeltaTimeSec: %.8f", DeltaTimeSec);
    ImGui::Text("FPS: %.1f (%.4f ms)", FPS,DeltaTimeMs);

    ImGui::Text("Picking Time %.4f ms", pickElapsedTime);

    ImGui::Text("Num Attempts %d", numAttempts);

    ImGui::Text("Accumulated Time %.4f ms", accumulatedTime);

    ImGui::End();

    ImGui::PopStyleColor();
}

void FPSEditorPanel::OnResize(HWND hWnd)
{

}

void FPSEditorPanel::SetPickElapsedTime(double pickElapsedTime)
{
    FPSEditorPanel::pickElapsedTime = pickElapsedTime;
    numAttempts++;
    accumulatedTime += pickElapsedTime;
}
