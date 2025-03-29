#include "Player.h"

#include "UnrealClient.h"
#include "World.h"
#include "BaseGizmos/GizmoArrowComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Components/LightComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "Math/MathUtility.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "Stats/ScopeCycleCounter.h"
#include "PropertyEditor/FPSEditorPanel.h"
using namespace DirectX;

AEditorPlayer::AEditorPlayer()
{
}

void AEditorPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Input();
}

void AEditorPlayer::Input()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
        if (!bLeftMouseDown)
        {
            bLeftMouseDown = true;

            POINT mousePos;
            GetCursorPos(&mousePos);
            GetCursorPos(&m_LastMousePos);

            //uint32 UUID = GetEngine().graphicDevice.GetPixelUUID(mousePos);
            //// TArray<UObject*> objectArr = GetWorld()->GetObjectArr();
            //for ( const auto obj : TObjectRange<USceneComponent>())
            //{
            //    if (obj->GetUUID() != UUID) continue;

            //    UE_LOG(LogLevel::Display, *obj->GetName());
            //}

            ScreenToClient(GetEngine().hWnd, &mousePos);

            FVector origin;
            FVector direction;

            const auto& ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();
            // ScreenToViewSpace(mousePos.x, mousePos.y, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), pickPosition);
            ScreenToWorldSpace(
                mousePos.x,
                mousePos.y,
                ActiveViewport->GetViewMatrix(),
                ActiveViewport->GetProjectionMatrix(),
                origin,
                direction
            );
            UE_LOG(LogLevel::Display, TEXT("pickPosition: %f %f %f"), origin.x, origin.y, origin.z);
            UE_LOG(LogLevel::Display, TEXT("direction: %f %f %f"), direction.x, direction.y, direction.z);
            bool res = PickGizmo(origin, direction);
            if (!res) PickActor(origin, direction);
        }
        else
        {
            PickedObjControl();
        }
    }
    else
    {
        if (bLeftMouseDown)
        {
            bLeftMouseDown = false; // ���콺 ������ ��ư�� ���� ���� �ʱ�ȭ
            GetWorld()->SetPickingGizmo(nullptr);
        }
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        if (!bSpaceDown)
        {
            AddControlMode();
            bSpaceDown = true;
        }
    }
    else
    {
        if (bSpaceDown)
        {
            bSpaceDown = false;
        }
    }
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (!bRightMouseDown)
        {
            bRightMouseDown = true;
        }
    }
    else
    {
        bRightMouseDown = false;

        if (GetAsyncKeyState('Q') & 0x8000)
        {
            //GetWorld()->SetPickingObj(nullptr);
        }
        if (GetAsyncKeyState('W') & 0x8000)
        {
            cMode = CM_TRANSLATION;
        }
        if (GetAsyncKeyState('E') & 0x8000)
        {
            cMode = CM_ROTATION;
        }
        if (GetAsyncKeyState('R') & 0x8000)
        {
            cMode = CM_SCALE;
        }
    }

    if (GetAsyncKeyState(VK_DELETE) & 0x8000)
    {
        UWorld* World = GetWorld();
        if (AActor* PickedActor = World->GetSelectedActor())
        {
            World->DestroyActor(PickedActor);
            World->SetPickedActor(nullptr);
        }
    }
}

