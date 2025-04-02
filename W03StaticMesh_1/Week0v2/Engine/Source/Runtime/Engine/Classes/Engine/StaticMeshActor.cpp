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
    duplicated->StaticMeshComponent = Cast<UStaticMeshComponent>(this->StaticMeshComponent->Duplicate());
 
    return duplicated;
}
