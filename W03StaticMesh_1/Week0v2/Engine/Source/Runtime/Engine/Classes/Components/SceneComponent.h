#pragma once
#include "ActorComponent.h"
#include "Math/Quat.h"
#include "Core/Container/Array.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();
    virtual ~USceneComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    virtual FVector GetForwardVector();
    virtual FVector GetRightVector();
    virtual FVector GetUpVector();
    void AddLocation(FVector _added);
    void AddRotation(FVector _added);
    void AddScale(FVector _added);

    USceneComponent* Duplicate();

    UObject* Duplicate(FDuplicateContext& Context) override;
protected:
    FVector RelativeLocation;
    FVector RelativeRotation;
    FQuat QuatRotation;
    FVector RelativeScale3D;

    USceneComponent* AttachParent = nullptr;
    TArray<USceneComponent*> AttachChildren;

public:
    virtual FVector GetWorldRotation();
    FVector GetWorldScale();
    FVector GetWorldLocation();
    FVector GetLocalRotation();
    FQuat GetQuat() const { return QuatRotation; }

    FVector GetLocalScale() const { return RelativeScale3D; }
    FVector GetLocalLocation() const { return RelativeLocation; }

    void SetLocalLocation(FVector _newLoc) { RelativeLocation = _newLoc; }
    virtual void SetLocalRotation(FVector _newRot);
    void SetLocalRotation(FQuat _newRot) { QuatRotation = _newRot; }
    void SetLocalScale(FVector _newScale) { RelativeScale3D = _newScale; }

public:
    void SetupAttachment(USceneComponent* InParent);
    void SetToComponent(USceneComponent* InParent);
    void DetachFromComponent();
    inline TArray<USceneComponent*> GetAttachChildren() const { return AttachChildren; }
    inline uint32 GetChildrenCount() const { return AttachChildren.Num(); }

private:
    class UTextUUID* uuidText = nullptr;

public:
};
