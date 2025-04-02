#pragma once
#include "Define.h"
#include "Level.h"
#include "Container/Set.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"

class FObjectFactory;
class AActor;
class UObject;
class UGizmoArrowComponent;
class UCameraComponent;
class AEditorPlayer;
class USceneComponent;
class UTransformGizmo;

class UWorld : public UObject
{
    DECLARE_CLASS(UWorld, UObject)

public:
    UWorld() = default;

    void Initialize(EWorldType::Type);
    void CreateBaseObject();
    void ReleaseBaseObject();
    void Tick(float DeltaTime);
    void Release();

    /**
     * World에 Actor를 Spawn합니다.
     * @tparam T AActor를 상속받은 클래스
     * @return Spawn된 Actor의 포인터
     */
    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();
    
    void AddtoActorsArray(AActor* spawnedActor);

    /** World에 존재하는 Actor를 제거합니다. */
    bool DestroyActor(AActor* ThisActor);
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    UObject* Duplicate() override;

private:
    const FString defaultMapName = "Default";

    ULevel* Level;

    EWorldType::Type WorldType;
    
    /** World에서 관리되는 모든 Actor의 목록 */
    // TSet<AActor*> ActorsArray;
    /** Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들 */
    TArray<AActor*> PendingBeginPlayActors;

    AActor* SelectedActor = nullptr;
    UActorComponent* SelectedComponent = nullptr;

    USceneComponent* pickingGizmo = nullptr;
    UCameraComponent* camera = nullptr;
    AEditorPlayer* EditorPlayer = nullptr;

public:
    UObject* worldGizmo = nullptr;

    const TArray<AActor*>& GetActors() const { return Level->Actors; }

    UTransformGizmo* LocalGizmo = nullptr;
    UCameraComponent* GetCamera() const { return camera; }
    AEditorPlayer* GetEditorPlayer() const { return EditorPlayer; }
    EWorldType::Type GetWorldType() const { return WorldType; }

    // EditorManager 같은데로 보내기
    AActor* GetSelectedActor() const { return SelectedActor; }
     void SetPickedActor(AActor* InActor)
     {
         SelectedActor = InActor;
     }

    AActor* GetSelectedTempActor() const
    {
        AActor* Result = nullptr;

        if (SelectedComponent != nullptr)
        {
            Result = SelectedComponent->GetOwner();
        }
        
        if (Result == nullptr && SelectedActor != nullptr)
        {
            Result = SelectedActor;
        }

        return Result;
    }

    UActorComponent* GetSelectedTempComponent() const
    {
        if (SelectedComponent != nullptr)
        {
            return SelectedComponent;
        }

        if (SelectedActor != nullptr)
        {
            return SelectedActor->GetRootComponent();
        }
    }
    
    UActorComponent* GetSelectedComponent() const { return SelectedComponent; }
    void SetPickedComponent(UActorComponent* InActor)
    {
        SelectedComponent = InActor;
    }

    UObject* GetWorldGizmo() const { return worldGizmo; }
    USceneComponent* GetPickingGizmo() const { return pickingGizmo; }
    void SetPickingGizmo(UObject* Object);

    ULevel* GetLevel() const { return Level; }
};

template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::SpawnActor()
{
    T* Actor = FObjectFactory::ConstructObject<T>();
    // TODO: 일단 AddComponent에서 Component마다 초기화
    // 추후에 RegisterComponent() 만들어지면 주석 해제
    // Actor->InitializeComponents();
    Level->Actors.Add(Actor);
    PendingBeginPlayActors.Add(Actor);
    return Actor;
}
