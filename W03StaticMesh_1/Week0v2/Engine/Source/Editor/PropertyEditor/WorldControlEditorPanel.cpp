#include "WorldControlEditorPanel.h"
#include "Level.h"
#include "World.h"
#include "UnrealEd/Editor/EditorEngine.h"

#include "World.h"
#include "Components/LightComponent.h"
#include "Components/SphereComp.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/UText.h"
#include "Engine/FLoaderOBJ.h"
#include "Engine/StaticMeshActor.h"

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

    //ImGui::SetNextWindowPos()
    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.01f, 1.f));
    /* Render Start */
    ImGui::Begin("World Control Panel", nullptr, PanelFlags);
    ImGui::SameLine(200);
    CreateAddActorButton(IconSize, IconFont);

    ImGui::SameLine(400);
    
    CreateLevelEditorPlayButton(IconSize, IconFont);
    
    ImGui::SameLine();
    
    ImGui::End();
    ImGui::PopStyleColor();
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
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.03f, 0.03f, 0.03f, 1.f));
    ImGui::PushFont(IconFont);
    if (ImGui::Button(ICON_FA_BOX, ButtonSize))
    {
        ImGui::OpenPopup("AddActor");
    }
    ImVec2 pos = ImGui::GetItemRectMin() + ButtonSize - ImVec2(19, 19);
    ImGui::PopStyleColor();
    ImGui::GetWindowDrawList()->AddText(pos, IM_COL32(0, 255, 0, 255), ICON_FA_PLUS);
    ImGui::PopFont();
    
    if (ImGui::BeginPopup("AddActor"))
    {
        static const char* ItemLabels[] = {
            "Empty Actor",
            "Cube",
            "Sphere",
            "SpotLight",
            "Particle",
            "Text"
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
                    SpawnedActor->SetActorLabel(TEXT("Actor"));
                }
                else if (label == FString("Cube"))
                {
                    AStaticMeshActor* TempActor = World->SpawnActor<AStaticMeshActor>();
                    SpawnedActor = TempActor;
                    TempActor->SetActorLabel(TEXT("OBJ_CUBE"));
                    UStaticMeshComponent* MeshComp = TempActor->GetStaticMeshComponent();
                    FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
                    MeshComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
                }
                else if (label == FString("Sphere"))
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_SPHERE"));
                    SpawnedActor->AddComponent<USphereComp>();
                }
                else if (label == FString("SpotLight"))
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_SpotLight"));
                    SpawnedActor->AddComponent<ULightComponentBase>();
                    auto BillBoardComponent = SpawnedActor->AddComponent<UBillboardComponent>();
                    BillBoardComponent->SetTexture(L"Assets/Texture/spotLight.png");
                }
                else if (label == FString("Particle"))
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_PARTICLE"));
                    UParticleSubUVComp* ParticleComponent = SpawnedActor->AddComponent<UParticleSubUVComp>();
                    ParticleComponent->SetTexture(L"Assets/Texture/T_Explosion_SubUV.png");
                    ParticleComponent->SetRowColumnCount(6, 6);
                    ParticleComponent->SetLocalScale(FVector(10.0f, 10.0f, 1.0f));
                    ParticleComponent->Activate();
                }
                else if (label == FString("Text"))
                {
                    SpawnedActor = World->SpawnActor<AActor>();
                    SpawnedActor->SetActorLabel(TEXT("OBJ_Text"));
                    UText* TextComponent = SpawnedActor->AddComponent<UText>();
                    TextComponent->SetTexture(L"Assets/Texture/font.png");
                    TextComponent->SetRowColumnCount(106, 106);
                    TextComponent->SetText(L"안녕하세요 Jungle 1");
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
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.03f, 0.03f, 0.03f, 1.f));
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
    ImGui::PopStyleColor();
    ImGui::PopFont();
}