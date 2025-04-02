#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

UObject* AStaticMeshActor::Duplicate()
{
    AStaticMeshActor* duplicated = Cast<AStaticMeshActor>(FObjectFactory::DuplicateObject(this, this->GetClass()));
    duplicated->StaticMeshComponent = Cast<UStaticMeshComponent>(FObjectFactory::DuplicateObject(this->StaticMeshComponent, this->StaticMeshComponent->GetClass()));
    Super::Duplicate();
    return duplicated;
}
