#include "FPSEditorPanel.h"
#include "ImGUI/imgui_internal.h"

void FPSEditorPanel::Render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);

    float PanelWidth = Width;
    float PanelHeight = Height;

    float PanelPosX = 500.0f;
    float PanelPosY = 30.0f;

    ImVec2 MinSize(100, 50);
    ImVec2 MaxSize(FLT_MAX, 50);

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

    ImGui::Text("FPS: %.1f", io.Framerate);

    ImGui::End();

    ImGui::PopStyleColor();
}

void FPSEditorPanel::OnResize(HWND hWnd)
{

}
