#include "WorldControlEditorPanel.h"
#include "Level.h"
#include "World.h"
#include "UnrealEd/Editor/EditorEngine.h"

#include "World.h"

void WorldControlEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();
    ImFont* IconFont = io.Fonts->Fonts[AWESOME_FONT];
    ImVec2 IconSize = ImVec2(32, 32);
    
    float PanelWidth = (Width) * 0.8f;
    float PanelHeight = 45.0f;

    float PanelPosX = 1.0f;
    float PanelPosY = 1.0f;

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
    ImGui::Begin("World Control Panel", nullptr, PanelFlags);

    CreateAddActorButton(IconSize, IconFont);

    ImGui::SameLine();
    
    CreateLevelEditorPlayButton(IconSize, IconFont);
    
    ImGui::SameLine();
    
    //CreateFlagButton();
    
    // ImGui::SameLine();

    //CreateModifyButton(IconSize, IconFont);

    // ImGui::SameLine();

    /* Get Window Content Region */
    // float ContentWidth = ImGui::GetWindowContentRegionMax().x;

    /* Move Cursor X Position */
    // ImGui::SetCursorPosX(ContentWidth - (IconSize.x * 3.0f + 16.0f));
    
    // ImGui::PushFont(IconFont);
    //CreateSRTButton(IconSize);
    // ImGui::PopFont();
    
    ImGui::End();
}

void WorldControlEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}

void WorldControlEditorPanel::CreateAddActorButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImGui::PushFont(IconFont);
    if (ImGui::Button(ICON_FA_BOX, ButtonSize))
    {
        ImGui::OpenPopup("AddActor");
    }
    ImVec2 pos = ImGui::GetItemRectMin() + ButtonSize - ImVec2(19, 19);
    
    ImGui::GetWindowDrawList()->AddText(pos, IM_COL32(0, 255, 0, 255), ICON_FA_PLUS);
    ImGui::PopFont();
    
    if (ImGui::BeginPopup("AddActor"))
    {
        static const char* ItemLabels[] = {
            "Empty Actor",
        };

        for (const char* label : ItemLabels)
        {
            if (ImGui::Selectable(label))
            {
                UWorld* World = GEngineLoop.GetWorld();
                AActor* SpawnedActor = nullptr;
                if (label == FString("Empty Actor"))
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("NameNAMENameNAMENameNAME"));
                }

                if (SpawnedActor && SpawnedActor->GetRootComponent())
                {
                    World->SetPickedActor(SpawnedActor);
                    World->SetPickedComponent(nullptr);
                }
            }
        }
        ImGui::EndPopup();
    }
}

void WorldControlEditorPanel::CreateLevelEditorPlayButton(ImVec2 ButtonSize, ImFont* IconFont)
{
    ImVec4 Green = ImVec4(0.f, 0.7f, 0.f, 1.0f);
    ImVec4 Red = ImVec4(0.7f, 0.f, 0.f, 1.0f);
    ImVec4 ActiveGray = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    ImVec4 InActiveGray = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ULevel* TargetLevel = GEngineLoop.GetWorld()->GetLevel();
    
    ImGui::PushFont(IconFont);
    if (TargetLevel->LevelState == ELevelState::Stop)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Green);
        // Play Level
        if (ImGui::Button(ICON_FA_PLAY, ButtonSize))
        {
            // GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
            GEngineLoop.StartPIE();
        }
    }
    else if (TargetLevel->LevelState == ELevelState::Play)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ActiveGray);
        // Pause
        if (ImGui::Button(ICON_FA_PAUSE, ButtonSize))
        {
            // GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
            TargetLevel->LevelState = ELevelState::Pause;
        }   
    }
    else if (TargetLevel->LevelState == ELevelState::Pause)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ActiveGray);
        // Play Continue
        if (ImGui::Button(ICON_FA_PLAY, ButtonSize))
        {
            // GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
            TargetLevel->LevelState = ELevelState::Play;
        }   
    }
    ImGui::PopStyleColor();
    ImGui::PopFont();
 
    ImGui::SameLine();
    
    ImGui::PushFont(IconFont);
    if (TargetLevel->LevelState == ELevelState::Play || TargetLevel->LevelState == ELevelState::Pause)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Red);
    
        if (ImGui::Button(ICON_FA_STOP, ButtonSize)) // Slider
        {
            // GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
            GEngineLoop.EndPIE();
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, InActiveGray);

        if (ImGui::Button(ICON_FA_STOP, ButtonSize)) // Slider
        {
            // GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetGridSize(GridScale);
            GEngineLoop.EndPIE();
        }
    }
    ImGui::PopStyleColor();
    ImGui::PopFont();
}