bool AEditorPlayer::PickGizmo(FVector& pickOrigin, FVector& pickDirection)
{
    bool isPickedGizmo = false;
    if (GetWorld()->GetSelectedActor())
    {
        if (cMode == CM_TRANSLATION)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetArrowArr())
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                if (!iter) continue;
                if (RayIntersectsObject(pickOrigin, pickDirection, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                }
            }
        }
        else if (cMode == CM_ROTATION)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetDiscArr())
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                //UPrimitiveComponent* localGizmo = dynamic_cast<UPrimitiveComponent*>(GetWorld()->LocalGizmo[i]);
                if (!iter) continue;
                if (RayIntersectsObject(pickOrigin, pickDirection, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                }
            }
        }
        else if (cMode == CM_SCALE)
        {
            for (auto iter : GetWorld()->LocalGizmo->GetScaleArr())
            {
                int maxIntersect = 0;
                float minDistance = FLT_MAX;
                float Distance = 0.0f;
                int currentIntersectCount = 0;
                if (!iter) continue;
                if (RayIntersectsObject(pickOrigin, pickDirection, iter, Distance, currentIntersectCount))
                {
                    if (Distance < minDistance)
                    {
                        minDistance = Distance;
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                    else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                    {
                        maxIntersect = currentIntersectCount;
                        GetWorld()->SetPickingGizmo(iter);
                        isPickedGizmo = true;
                    }
                }
            }
        }
    }
    return isPickedGizmo;
}

void AEditorPlayer::PickActor(const FVector& pickOrigin, const FVector& pickDirection)
{
    TStatId dummy;
    FScopeCycleCounter counter(dummy);

    if (!(ShowFlags::GetInstance().currentFlags & EEngineShowFlags::SF_Primitives)) return;

    const UActorComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;

    // iterate all PrimitiveComponent
    //for (const auto iter : TObjectRange<UPrimitiveComponent>())
    //{
    //    UPrimitiveComponent* pObj;
    //    if (iter->IsA<UPrimitiveComponent>() || iter->IsA<ULightComponentBase>())
    //    {
    //        pObj = static_cast<UPrimitiveComponent*>(iter);
    //    }
    //    else
    //    {
    //        continue;
    //    }
    //    if (pObj && !pObj->IsA<UGizmoBaseComponent>())
    //    {
    //        float Distance = 0.0f;
    //        int currentIntersectCount = 0;
    //        if (RayIntersectsObject(pickOrigin, pickDirection, pObj, Distance, currentIntersectCount))
    //        {
    //            if (Distance < minDistance)
    //            {
    //                minDistance = Distance;
    //                maxIntersect = currentIntersectCount;
    //                Possible = pObj;
    //            }
    //            else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
    //            {
    //                maxIntersect = currentIntersectCount;
    //                Possible = pObj;
    //            }
    //        }
    //    }
    //}

    // travese octree
    TArray<UStaticMeshComponent*> PossibleList = TArray<UStaticMeshComponent*>();

    FOctree octreeManager = GetWorld()->GetOcTree();
    auto callback = [&PossibleList, &minDistance](const FOctreeElement<UStaticMeshComponent>& e, float hitDistance )-> void {
        PossibleList.Add(e.element);
        //if (minDistance > hitDistance) {
        //    minDistance = hitDistance;
        //    //Possible = e.element;
        //}
    };
    octreeManager.QueryRay(pickOrigin, pickDirection, callback);
    if (PossibleList.Num() > 0) {
        for (UStaticMeshComponent* e : PossibleList) {
            float hitDistance;

            FMatrix inverseWorldMat = FMatrix::CreateInverseMatrixWithSRT(
                e->GetWorldScale(),
                e->GetWorldRotation(),
                e->GetWorldLocation()
            );

            FVector rayOrigin = inverseWorldMat.TransformPosition(pickOrigin);
            FVector rayDirection = FMatrix::TransformVector(pickDirection, inverseWorldMat);

            if ( e->CheckRayIntersection(rayOrigin, rayDirection, hitDistance) > 0 ) {
                if (minDistance > hitDistance) {
                    minDistance = hitDistance;
                    Possible = e;
                }
            }
        }
    }


    if (Possible)
    {
        uint64 CycleDiff = counter.Finish();;
        double elapsedTime = FWindowsPlatformTime::ToMilliseconds(CycleDiff);
        FPSEditorPanel::SetPickElapsedTime(elapsedTime);
        GetWorld()->SetPickedActor(Possible->GetOwner());
    }
}

void AEditorPlayer::AddControlMode()
{
    cMode = static_cast<ControlMode>((cMode + 1) % CM_END);
}

