#pragma once
#include <vector>
#include <functional>
#include <cassert>
#include <algorithm>

#include "Define.h"
#include "Renderer/Frustum.h"

class FFrustum;

// Octree에 저장할 요소. 각 요소는 자신의 BoundingBox와 고유 ID를 가지고 있습니다.
template <typename T>
struct FOctreeElement
{
    FBoundingBox Bounds;
    uint32 Id; // 예: UUID
    T* element;


    FOctreeElement(const FBoundingBox& InBounds, uint32 InId)
        : Bounds(InBounds), Id(InId)
    {}
};

// Octree 노드
template<typename T>
struct FOctreeNode  : public std::enable_shared_from_this<FOctreeNode<T>>
{
    FBoundingBox Bounds;                // 현재 노드의 영역
    TArray<FOctreeElement<T>> Elements;            // 이 노드에 속하는 요소들 (leaf인 경우)
    std::shared_ptr<FOctreeNode<T>> Children[8];            // 8개 자식 노드 (leaf가 아니라면 사용)
    bool IsLeaf;                        // leaf 여부
    int ChildrenCullFlags;
    FOctreeNode(const FBoundingBox& InBounds)
        : Bounds(InBounds), IsLeaf(true)
    {
        for (int i = 0; i < 8; ++i)
            Children[i] = nullptr;
    }

    ~FOctreeNode()
    {
        for (int i = 0; i < 8; ++i)
            if (Children[i])
                Children[i].reset();
    }
};

// 간단한 Octree 클래스
template<typename T>
class FOctree
{
public:
    // 생성자: 루트 영역과 노드 당 최대 요소 수, 최대 깊이 설정
    explicit FOctree(const FBoundingBox& InBounds, const size_t InMaxElementsPerLeaf = 8, const int InMaxDepth = 8, const float InLooseFactor = 1.2f)
        : Root(std::make_shared<FOctreeNode<T>>(InBounds)), MaxElementsPerLeaf(InMaxElementsPerLeaf), MaxDepth(InMaxDepth), LooseFactor(InLooseFactor)
    {}

    ~FOctree() { Root.reset(); }

    // 요소 삽입 함수: element와 해당 요소의 BoundingBox를 입력받습니다.
    void Insert(const FOctreeElement<T> &element, const FBoundingBox& ElementBounds)
    {
        InsertRecursive(Root, element, ElementBounds, 0);
    }

    // 쿼리 함수: 주어진 영역과 교차하는 요소들에 대해 callback 함수를 호출합니다.
    template<typename Fn>
    void Query(const FBoundingBox &QueryBounds, Fn Callback) const
    {
        QueryCollisionBox(Root, QueryBounds, Callback);
    }

    void QueryRay(
        const FVector& origin, 
        const FVector &direction, 
        std::function<void(const FOctreeElement<T>&, float)> Callback
    ) const {
        QueryCollisionRay(Root, origin, direction, Callback);
    }

    void FrustumCull(const FFrustum& frustum) const
    {
        FrustumCullRecursive(Root, frustum);
    }

    void OcclusionCull(const FVector& cameraPos) const {
        QueryOcclusion(Root, cameraPos);
    }

    template<typename Fn>
    void PrepareCull(Fn Callback) const {
        PrepareCullRecursive(Root, Callback);
    }

    template<typename Fn>
    void ExecuteCallbackForVisible(Fn Callback) const {
        ExecuteCallbackForVisibleRecursive(Root, Callback);
    }

    // Octree 클래스 내에 public 인터페이스로 추가
    TArray<FBoundingBox> GetAncestorBoundingBoxes(uint32 TargetUUID) const
    {
        TArray<FBoundingBox> Path;
        // 재귀 함수로 경로를 찾습니다. 경로는 leaf부터 루트까지 저장됩니다.
        if (FindPathToUUID(Root, TargetUUID, Path))
        {
            // leaf부터 루트까지 순서로 저장되었으므로, 이를 뒤집어 루트부터 leaf 순으로 만듭니다.
            std::ranges::reverse(Path);
        }
        return Path;
    }

private:
    std::shared_ptr<FOctreeNode<T>> Root;
    size_t MaxElementsPerLeaf;
    int MaxDepth;
    float LooseFactor; // 자식 노드의 BoundingBox를 확장하는 계수 (예: 1.1)

