#include "UnrealEd.h"
#include "EditorPanel.h"

#include "PropertyEditor/ControlEditorPanel.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"
#include "PropertyEditor/FPSEditorPanel.h"

void UnrealEd::Initialize()
{
    Panels[TEXT("ControlPanel")] = std::make_shared<ControlEditorPanel>();
    
    //Panels[TEXT("OutlinerPanel")] = std::make_shared<OutlinerEditorPanel>();
    
    //Panels["PropertyPanel"] = std::make_shared<PropertyEditorPanel>();

    Panels["FPSPanel"] =std::make_shared<FPSEditorPanel>();
}

void UnrealEd::Render() const
{
    for (const auto& Panel : Panels)
    {
        if (Panel.Value.get() == nullptr) continue;
        Panel.Value.get()->Render();
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