void AEditorPlayer::AddCoordiMode()
{
    cdMode = static_cast<CoordiMode>((cdMode + 1) % CDM_END);
}

// 이게 뭔 함수야.
void AEditorPlayer::ScreenToViewSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& pickPosition)
{
    D3D11_VIEWPORT viewport = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport();
    
    float viewportX = screenX - viewport.TopLeftX;
    float viewportY = screenY - viewport.TopLeftY;

    pickPosition.x = ((2.0f * viewportX / viewport.Width) - 1) / projectionMatrix[0][0];
    pickPosition.y = -((2.0f * viewportY / viewport.Height) - 1) / projectionMatrix[1][1];
    if (GetEngine().GetLevelEditor()->GetActiveViewportClient()->IsOrtho())
    {
        pickPosition.z = 0.0f;  // 오쏘 모드에서는 unproject 시 near plane 위치를 기준
    }
    else
    {
        pickPosition.z = 1.0f;  // 퍼스펙티브 모드: near plane
    }
}

void AEditorPlayer::ScreenToWorldSpace(int screenX, int screenY, const FMatrix& viewMatrix, const FMatrix& projectionMatrix, FVector& OutOrigin, FVector& OutDirection) {

    D3D11_VIEWPORT viewport = GetEngine().GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport();

    float viewportX = screenX - viewport.TopLeftX;
    float viewportY = screenY - viewport.TopLeftY;

    float NDCx = ((2.0f * viewportX / viewport.Width) - 1);
    float NDCy = -((2.0f * viewportY / viewport.Height) - 1);

    FVector4 RayOrigin = FVector4(NDCx, NDCy, 0.f, 1.0f);
    FVector4 RayEnd = FVector4(NDCx, NDCy, 1.f, 1.0f);

    FMatrix InvProjMat = FMatrix::Inverse(projectionMatrix);
    FMatrix InvViewMat = FMatrix::Inverse(viewMatrix);

    RayOrigin = FMatrix::TransformVector(RayOrigin, InvProjMat);
    RayEnd = FMatrix::TransformVector(RayEnd, InvProjMat);
    RayOrigin = RayOrigin / RayOrigin.a;
    RayEnd = RayEnd / RayEnd.a;

    RayOrigin = FMatrix::TransformVector(RayOrigin, InvViewMat);
    RayEnd = FMatrix::TransformVector(RayEnd, InvViewMat);
    RayOrigin = RayOrigin / RayOrigin.a;
    RayEnd = RayEnd / RayEnd.a;

    FVector4 direction = RayEnd - RayOrigin;
    OutOrigin = FVector(RayOrigin.x, RayOrigin.y, RayOrigin.z);
    OutDirection = FVector(direction.x, direction.y, direction.z).Normalize();
}

