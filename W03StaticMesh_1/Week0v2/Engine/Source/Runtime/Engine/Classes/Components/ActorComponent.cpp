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

UActorComponent* UActorComponent::Duplicate()
{
    FDuplicateContext Context;
    return dynamic_cast<UActorComponent*>( Duplicate(Context));
}

UObject* UActorComponent::Duplicate(FDuplicateContext& Context)
{
    if (Context.DuplicateMap.Contains(this))
    {
        return Context.DuplicateMap[this];
    }
    
    UActorComponent* DuplicatedObject = reinterpret_cast<UActorComponent*>(Super::Duplicate(Context));
    Context.DuplicateMap.Add(this, DuplicatedObject);
    
    memcpy(reinterpret_cast<char*>(DuplicatedObject) + sizeof(Super),
           reinterpret_cast<char*>(this) + sizeof(Super),
           sizeof(UActorComponent) - sizeof(Super));    
    
    if (this->Owner)
    {
        DuplicatedObject->Owner = Cast<AActor>(this->Owner->Duplicate(Context));
    }

    DuplicatedObject->bHasBeenInitialized = this->bHasBeenInitialized;
    DuplicatedObject->bHasBegunPlay = this->bHasBegunPlay;
    DuplicatedObject->bIsBeingDestroyed = this->bIsBeingDestroyed;
    DuplicatedObject->bIsActive = this->bIsActive;
    DuplicatedObject->bAutoActive = this->bAutoActive;

    return DuplicatedObject;
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
