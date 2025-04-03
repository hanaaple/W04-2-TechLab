#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

void AStaticMeshActor::CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap)
{
    AActor::CopyPropertiesFrom(Source, DupMap);
    const AStaticMeshActor* SourceStaticMeshActor = Cast<AStaticMeshActor>(Source);
    if (SourceStaticMeshActor != nullptr)
    {
        StaticMeshComponent = FObjectFactory::DuplicateObject(SourceStaticMeshActor->StaticMeshComponent, SourceStaticMeshActor->StaticMeshComponent->GetClass(), DupMap);
    }
}

void AStaticMeshActor::CopyPropertiesTo(UObject* Dest, TMap<UObject*, UObject*>& DupMap)
{
    AActor::CopyPropertiesTo(Dest, DupMap);
    AStaticMeshActor* DestStaticMeshActor = Cast<AStaticMeshActor>(Dest);
    if (DestStaticMeshActor != nullptr)
    {
        DestStaticMeshActor->StaticMeshComponent = FObjectFactory::DuplicateObject(StaticMeshComponent, DestStaticMeshActor->StaticMeshComponent->GetClass(), DupMap);
    }
}