int AEditorPlayer::RayIntersectsObject(const FVector& pickOrigin, const FVector& pickDirection, USceneComponent* obj, float& hitDistance, int& intersectCount)
{
	//FMatrix scaleMatrix = FMatrix::CreateScale(
	//	obj->GetWorldScale().x,
	//	obj->GetWorldScale().y,
	//	obj->GetWorldScale().z
	//);
	//FMatrix rotationMatrix = FMatrix::CreateRotation(
	//	obj->GetWorldRotation().x,
	//	obj->GetWorldRotation().y,
	//	obj->GetWorldRotation().z
	//);

	//FMatrix translationMatrix = FMatrix::CreateTranslationMatrix(obj->GetWorldLocation());

	//// ���� ��ȯ ���
	//FMatrix worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
	//FMatrix viewMatrix = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetViewMatrix();
    
    bool bIsOrtho = GetEngine().GetLevelEditor()->GetActiveViewportClient()->IsOrtho();
    

    if (bIsOrtho)
    {
        //// 오쏘 모드: ScreenToViewSpace()에서 계산된 pickPosition이 클립/뷰 좌표라고 가정
        //FMatrix inverseView = FMatrix::Inverse(viewMatrix);
        //// pickPosition을 월드 좌표로 변환
        //FVector worldPickPos = inverseView.TransformPosition(pickOrigin);  
        //// 오쏘에서는 픽킹 원점은 unproject된 픽셀의 위치
        //FVector rayOrigin = worldPickPos;
        //// 레이 방향은 카메라의 정면 방향 (평행)
        //FVector orthoRayDir = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewTransformOrthographic.GetForwardVector().Normalize();

        //// 객체의 로컬 좌표계로 변환
        ////FMatrix localMatrix = FMatrix::Inverse(worldMatrix);
        //FMatrix localMatrix = FMatrix::CreateInverseMatrixWithSRT(
        //    obj->GetWorldScale(), 
        //    obj->GetWorldRotation(), 
        //    obj->GetWorldLocation()
        //);
        //FVector localRayOrigin = localMatrix.TransformPosition(rayOrigin);
        //FVector localRayDir = (localMatrix.TransformPosition(rayOrigin + orthoRayDir) - localRayOrigin).Normalize();
        
        intersectCount = obj->CheckRayIntersection(pickOrigin, pickDirection, hitDistance);
        return intersectCount;
    }
    else
    {
        FMatrix inverseWorldMat = FMatrix::CreateInverseMatrixWithSRT(
            obj->GetWorldScale(),
            obj->GetWorldRotation(),
            obj->GetWorldLocation()
        );

        FVector rayOrigin = inverseWorldMat.TransformPosition(pickOrigin);
        FVector rayDirection = FMatrix::TransformVector(pickDirection, inverseWorldMat);

        intersectCount = obj->CheckRayIntersection(rayOrigin, rayDirection, hitDistance);
        
        return intersectCount;
    }
}

void AEditorPlayer::PickedObjControl()
{
    if (GetWorld()->GetSelectedActor() && GetWorld()->GetPickingGizmo())
    {
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);
        int32 deltaX = currentMousePos.x - m_LastMousePos.x;
        int32 deltaY = currentMousePos.y - m_LastMousePos.y;

        // USceneComponent* pObj = GetWorld()->GetPickingObj();
        AActor* PickedActor = GetWorld()->GetSelectedActor();
        UGizmoBaseComponent* Gizmo = static_cast<UGizmoBaseComponent*>(GetWorld()->GetPickingGizmo());
        switch (cMode)
        {
        case CM_TRANSLATION:
            ControlTranslation(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);
            break;
        case CM_SCALE:
            ControlScale(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);

            break;
        case CM_ROTATION:
            ControlRotation(PickedActor->GetRootComponent(), Gizmo, deltaX, deltaY);
            break;
        default:
            break;
        }
        m_LastMousePos = currentMousePos;
    }
}

