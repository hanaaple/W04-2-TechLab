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