    // 재귀적으로 노드를 탐색하여, TargetUUID를 가진 요소가 포함된 경로(leaf부터 루트까지)를 찾습니다.
    // 찾으면 true를 반환하고, OutPath에 해당 노드의 BoundingBox를 push_back 합니다.
    bool FindPathToUUID(const  std::shared_ptr<FOctreeNode<T>> Node, uint32 TargetUUID, TArray<FBoundingBox>& OutPath) const
    {
        if (!Node)
            return false;

        // Leaf인 경우: 요소 배열에서 해당 UUID를 검색
        if (Node->IsLeaf)
        {
            for (const FOctreeElement<T> &elem : Node->Elements)
            {
                if (elem.Id == TargetUUID)
                {
                    OutPath.Add(Node->Bounds);
                    return true;
                }
            }
            return false;
        }
    
        // 내부 노드인 경우: 모든 자식 노드를 순회
        for (int i = 0; i < 8; ++i)
        {
            if (Node->Children[i])
            {
                if (FindPathToUUID(Node->Children[i], TargetUUID, OutPath))
                {
                    // 자식에서 찾았다면 현재 노드의 BoundingBox도 경로에 추가
                    OutPath.Add(Node->Bounds);
                    return true;
                }
            }
        }
        return false;
    }

    // 재귀적으로 요소를 삽입하는 함수
    void InsertRecursive(std::shared_ptr<FOctreeNode<T>> Node, const FOctreeElement<T> &element, const FBoundingBox &ElementBounds, int Depth)
    {
        // 만약 현재 노드가 leaf이고, 요소 수가 충분하거나 최대 깊이에 도달했다면 현재 노드에 추가합니다.
        if (Node->IsLeaf && (Node->Elements.Num() < MaxElementsPerLeaf || Depth >= MaxDepth))
        {
            Node->Elements.Add(element);
            return;
        }

        // 만약 leaf이면서 분할이 필요한 경우, 자식 노드로 분할합니다.
        if (Node->IsLeaf)
        {
            // 현재 노드를 8개의 자식 노드로 분할합니다.
            Subdivide(Node);
            TArray<FOctreeElement<T>> RemainingElements;
            // 기존 요소들을 재삽입합니다.
            for (const FOctreeElement<T> &existingElement : Node->Elements)
            {
                // 기존 요소의 바운딩 박스는 existingElement.Bounds로 가정
                int ChildIndex = GetChildIndex(Node->Bounds, existingElement.Bounds);
                if (ChildIndex >= 0)
                {
                    InsertRecursive(Node->Children[ChildIndex], existingElement, existingElement.Bounds, Depth + 1);
                }
                else
                {
                    // 자식에 완전히 포함되지 않으면 현재 노드에 그대로 남깁니다.
                    RemainingElements.Add(existingElement);
                }
            }
            // 재삽입 후, 자식으로 이동하지 못한 요소들을 현재 노드에 보관합니다.
            Node->Elements = RemainingElements;
            Node->IsLeaf = false;
        }

        // 새 요소를 삽입할 자식 노드 결정
        int ChildIndex = GetChildIndex(Node->Bounds, ElementBounds);
        if (ChildIndex >= 0)
        {
            InsertRecursive(Node->Children[ChildIndex], element, ElementBounds, Depth + 1);
        }
        else
        {
            // 새 요소가 자식 영역에 완전히 포함되지 않으면, 현재 노드에 추가합니다.
            Node->Elements.Add(element);
        }
    }

    // 노드 분할: 현재 노드의 영역을 8개 자식 영역으로 분할하고, 자식 노드들을 생성합니다.
    void Subdivide(const std::shared_ptr<FOctreeNode<T>> Node)
    {
        FVector center = Node->Bounds.GetCenter();
        FVector extent = Node->Bounds.GetExtent() * 0.5f; // 자식의 extent는 부모의 절반

        for (int i = 0; i < 8; ++i)
        {
            FVector childMin = Node->Bounds.min;
            FVector childMax = Node->Bounds.max;

            // 각 축에 대해, i의 비트값에 따라 자식의 min/max를 결정
            childMin.x = (i & 1) ? center.x : Node->Bounds.min.x;
            childMax.x = (i & 1) ? Node->Bounds.max.x : center.x;
            childMin.y = (i & 2) ? center.y : Node->Bounds.min.y;
            childMax.y = (i & 2) ? Node->Bounds.max.y : center.y;
            childMin.z = (i & 4) ? center.z : Node->Bounds.min.z;
            childMax.z = (i & 4) ? Node->Bounds.max.z : center.z;

            FBoundingBox childBox(childMin, childMax);
            Node->Children[i] = std::make_shared<FOctreeNode<T>>(childBox);
        }
    }

