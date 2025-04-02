#include "Engine/Source/Runtime/Engine/World.h"

#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Engine/FLoaderOBJ.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"


void UWorld::Initialize(EWorldType::Type worldType)
{
    // TODO: Load Scene
    CreateBaseObject();
    Level = FObjectFactory::ConstructObject<ULevel>();
    Level->LevelState = ELevelState::Stop;
    //SpawnObject(OBJ_CUBE);
    FManagerOBJ::CreateStaticMesh("Assets/Dodge/Dodge.obj");

    FManagerOBJ::CreateStaticMesh("Assets/SkySphere.obj");
    AActor* SpawnedActor = SpawnActor<AActor>();
    UStaticMeshComponent* skySphere = SpawnedActor->AddComponent<UStaticMeshComponent>();
    skySphere->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Dodge.obj"));
    //skySphere->GetStaticMesh()->GetMaterials()[0]->Material->SetDiffuse(FVector((float)32/255, (float)171/255, (float)191/255));

    AActor* dupActor = Cast<AActor>(SpawnedActor->Duplicate());
    
    WorldType = worldType;
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
        camera->SetLocalLocation(FVector(8.0f, 8.0f, 8.f));
        camera->SetLocalRotation(FVector(0.0f, 45.0f, -135.0f));
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
	// camera->TickComponent(DeltaTime);
	EditorPlayer->Tick(DeltaTime);
	LocalGizmo->Tick(DeltaTime);

    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();

    // 매 틱마다 Actor->Tick(...) 호출
	// for (AActor* Actor : ActorsArray)
	// {
	//     Actor->Tick(DeltaTime);
	// }
    if (Level->LevelState != ELevelState::Play)
        return;
    Level->Tick(DeltaTime);
}

void UWorld::Release()
{
	for (AActor* Actor : Level->Actors)
	{
		Actor->EndPlay(EEndPlayReason::WorldTransition);
        TSet<UActorComponent*> Components = Actor->GetComponents();
	    for (UActorComponent* Component : Components)
	    {
	        GUObjectArray.MarkRemoveObject(Component);
	    }
	    GUObjectArray.MarkRemoveObject(Actor);
	}
    Level->Actors.Empty();

	pickingGizmo = nullptr;
	ReleaseBaseObject();

    GUObjectArray.ProcessPendingDestroyObjects();
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
    Level->Actors.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

UObject* UWorld::Duplicate()
{
     UWorld* dup = Cast<UWorld>(FObjectFactory::DuplicateObject(this, this->GetClass()));
     dup->Level = Cast<ULevel>(Level->Duplicate());
    
     dup->PendingBeginPlayActors.Empty();
     for (const auto item : this->PendingBeginPlayActors)
     {
         dup->PendingBeginPlayActors.Add(Cast<AActor>(item->Duplicate()));
     }

    if (this->SelectedActor != nullptr)
         dup->SelectedActor = Cast<AActor>(this->SelectedActor->Duplicate());

    if (this->SelectedComponent != nullptr)
        dup->SelectedComponent = Cast<UActorComponent>(this->SelectedComponent->Duplicate());

    if (this->pickingGizmo != nullptr)
         dup->pickingGizmo = Cast<USceneComponent>(this->pickingGizmo->Duplicate());

    if (this->camera != nullptr)
        dup->camera = Cast<UCameraComponent>(this->camera->Duplicate());

    if (this->EditorPlayer != nullptr)
        dup->EditorPlayer = Cast<AEditorPlayer>(this->EditorPlayer->Duplicate());

    if (this->worldGizmo != nullptr)
         dup->worldGizmo = Cast<UObject>(this->worldGizmo->Duplicate());
    
    
    return dup;
}

void UWorld::SetPickingGizmo(UObject* Object)
{
	pickingGizmo = Cast<USceneComponent>(Object);
}
