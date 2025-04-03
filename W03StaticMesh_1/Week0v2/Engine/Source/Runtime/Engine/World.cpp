#include "Engine/Source/Runtime/Engine/World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"


void UWorld::Initialize()
{
    // TODO: Load Scene
    CreateBaseObject();
    //SpawnObject(OBJ_CUBE);
    FManagerOBJ::CreateStaticMesh("Assets/Dodge/Dodge.obj");

    FManagerOBJ::CreateStaticMesh("Assets/SkySphere.obj");
    AActor* SpawnedActor = SpawnActor<AActor>();
    UStaticMeshComponent* dodge = SpawnedActor->AddComponent<UStaticMeshComponent>();
    dodge->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Dodge.obj"));

    AActor* duplicatedActor = FObjectFactory::DuplicateObject(SpawnedActor, SpawnedActor->GetClass());

    AddtoActorsArray(duplicatedActor);
}

void UWorld::CreateBaseObject()
{
    if (EditorPlayer == nullptr)
    {
        EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>();;
    }

    if (camera == nullptr)
    {
        camera = FObjectFactory::ConstructObject<UCameraComponent>();
        camera->SetLocation(FVector(8.0f, 8.0f, 8.f));
        camera->SetRotation(FVector(0.0f, 45.0f, -135.0f));
    }

    if (LocalGizmo == nullptr)
    {
        LocalGizmo = FObjectFactory::ConstructObject<UTransformGizmo>();
    }
}

void UWorld::ReleaseBaseObject()
{
    if (LocalGizmo)
    {
        delete LocalGizmo;
        LocalGizmo = nullptr;
    }

    if (worldGizmo)
    {
        delete worldGizmo;
        worldGizmo = nullptr;
    }

    if (camera)
    {
        delete camera;
        camera = nullptr;
    }

    if (EditorPlayer)
    {
        delete EditorPlayer;
        EditorPlayer = nullptr;
    }

}

void UWorld::Tick(float DeltaTime)
{

	camera->TickComponent(DeltaTime);
	EditorPlayer->Tick(DeltaTime);
	LocalGizmo->Tick(DeltaTime);

    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();

    // 매 틱마다 Actor->Tick(...) 호출
	for (AActor* Actor : ActorsArray)
	{
	    Actor->Tick(DeltaTime);
	}
}

void UWorld::Release()
{
	for (AActor* Actor : ActorsArray)
	{
		Actor->EndPlay(EEndPlayReason::WorldTransition);
        TSet<UActorComponent*> Components = Actor->GetComponents();
	    for (UActorComponent* Component : Components)
	    {
	        GUObjectArray.MarkRemoveObject(Component);
	    }
	    GUObjectArray.MarkRemoveObject(Actor);
	}
    ActorsArray.Empty();

	pickingGizmo = nullptr;
	ReleaseBaseObject();

    GUObjectArray.ProcessPendingDestroyObjects();
}

void UWorld::AddtoActorsArray(AActor* spawnedActor)
{
    ActorsArray.Add(spawnedActor);
    PendingBeginPlayActors.Add(spawnedActor);
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TSet<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    ActorsArray.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

void UWorld::CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap)
{
    Super::CopyPropertiesFrom(Source, DupMap);
    const UWorld* SourceUWorld = Cast<UWorld>(Source);
    if (SourceUWorld)
    {
        for (auto item : SourceUWorld->ActorsArray)
        {
            ActorsArray.Add(FObjectFactory::DuplicateObject(item, item->GetClass(), DupMap));
        }

        for (auto item : SourceUWorld->PendingBeginPlayActors)
        {
            PendingBeginPlayActors.Add(FObjectFactory::DuplicateObject(item, item->GetClass(), DupMap));
        }

        SelectedActor = FObjectFactory::DuplicateObject(SourceUWorld->SelectedActor, SourceUWorld->SelectedActor->GetClass(), DupMap);

        pickingGizmo = FObjectFactory::DuplicateObject(SourceUWorld->pickingGizmo, SourceUWorld->pickingGizmo->GetClass(), DupMap);
        camera = FObjectFactory::DuplicateObject(SourceUWorld->camera, SourceUWorld->camera->GetClass(), DupMap);
        EditorPlayer = FObjectFactory::DuplicateObject(SourceUWorld->EditorPlayer, SourceUWorld->EditorPlayer->GetClass(), DupMap);

        worldGizmo = FObjectFactory::DuplicateObject(SourceUWorld->worldGizmo, SourceUWorld->worldGizmo->GetClass(), DupMap);
        LocalGizmo = FObjectFactory::DuplicateObject(SourceUWorld->LocalGizmo, SourceUWorld->LocalGizmo->GetClass(), DupMap);
    }
}

void UWorld::CopyPropertiesTo(UObject* Dest, TMap<UObject*, UObject*>& DupMap)
{
    UObject::CopyPropertiesTo(Dest, DupMap);
    UWorld* DestUWorld = Cast<UWorld>(Dest);
    if (DestUWorld)
    {
        for (auto item : ActorsArray)
        {
            DestUWorld->ActorsArray.Add(FObjectFactory::DuplicateObject(item, item->GetClass(), DupMap));
        }

        for (auto item : PendingBeginPlayActors)
        {
            DestUWorld->PendingBeginPlayActors.Add(FObjectFactory::DuplicateObject(item, item->GetClass(), DupMap));
        }

        if (SelectedActor)
        {
            DestUWorld->SelectedActor = FObjectFactory::DuplicateObject(SelectedActor, SelectedActor->GetClass(), DupMap);
        }

        if (pickingGizmo)
        {
            DestUWorld->pickingGizmo = FObjectFactory::DuplicateObject(pickingGizmo, pickingGizmo->GetClass(), DupMap);
        }

        if (camera)
        {
            DestUWorld->camera = FObjectFactory::DuplicateObject(camera, camera->GetClass(), DupMap);
        }

        if (EditorPlayer)
        {
            DestUWorld->EditorPlayer = FObjectFactory::DuplicateObject(EditorPlayer, EditorPlayer->GetClass(), DupMap);
        }

        if (worldGizmo)
        {
            DestUWorld->worldGizmo = FObjectFactory::DuplicateObject(worldGizmo, worldGizmo->GetClass(), DupMap);
        }

        if (LocalGizmo)
        {
            DestUWorld->LocalGizmo = FObjectFactory::DuplicateObject(LocalGizmo, LocalGizmo->GetClass(), DupMap);
        }
    }
}

void UWorld::SetPickingGizmo(UObject* Object)
{
	pickingGizmo = Cast<USceneComponent>(Object);
}