    // 부모 노드의 BoundingBox와 요소의 BoundingBox로부터, 요소가 완전히 포함되는 자식 인덱스를 반환합니다.
    // 만약 완전히 포함되지 않으면 -1을 반환합니다.
    int GetChildIndex(const FBoundingBox &ParentBox, const FBoundingBox &ElementBox)
    {
        // FVector parentCenter = ParentBox.GetCenter();
        // // 우선, 요소의 중심을 기준으로 자식 인덱스 후보를 결정합니다.
        // FVector elementCenter = ElementBox.GetCenter();
        // int index = 0;
        // if (elementCenter.x >= parentCenter.x) index |= 1;
        // if (elementCenter.y >= parentCenter.y) index |= 2;
        // if (elementCenter.z >= parentCenter.z) index |= 4;
        //
        // // 후보 인덱스에 해당하는 자식 노드의 BoundingBox를 계산합니다.
        // FVector childMin, childMax;
        // childMin.x = (index & 1) ? parentCenter.x : ParentBox.min.x;
        // childMax.x = (index & 1) ? ParentBox.max.x : parentCenter.x;
        // childMin.y = (index & 2) ? parentCenter.y : ParentBox.min.y;
        // childMax.y = (index & 2) ? ParentBox.max.y : parentCenter.y;
        // childMin.z = (index & 4) ? parentCenter.z : ParentBox.min.z;
        // childMax.z = (index & 4) ? ParentBox.max.z : parentCenter.z;
        //
        // // 자식의 중심과 절반 크기 계산
        // const FVector childCenter = (childMin + childMax) * 0.5f;
        // const FVector childHalfExtents = (childMax - childMin) * 0.5f;
        // // LooseFactor를 적용한 확장된 절반 크기
        // const FVector looseHalfExtents = childHalfExtents * LooseFactor;
        // // 확장된(Loose) 자식의 BoundingBox 계산
        // const FVector looseMin = childCenter - looseHalfExtents;
        // const FVector looseMax = childCenter + looseHalfExtents;
        //
        // // 요소의 BoundingBox가 확장된 자식 영역에 완전히 포함되는지 검사합니다.
        // if (ElementBox.min.x >= looseMin.x && ElementBox.max.x <= looseMax.x &&
        //     ElementBox.min.y >= looseMin.y && ElementBox.max.y <= looseMax.y &&
        //     ElementBox.min.z >= looseMin.z && ElementBox.max.z <= looseMax.z)
        // {
        //     return index;
        // }
        // else
        // {
        //     return -1;
        // }

        FVector parentCenter = ParentBox.GetCenter();
        FVector elementCenter = ElementBox.GetCenter();
        int index = 0;
        if (elementCenter.x >= parentCenter.x) index |= 1;
        if (elementCenter.y >= parentCenter.y) index |= 2;
        if (elementCenter.z >= parentCenter.z) index |= 4;
        return index;
    }

    template<typename Fn>
    void PrepareCullRecursive(const std::shared_ptr<FOctreeNode<T>>& Node, Fn Callback) const {
        Node->ChildrenCullFlags = 0;
        if ( Node->IsLeaf ) {
            for ( const auto& elem : Node->Elements ) {
                Callback(elem);
            }
        }
        for (int i = 0; i < 8; ++i) {
            if ( Node->Children[i] )
                PrepareCullRecursive(Node->Children[i], Callback);
        }
    }

    template<typename Fn>
    void ExecuteCallbackForVisibleRecursive(const std::shared_ptr<FOctreeNode<T>>& Node, Fn Callback) const {
        if ( Node->IsLeaf ) {
            int i = 0;
            for ( const auto& elem : Node->Elements ) {
                if ( !(Node->ChildrenCullFlags & 1 << i) ) {
                    Callback(elem);
                }
                ++i;
            }
        } else {
            for ( int i = 0; i < 8; ++i ) {
                if ( Node->Children[i] && !(Node->ChildrenCullFlags & 1 << i) ) {
                    ExecuteCallbackForVisibleRecursive(Node->Children[i], Callback);
                }
            }
        }
    }

    void FrustumCullRecursive(const std::shared_ptr<FOctreeNode<T>>& Node, const FFrustum &frustum) const
    {
        //if (!frustum.IsBoxVisible(Node->Bounds))
        //    return;
    
        if (Node->IsLeaf)
        {
            int i = 0;
            for ( const auto& elem: Node->Elements )
            {
                if (!frustum.IsBoxVisible(elem.Bounds)) {
                    Node->ChildrenCullFlags |= 1 << i;
                }
                ++i;
            }
            return;
        }
    
        for (int i = 0; i < 8; ++i)
        {
            if (Node->Children[i]) {
                if ( frustum.IsBoxVisible(Node->Children[i]->Bounds) ) {
                    FrustumCullRecursive(Node->Children[i], frustum);
                } else {
                    Node->ChildrenCullFlags |= 1 << i;
                }
                
            }
            
        }
    }

    // 재귀 쿼리 함수: 현재 노드의 영역이 QueryBounds와 교차하면 요소들을 Callback에 전달합니다.
    void QueryCollisionBox(const std::shared_ptr<FOctreeNode<T>> Node, const FBoundingBox &QueryBounds, std::function<void(const T&)> Callback) const
    {
        // TODO: Bounds.IntersectWithAABB 만들기
        if (!Node->Bounds.Intersects(QueryBounds))
            return;

        // 현재 노드의 요소 검사
        for (const T &elem : Node->Elements)
        {
            if (elem.Bounds.Intersects(QueryBounds))
                Callback(elem);
        }
        // 자식 노드 탐색
        if (!Node->IsLeaf)
        {
            for (int i = 0; i < 8; ++i)
                if (Node->Children[i])
                    QueryCollisionBox(Node->Children[i], QueryBounds, Callback);
        }
    }

