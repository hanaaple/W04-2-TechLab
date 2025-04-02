#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

void AStaticMeshActor::CopyPropertiesFrom(UObject* Source)
{
    AActor::CopyPropertiesFrom(Source);
    const AStaticMeshActor* SourceStaticMeshActor = Cast<AStaticMeshActor>(Source);
    if (SourceStaticMeshActor != nullptr)
    {
        StaticMeshComponent = FObjectFactory::DuplicateObject(SourceStaticMeshActor->GetStaticMeshComponent());
    }
}