void AEditorPlayer::ControlRotation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    auto ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();

    FVector CameraForward = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetForwardVector() : ActiveViewport->ViewTransformOrthographic.GetForwardVector();
    FVector CamearRight = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetRightVector() : ActiveViewport->ViewTransformOrthographic.GetRightVector();
    FVector CameraUp = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetUpVector() : ActiveViewport->ViewTransformOrthographic.GetUpVector();

    
    FQuat currentRotation = pObj->GetQuat();

    FQuat rotationDelta;

    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleX)
    {
        float rotationAmount = (CameraUp.z >= 0 ? -1.0f : 1.0f) * deltaY * 0.01f;
        rotationAmount = rotationAmount + (CamearRight.x >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;

        rotationDelta = FQuat(FVector(1.0f, 0.0f, 0.0f), rotationAmount); // ���� X �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleY)
    {
        float rotationAmount = (CamearRight.x >= 0 ? 1.0f : -1.0f) * deltaX * 0.01f;
        rotationAmount = rotationAmount + (CameraUp.z >= 0 ? 1.0f : -1.0f) * deltaY * 0.01f;

        rotationDelta = FQuat(FVector(0.0f, 1.0f, 0.0f), rotationAmount); // ���� Y �� ���� ȸ��
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::CircleZ)
    {
        float rotationAmount = (CameraForward.x <= 0 ? -1.0f : 1.0f) * deltaX * 0.01f;
        rotationDelta = FQuat(FVector(0.0f, 0.0f, 1.0f), rotationAmount); // ���� Z �� ���� ȸ��
    }
    if (cdMode == CDM_LOCAL)
    {
        pObj->SetRotation(currentRotation * rotationDelta);
    }
    else if (cdMode == CDM_WORLD)
    {
        pObj->SetRotation(rotationDelta * currentRotation);
    }
}

void AEditorPlayer::ControlTranslation(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float DeltaX = static_cast<float>(deltaX);
    float DeltaY = static_cast<float>(deltaY);
    auto ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();

    FVector CamearRight = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetRightVector() : ActiveViewport->ViewTransformOrthographic.GetRightVector();
    FVector CameraUp = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetUpVector() : ActiveViewport->ViewTransformOrthographic.GetUpVector();
    
    FVector WorldMoveDirection = (CamearRight * DeltaX + CameraUp * -DeltaY) * 0.1f;
    
    if (cdMode == CDM_LOCAL)
    {
        if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowX)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetForwardVector());
            pObj->AddLocation(pObj->GetForwardVector() * moveAmount);
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowY)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetRightVector());
            pObj->AddLocation(pObj->GetRightVector() * moveAmount);
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
        {
            float moveAmount = WorldMoveDirection.Dot(pObj->GetUpVector());
            pObj->AddLocation(pObj->GetUpVector() * moveAmount);
        }
    }
    else if (cdMode == CDM_WORLD)
    {
        // 월드 좌표계에서 카메라 방향을 고려한 이동
        if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowX)
        {
            // 카메라의 오른쪽 방향을 X축 이동에 사용
            FVector moveDir = CamearRight * DeltaX * 0.05f;
            pObj->AddLocation(FVector(moveDir.x, 0.0f, 0.0f));
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowY)
        {
            // 카메라의 오른쪽 방향을 Y축 이동에 사용
            FVector moveDir = CamearRight * DeltaX * 0.05f;
            pObj->AddLocation(FVector(0.0f, moveDir.y, 0.0f));
        }
        else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
        {
            // 카메라의 위쪽 방향을 Z축 이동에 사용
            FVector moveDir = CameraUp * -DeltaY * 0.05f;
            pObj->AddLocation(FVector(0.0f, 0.0f, moveDir.z));
        }
    }
}

void AEditorPlayer::ControlScale(USceneComponent* pObj, UGizmoBaseComponent* Gizmo, int32 deltaX, int32 deltaY)
{
    float DeltaX = static_cast<float>(deltaX);
    float DeltaY = static_cast<float>(deltaY);
    auto ActiveViewport = GetEngine().GetLevelEditor()->GetActiveViewportClient();

    FVector CamearRight = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetRightVector() : ActiveViewport->ViewTransformOrthographic.GetRightVector();
    FVector CameraUp = ActiveViewport->GetViewportType() == LVT_Perspective ?
        ActiveViewport->ViewTransformPerspective.GetUpVector() : ActiveViewport->ViewTransformOrthographic.GetUpVector();
    
    // 월드 좌표계에서 카메라 방향을 고려한 이동
    if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleX)
    {
        // 카메라의 오른쪽 방향을 X축 이동에 사용
        FVector moveDir = CamearRight * DeltaX * 0.05f;
        pObj->AddScale(FVector(moveDir.x, 0.0f, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleY)
    {
        // 카메라의 오른쪽 방향을 Y축 이동에 사용
        FVector moveDir = CamearRight * DeltaX * 0.05f;
        pObj->AddScale(FVector(0.0f, moveDir.y, 0.0f));
    }
    else if (Gizmo->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
    {
        // 카메라의 위쪽 방향을 Z축 이동에 사용
        FVector moveDir = CameraUp * -DeltaY * 0.05f;
        pObj->AddScale(FVector(0.0f, 0.0f, moveDir.z));
    }
}