    // 재귀 쿼리 함수: 현재 노드의 영역이 Ray와 교차하면 요소들을 Callback에 전달합니다.
    void QueryCollisionRay(
        const std::shared_ptr<FOctreeNode<T>> Node, 
        const FVector& origin, 
        const FVector& direction, 
        std::function<void(const FOctreeElement<T>&, float)> Callback
    ) const {
        float hitDistance = FLT_MAX;
        if ( !Node->Bounds.IntersectRay(origin, direction, hitDistance) )
            return;

        // 현재 노드의 요소 검사
        for ( const FOctreeElement<T>& elem : Node->Elements ) {
            if ( elem.Bounds.IntersectRay(origin, direction, hitDistance) ) {
                Callback(elem, hitDistance);
            }
            
        }
        // 자식 노드 탐색
        if ( !Node->IsLeaf ) {
            for ( int i = 0; i < 8; ++i ) {
                if ( Node->ChildrenCullFlags & 1 << i )
                    continue;
                if ( Node->Children[i] )
                    QueryCollisionRay(Node->Children[i], origin, direction, Callback);
            }
        }
    }

    // Occlusion되지 않으면 ChildrenCullFlag 활성화
    void QueryOcclusion(
        const std::shared_ptr<FOctreeNode<T>> Node,
        const FVector& cameraPos
    ) const {

        if ( Node->IsLeaf )
            return;

        // InOccludee가 나머지 7개의 BoundingBox에 가려지면 true.
        auto IsOcclusions = [&cameraPos](
            std::shared_ptr<FOctreeNode<T>> InNodes[], 
            int InOccludeeIdx
            )->bool {
            constexpr float occluderScale = 0.8f;
            constexpr float occludeeScale = 1.2f;
            const FBoundingBox scaled = InNodes[InOccludeeIdx]->Bounds.Expanded(occluderScale);
            const FVector& max = scaled.max;
            const FVector& min = scaled.min;
            const FVector occludeePoints[8] = {
                FVector(max.x, max.y, max.z),
                FVector(min.x, max.y, max.z),
                FVector(max.x, min.y, max.z),
                FVector(min.x, min.y, max.z),
                FVector(max.x, max.y, min.z),
                FVector(min.x, max.y, min.z),
                FVector(max.x, min.y, min.z),
                FVector(min.x, min.y, min.z),
            };

            // InOccludee의 AABB 8개 점과 InOccludee를 제외한 7개의 BoundingBox가 intersect하는지 테스트.
            //int intersectFlags = 0;
            //for ( int i = 0; i < 8; ++i ) {
            //    FVector dir = (cameraPos - occludeePoints[i]).Normalize();
            //    float hitDistance;
            //    for ( int j = 0; j < 8; ++j ) {
            //        if ( j == InOccludeeIdx )
            //            continue;
            //        FBoundingBox bb = InNodes[j]->Bounds.Expanded(occluderScale);
            //        if ( bb.IntersectLine(occludeePoints[i], cameraPos) ) {
            //            intersectFlags |= 1 << i;
            //            break;
            //        }
            //    }
            //}
            int intersectFlags = 0;
            for ( int j = 0; j < 8; ++j ) {
                if ( j == InOccludeeIdx )
                    continue;
                FBoundingBox bb = InNodes[j]->Bounds.Expanded(occluderScale);
                intersectFlags |= bb.IntersectLineMulti(occludeePoints, cameraPos);

            }
            return (intersectFlags == 0xff);
        };

        // Node 내에 카메라가 있으면 자식들 다 그려지는걸로 판정
        const FVector& max = Node->Bounds.max;
        const FVector& min = Node->Bounds.min;
        bool bIsCameraConatined = false;

        if ( min.x < cameraPos.x && cameraPos.x < max.x &&
            min.y < cameraPos.y && cameraPos.y < max.y &&
            min.z < cameraPos.z && cameraPos.z < max.z
            ) {
            bIsCameraConatined = true;
        }
        for ( int i = 0; i < 8; ++i ) {
            if ( Node->ChildrenCullFlags & 1 << i )
                continue;
            if ( bIsCameraConatined ) {
                QueryOcclusion(Node->Children[i], cameraPos);
            } else if ( IsOcclusions(Node->Children, i) == false ) {
                QueryOcclusion(Node->Children[i], cameraPos);
            } else {
                Node->ChildrenCullFlags |= 1 << i;
            }
        }
    }
};
