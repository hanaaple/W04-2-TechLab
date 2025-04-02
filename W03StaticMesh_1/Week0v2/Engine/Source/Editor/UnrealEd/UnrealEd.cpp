#include "UnrealEd.h"
#include "EditorPanel.h"
#include "World.h"
#include "Editor/EditorEngine.h"

#include "PropertyEditor/ViewControlEditorPanel.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"
#include "PropertyEditor/WorldControlEditorPanel.h"

extern FEditorEngine GEngineLoop;

void UnrealEd::Initialize()
{
    auto WorldControlPanel = std::make_shared<WorldControlEditorPanel>();
    Panels["WorldControlPanel"] = WorldControlPanel;
    
    auto ViewportControlPanel = std::make_shared<ViewportControlEditorPanel>();
    Panels["ViewportControlPanel"] = ViewportControlPanel;
    
    auto OutlinerPanel = std::make_shared<OutlinerEditorPanel>();
    Panels["OutlinerPanel"] = OutlinerPanel;
    
    auto PropertyPanel = std::make_shared<PropertyEditorPanel>();
    Panels["PropertyPanel"] = PropertyPanel;
}

void UnrealEd::Render() const
{
    for (const auto& Panel : Panels)
    {
        Panel.Value->Render();
    }
}

void UnrealEd::AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel)
{
    Panels[PanelId] = EditorPanel;
}

void UnrealEd::OnResize(HWND hWnd) const
{
    for (auto& Panel : Panels)
    {
        Panel.Value->OnResize(hWnd);
    }
}

std::shared_ptr<UEditorPanel> UnrealEd::GetEditorPanel(const FString& PanelId)
{
    return Panels[PanelId];
}
