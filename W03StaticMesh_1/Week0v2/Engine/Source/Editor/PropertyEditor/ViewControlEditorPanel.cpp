#include "ViewControlEditorPanel.h"

#include "World.h"
#include "Actors/Player.h"
#include "Components/LightComponent.h"
#include "Components/SphereComp.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/UText.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"
#include "ImGUI/imgui_internal.h"
#include "LevelEditor/SLevelEditor.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include "UnrealEd/EditorViewportClient.h"
#include "PropertyEditor/ShowFlags.h"
#include "Renderer/Renderer.h"

void ViewportControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[FEATHER_FONT];
    ImVec2 IconSize = ImVec2(32, 32);
    
    float PanelWidth = (Width) * 0.8f;
    float PanelHeight = 45.0f;

    PanelPosX = 1.0f;
    PanelPosY = 50.0f;

    ImVec2 MinSize(300, 50);
    ImVec2 MaxSize(FLT_MAX, 50);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    
    /* Render Start */
    ImGui::Begin("Viewport Control Panel", nullptr, PanelFlags);
    
    CreateMenuButton(IconSize, IconFont);
    
    ImGui::SameLine();
    
    CreateFlagButton();
    
    ImGui::SameLine();

    CreateModifyButton(IconSize, IconFont);

    ImGui::SameLine();

    /* Get Window Content Region */
    float ContentWidth = ImGui::GetWindowContentRegionMax().x;

    /* Move Cursor X Position */
    ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));
    
    ImGui::PushFont(IconFont);
    
    CreateSRTButton(IconSize);
    ImGui::PopFont();
    
    ImGui::End();
}

void ViewportControlEditorPanel::CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9ad", ButtonSize)) // Menu
    {
        bOpenMenu = !bOpenMenu;
    }
    ImGui::PopFont();
    
    if (bOpenMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(PanelPosX + 10, PanelPosY + 55), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(135, 170), ImGuiCond_Always);
        
        ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        if (ImGui::MenuItem("New Scene"))
        {
            // TODO: New Scene
        }

        if (ImGui::MenuItem("Load Scene"))
        {
            char const * lFilterPatterns[1]={"*.scene"};
            const char* FileName =  tinyfd_openFileDialog("Open Scene File", "", 1, lFilterPatterns,"Scene(.scene) file", 0);

            if (FileName == nullptr)
            {
                tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                ImGui::End();
                return;
            }

            // TODO: Load Scene
        }

        ImGui::Separator();
        
        if (ImGui::MenuItem("Save Scene"))
        {
            char const * lFilterPatterns[1]={"*.scene"};
            const char* FileName =  tinyfd_saveFileDialog("Save Scene File", "", 1, lFilterPatterns,"Scene(.scene) file");

            if (FileName == nullptr)
            {
                ImGui::End();
                return;
            }

            // TODO: Save Scene

            tinyfd_messageBox("알림", "저장되었습니다.", "ok", "info", 1);
        }

        ImGui::Separator();
        
        if (ImGui::BeginMenu("Import"))
        {
            if (ImGui::MenuItem("Wavefront (.obj)"))
            {
                char const * lFilterPatterns[1]={"*.obj"};
                const char* FileName =  tinyfd_openFileDialog("Open OBJ File", "", 1, lFilterPatterns,"Wavefront(.obj) file", 0);

                if (FileName != nullptr)
                {
                    std::cout << FileName << std::endl;

                    if (FManagerOBJ::CreateStaticMesh(FileName) == nullptr)
                    {
                        tinyfd_messageBox("Error", "파일을 불러올 수 없습니다.", "ok", "error", 1);
                    }
                }
            }
            
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Quit"))
        {
            ImGui::OpenPopup("프로그램 종료");   
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("프로그램 종료", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("정말 프로그램을 종료하시겠습니까?");
            ImGui::Separator();

            float ContentWidth = ImGui::GetWindowContentRegionMax().x;

            /* Move Cursor X Position */
            ImGui::SetCursorPosX(ContentWidth - (160.f + 10.0f));
            
            if (ImGui::Button("OK", ImVec2(80, 0))) { PostQuitMessage(0); }

            ImGui::SameLine();
            
            ImGui::SetItemDefaultFocus();
            ImGui::PushID("CancelButtonWithQuitWindow");
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
            if (ImGui::Button("Cancel", ImVec2(80, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::PopStyleColor(3);
            ImGui::PopID();

            ImGui::EndPopup();
        }
        
        ImGui::End();
    }
}

void ViewportControlEditorPanel::CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button("\ue9c4", ButtonSize)) // Slider
    {
        ImGui::OpenPopup("SliderControl");
    }
    ImGui::PopFont();

    if (ImGui::BeginPopup("SliderControl"))
    {
        ImGui::Text("Grid Scale");
        GridScale = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetGridSize();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Grid Scale", &GridScale, 0.1f, 1.0f, 20.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
        }
        ImGui::Separator();

        ImGui::Text("Camera FOV");
        FOV = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewFOV();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##Fov", &FOV, 0.1f, 30.0f, 120.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetViewFOV(FOV);
        }
        ImGui::Spacing();

        ImGui::Text("Camera Speed");
        CameraSpeed = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##CamSpeed", &CameraSpeed, 0.1f, 0.198f, 192.0f, "%.1f"))
        {
            GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeedScalar(CameraSpeed);
        }
        
        ImGui::EndPopup();
    }
}

