#include "Actor.h"

#include "World.h"

void AActor::BeginPlay()
{
    // TODO: 나중에 삭제를 Pending으로 하던가 해서 복사비용 줄이기
    const auto CopyComponents = OwnedComponents;
    for (UActorComponent* Comp : CopyComponents)
    {
        Comp->BeginPlay();
    }
}

void AActor::Tick(float DeltaTime)
{
    // TODO: 임시로 Actor에서 Tick 돌리기
    // TODO: 나중에 삭제를 Pending으로 하던가 해서 복사비용 줄이기
    const auto CopyComponents = OwnedComponents;
    for (UActorComponent* Comp : CopyComponents)
    {
        Comp->TickComponent(DeltaTime);
    }
}

void AActor::Destroyed()
{
    // Actor가 제거되었을 때 호출하는 EndPlay
    EndPlay(EEndPlayReason::Destroyed);
}

void AActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 본인이 소유하고 있는 모든 컴포넌트의 EndPlay 호출
    for (UActorComponent* Component : GetComponents())
    {
        if (Component->HasBegunPlay())
        {
            Component->EndPlay(EndPlayReason);
        }
    }
    UninitializeComponents();
}

bool AActor::Destroy()
{
    if (!IsActorBeingDestroyed())
    {
        if (UWorld* World = GetWorld())
        {
            World->DestroyActor(this);
            bActorIsBeingDestroyed = true;
        }
    }

    return IsActorBeingDestroyed();
}

void AActor::RemoveOwnedComponent(UActorComponent* Component)
{
    OwnedComponents.Remove(Component);
}

void AActor::InitializeComponents()
{
    TArray<UActorComponent*> Components = GetComponents();
    for (UActorComponent* ActorComp : Components)
    {
        if (ActorComp->bAutoActive && !ActorComp->IsActive())
        {
            ActorComp->Activate();
        }

        if (!ActorComp->HasBeenInitialized())
        {
            ActorComp->InitializeComponent();
        }
    }
}

void AActor::UninitializeComponents()
{
    TArray<UActorComponent*> Components = GetComponents();
    for (UActorComponent* ActorComp : Components)
    {
        if (ActorComp->HasBeenInitialized())
        {
            ActorComp->UninitializeComponent();
        }
    }
}

bool AActor::SetRootComponent(USceneComponent* NewRootComponent)
{
    if (NewRootComponent == nullptr || NewRootComponent->GetOwner() == this)
    {
        if (RootComponent != NewRootComponent)
        {
            USceneComponent* OldRootComponent = RootComponent;
            RootComponent = NewRootComponent;

            OldRootComponent->SetupAttachment(RootComponent);
        }
        return true;
    }
    return false;
}

bool AActor::SetActorLocation(const FVector& NewLocation)
{
    if (RootComponent)
    {
        RootComponent->SetLocalLocation(NewLocation);
        return true;
    }
    return false;
}

bool AActor::SetActorRotation(const FVector& NewRotation)
{
    if (RootComponent)
    {
        RootComponent->SetLocalRotation(NewRotation);
        return true;
    }
    return false;
}

bool AActor::SetActorScale(const FVector& NewScale)
{
    if (RootComponent)
    {
        RootComponent->SetLocalScale(NewScale);
        return true;
    }
    return false;
}

AActor* AActor::Duplicate()
{
    FDuplicateContext Context;
    return Cast<AActor>( Duplicate(Context));
}

UObject* AActor::Duplicate(FDuplicateContext& Context)
{
    if (Context.DuplicateMap.Contains(this))
    {
        return Context.DuplicateMap[this];
    }
    
    // Super::Duplicate(Context)로 UObject의 Duplicate를 호출합니다.
    AActor* DuplicatedObject = reinterpret_cast<AActor*>(Super::Duplicate(Context));
    Context.DuplicateMap.Add(this, DuplicatedObject);

    // UObject 부분 이후(즉, Super 부분)부터 AActor에 추가된 멤버들을 복사합니다.
    memcpy(reinterpret_cast<char*>(DuplicatedObject) + sizeof(Super),
           reinterpret_cast<char*>(this) + sizeof(Super),
           sizeof(AActor) - sizeof(Super));

    if (this->Owner != nullptr)
    {
        // 소유자가 있다면, 소유자도 컨텍스트를 이용해 복제합니다.
        DuplicatedObject->Owner = reinterpret_cast<AActor*>(this->Owner->Duplicate(Context));
    }
    DuplicatedObject->bActorIsBeingDestroyed = this->bActorIsBeingDestroyed;

    // OwnedComponents 메모리 초기화
    memset(&DuplicatedObject->OwnedComponents, 0, sizeof(DuplicatedObject->OwnedComponents));

    if (this->RootComponent != nullptr)
    {
        DuplicatedObject->RootComponent = reinterpret_cast<USceneComponent*>(this->RootComponent->Duplicate(Context));
        DuplicatedObject->OwnedComponents.Add(DuplicatedObject->RootComponent);
        DuplicatedObject->RootComponent->Owner = DuplicatedObject;
    }
    
    // 소유한 컴포넌트들에 대해 복제를 수행 (RootComponent는 이미 처리했으므로 건너뜁니다)
    for (const auto comp : this->OwnedComponents)
    {
        if (this->RootComponent == comp)
        {
            continue;
        }
        const auto CopiedComp = reinterpret_cast<UActorComponent*>(comp->Duplicate(Context));
        DuplicatedObject->DuplicateComponent(CopiedComp);
    }
    
    DuplicatedObject->ActorLabel = this->ActorLabel;
    return DuplicatedObject;
}
