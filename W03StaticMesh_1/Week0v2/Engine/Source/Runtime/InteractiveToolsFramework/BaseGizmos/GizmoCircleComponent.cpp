#include "GizmoCircleComponent.h"

#include "World.h"
#include "Math/JungleMath.h"
#include "Actors/Player.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#define DISC_RESOLUTION 128

UGizmoCircleComponent::UGizmoCircleComponent()
{
}

UGizmoCircleComponent::~UGizmoCircleComponent()
{
}

bool UGizmoCircleComponent::IntersectsRay(const FVector& rayOrigin, const FVector& rayDir, float& dist)
{
    if (rayDir.y == 0) return false; // normal to normal vector of plane

    dist = -rayOrigin.y / rayDir.y;

    FVector intersectionPoint = rayOrigin + rayDir * dist;
    float intersectionToDiscCenterSquared = intersectionPoint.Magnitude();

    return (inner * inner < intersectionToDiscCenterSquared && intersectionToDiscCenterSquared < 1);
}

void UGizmoCircleComponent::CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap)
{
    Super::CopyPropertiesFrom(Source, DupMap);
    const UGizmoCircleComponent* SourceUGizmoComponent = Cast<UGizmoCircleComponent>(Source);
    if (SourceUGizmoComponent)
    {
        inner = SourceUGizmoComponent->inner;
    }
}

void UGizmoCircleComponent::CopyPropertiesTo(UObject* Dest, TMap<UObject*, UObject*>& OutMap)
{
    UGizmoBaseComponent::CopyPropertiesTo(Dest, OutMap);
    UGizmoCircleComponent* DestGizmoComponent = Cast<UGizmoCircleComponent>(Dest);
    if (DestGizmoComponent)
    {
        DestGizmoComponent->inner = inner;
    }
}
