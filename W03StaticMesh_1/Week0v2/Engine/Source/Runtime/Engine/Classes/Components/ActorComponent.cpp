#include "ActorComponent.h"

#include "GameFramework/Actor.h"


void UActorComponent::InitializeComponent()
{
    assert(!bHasBeenInitialized);

    bHasBeenInitialized = true;
}

void UActorComponent::UninitializeComponent()
{
    assert(bHasBeenInitialized);

    bHasBeenInitialized = false;
}

void UActorComponent::BeginPlay()
{
    bHasBegunPlay = true;
}

void UActorComponent::TickComponent(float DeltaTime)
{
}

void UActorComponent::OnComponentDestroyed()
{
}

void UActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    assert(bHasBegunPlay);

    bHasBegunPlay = false;
}

UObject* UActorComponent::Duplicate()
{
    UActorComponent* duplicated = Cast<UActorComponent>(FObjectFactory::DuplicateObject(this, this->GetClass()));
    if (this->Owner)
    {
        duplicated->Owner = Cast<AActor>(this->Owner->Duplicate());
    }

    duplicated->bHasBeenInitialized = this->bHasBeenInitialized;
    duplicated->bHasBegunPlay = this->bHasBegunPlay;
    duplicated->bIsBeingDestroyed = this->bIsBeingDestroyed;
    duplicated->bIsActive = this->bIsActive;
    duplicated->bAutoActive = this->bAutoActive;


    return duplicated;
}

void UActorComponent::DestroyComponent()
{
    if (bIsBeingDestroyed)
    {
        return;
    }

    bIsBeingDestroyed = true;

    // Owner에서 Component 제거하기
    if (AActor* MyOwner = GetOwner())
    {
        MyOwner->RemoveOwnedComponent(this);
        if (MyOwner->GetRootComponent() == this)
        {
            MyOwner->SetRootComponent(nullptr);
        }
    }

    if (bHasBegunPlay)
    {
        EndPlay(EEndPlayReason::Destroyed);
    }

    if (bHasBeenInitialized)
    {
        UninitializeComponent();
    }

    OnComponentDestroyed();

    // 나중에 ProcessPendingDestroyObjects에서 실제로 제거됨
    GUObjectArray.MarkRemoveObject(this);
}

void UActorComponent::Activate()
{
    // TODO: Tick 다시 재생
    bIsActive = true;
}

void UActorComponent::Deactivate()
{
    // TODO: Tick 멈추기
    bIsActive = false;
}
