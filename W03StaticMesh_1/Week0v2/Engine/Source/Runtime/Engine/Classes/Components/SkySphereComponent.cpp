#include "SkySphereComponent.h"

#include "World.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"


USkySphereComponent::USkySphereComponent()
{
    SetType(StaticClass()->GetName());
}

USkySphereComponent::~USkySphereComponent()
{
}

void USkySphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USkySphereComponent::TickComponent(float DeltaTime)
{
    UOffset += 0.005f;
    VOffset += 0.005f;
    Super::TickComponent(DeltaTime);
}

void USkySphereComponent::CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap)
{
    Super::CopyPropertiesFrom(Source, DupMap);

    const USkySphereComponent* SourceUSkySphereComponent = Cast<USkySphereComponent>(Source);
    if (SourceUSkySphereComponent)
    {
        UOffset = SourceUSkySphereComponent->UOffset;
        VOffset = SourceUSkySphereComponent->VOffset;
    }
}

void USkySphereComponent::CopyPropertiesTo(UObject* Source, TMap<UObject*, UObject*>& OutMap)
{
    UStaticMeshComponent::CopyPropertiesTo(Source, OutMap);
    USkySphereComponent* DestSkySphereComponent = Cast<USkySphereComponent>(Source);
    if (DestSkySphereComponent)
    {
        DestSkySphereComponent->UOffset = UOffset;
        DestSkySphereComponent->VOffset = VOffset;
    }
}
