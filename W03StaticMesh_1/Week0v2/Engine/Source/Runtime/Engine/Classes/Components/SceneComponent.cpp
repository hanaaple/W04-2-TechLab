#include "Components/SceneComponent.h"
#include "World.h"
#include "Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "UTextUUID.h"
USceneComponent::USceneComponent() :RelativeLocation(FVector(0.f, 0.f, 0.f)), RelativeRotation(FVector(0.f, 0.f, 0.f)), RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}

USceneComponent::~USceneComponent()
{
	if (uuidText) delete uuidText;
}
void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}


int USceneComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    int nIntersections = 0;
    return nIntersections;
}

FVector USceneComponent::GetForwardVector()
{
	FVector Forward = FVector(1.f, 0.f, 0.0f);
	Forward = JungleMath::FVectorRotate(Forward, QuatRotation);
	return Forward;
}

FVector USceneComponent::GetRightVector()
{
	FVector Right = FVector(0.f, 1.f, 0.0f);
	Right = JungleMath::FVectorRotate(Right, QuatRotation);
	return Right;
}

FVector USceneComponent::GetUpVector()
{
	FVector Up = FVector(0.f, 0.f, 1.0f);
	Up = JungleMath::FVectorRotate(Up, QuatRotation);
	return Up;
}


void USceneComponent::AddLocation(FVector _added)
{
	RelativeLocation = RelativeLocation + _added;

}

void USceneComponent::AddRotation(FVector _added)
{
	RelativeRotation = RelativeRotation + _added;

}

void USceneComponent::AddScale(FVector _added)
{
	RelativeScale3D = RelativeScale3D + _added;

}


USceneComponent* USceneComponent::Duplicate()
{
    FDuplicateContext Context;
    return dynamic_cast<USceneComponent*>( Duplicate(Context));
}

UObject* USceneComponent::Duplicate(FDuplicateContext& Context)
{
    if (Context.DuplicateMap.Contains(this))
    {
        return Context.DuplicateMap[this];
    }
    
    USceneComponent* DuplicatedObject = reinterpret_cast<USceneComponent*>(Super::Duplicate(Context));
    Context.DuplicateMap.Add(this, DuplicatedObject);
    
    memcpy(reinterpret_cast<char*>(DuplicatedObject) + sizeof(Super),
           reinterpret_cast<char*>(this) + sizeof(Super),
           sizeof(USceneComponent) - sizeof(Super));
    
    DuplicatedObject->RelativeLocation = this->RelativeLocation;
    DuplicatedObject->RelativeRotation = this->RelativeRotation;
    DuplicatedObject->RelativeScale3D = this->RelativeScale3D;
    DuplicatedObject->QuatRotation = this->QuatRotation;

    if (this->AttachParent != nullptr)
    {
        DuplicatedObject->AttachParent = Cast<USceneComponent>(this->AttachParent->Duplicate(Context));
    }

    memset(&DuplicatedObject->AttachChildren, 0, sizeof(DuplicatedObject->AttachChildren));
    for (const auto item : this->AttachChildren)
    {
        DuplicatedObject->AttachChildren.Add(Cast<USceneComponent>(item->Duplicate(Context)));
    }

    return DuplicatedObject;
}

FVector USceneComponent::GetWorldRotation()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetLocalRotation() + GetLocalRotation());
	}
	else
		return GetLocalRotation();
}

FVector USceneComponent::GetWorldScale()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetWorldScale() + GetLocalScale());
	}
	else
		return GetLocalScale();
}

FVector USceneComponent::GetWorldLocation()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetWorldLocation() + GetLocalLocation());
	}
	else
		return GetLocalLocation();
}

FVector USceneComponent::GetLocalRotation()
{
	return JungleMath::QuaternionToEuler(QuatRotation);
}

void USceneComponent::SetLocalRotation(FVector _newRot)
{
	RelativeRotation = _newRot;
	QuatRotation = JungleMath::EulerToQuaternion(_newRot);
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (
        InParent != AttachParent                                  // 설정하려는 Parent가 기존의 Parent와 다르거나
        && InParent != this                                       // InParent가 본인이 아니고
        && InParent != nullptr                                    // InParent가 유효한 포인터 이며
        && (
            AttachParent == nullptr                               // AttachParent도 유효하며
            || !AttachParent->AttachChildren.Contains(this)  // 이미 AttachParent의 자식이 아닌 경우
        ) 
    ) {
        AttachParent = InParent;
        InParent->AttachChildren.AddUnique(this);
    }
}

void USceneComponent::SetToComponent(USceneComponent* InParent)
{
    if (AttachParent != nullptr)
    {
        DetachFromComponent();
    }
    SetupAttachment(InParent);
}

void USceneComponent::DetachFromComponent()
{
    if (AttachParent == nullptr)
        return;

    AActor* Owner = GetOwner();

    if (Owner && Owner->GetRootComponent() && Owner->GetRootComponent()->AttachChildren.Contains(this))
    {
        Owner->GetRootComponent()->AttachChildren.Remove(this);
        AttachParent = nullptr;
    }
}