void ViewportControlEditorPanel::CreateFlagButton() const
{
    auto ActiveViewport = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    const char* ViewTypeNames[] = { "Perspective", "Top", "Bottom", "Left", "Right", "Front", "Back" };
    ELevelViewportType ActiveViewType = ActiveViewport->GetViewportType();
    FString TextViewType = ViewTypeNames[ActiveViewType];
    
    if (ImGui::Button(GetData(TextViewType), ImVec2(120, 32)))
    {
        // toggleViewState = !toggleViewState;
        ImGui::OpenPopup("ViewControl");
    }

    if (ImGui::BeginPopup("ViewControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewTypeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewportType() == i);
            if (ImGui::Selectable(ViewTypeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewportType((ELevelViewportType)i);
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    
    const char* ViewModeNames[] = { "Lit", "Unlit", "Wireframe" };
    FString SelectLightControl = ViewModeNames[(int)ActiveViewport->GetViewMode()];
    ImVec2 LightTextSize = ImGui::CalcTextSize(GetData(SelectLightControl));
    
    if (ImGui::Button(GetData(SelectLightControl), ImVec2(30 + LightTextSize.x, 32)))
    {
        ImGui::OpenPopup("LightControl");
    }

    if (ImGui::BeginPopup("LightControl"))
    {
        for (int i = 0; i < IM_ARRAYSIZE(ViewModeNames); i++)
        {
            bool bIsSelected = ((int)ActiveViewport->GetViewMode() == i);
            if (ImGui::Selectable(ViewModeNames[i], bIsSelected))
            {
                ActiveViewport->SetViewMode((EViewModeIndex)i);
                FEditorEngine::graphicDevice.ChangeRasterizer(ActiveViewport->GetViewMode());
                FEditorEngine::renderer.ChangeViewMode(ActiveViewport->GetViewMode());
            }

            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    
    if (ImGui::Button("Show", ImVec2(60, 32)))
    {
        ImGui::OpenPopup("ShowControl");
    }
    
    const char* items[] = { "AABB", "Primitive", "BillBoard", "UUID"};
    uint64 ActiveViewportFlags = ActiveViewport->GetShowFlag();

    if (ImGui::BeginPopup("ShowControl"))
    {
        bool selected[IM_ARRAYSIZE(items)] =
        {
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_AABB)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText)) != 0,
            (ActiveViewportFlags & static_cast<uint64>(EEngineShowFlags::SF_UUIDText)) != 0
        };  // 각 항목의 체크 상태 저장
        
        for (int i = 0; i < IM_ARRAYSIZE(items); i++)
        {
            ImGui::Checkbox(items[i], &selected[i]);
        }
        ActiveViewport->SetShowFlag(ConvertSelectionToFlags(selected));
        ImGui::EndPopup();
    }
}

// code is so dirty / Please refactor
void ViewportControlEditorPanel::CreateSRTButton(ImVec2 ButtonSize) const
{
    AEditorPlayer* Player = GEngineLoop.GetWorld()->GetEditorPlayer();
    if (Player == nullptr)
        return;
    ImVec4 ActiveColor = ImVec4(0.00f, 0.00f, 0.85f, 1.0f);
    
    ControlMode ControlMode = Player->GetControlMode();

    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9bc", ButtonSize)) // Move
    {
        Player->SetMode(CM_TRANSLATION);
    }
    if (ControlMode == CM_TRANSLATION)
    {
        ImGui::PopStyleColor();
    }
	
    ImGui::SameLine();

    if (ControlMode == CM_ROTATION)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9d3", ButtonSize)) // Rotate
    {
        Player->SetMode(CM_ROTATION);
    }
    if (ControlMode == CM_ROTATION)
    {
        ImGui::PopStyleColor();
    }
	
    ImGui::SameLine();

    if (ControlMode == CM_SCALE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ActiveColor);
    }
    if (ImGui::Button("\ue9ab", ButtonSize)) // Scale
    {
        Player->SetMode(CM_SCALE);
    }
    if (ControlMode == CM_SCALE)
    {
        ImGui::PopStyleColor();
    }
}

uint64 ViewportControlEditorPanel::ConvertSelectionToFlags(const bool selected[]) const
{
    uint64 flags = static_cast<uint64>(EEngineShowFlags::None);

    if (selected[0])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_AABB);
    if (selected[1])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_Primitives);
    if (selected[2])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_BillboardText);
    if (selected[3])
        flags |= static_cast<uint64>(EEngineShowFlags::SF_UUIDText);
    return flags;
}


void ViewportControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}
