#pragma once
#include <vector>
#include <functional>
#include <cassert>
#include <algorithm>

#include "Define.h"


// Octree에 저장할 요소. 각 요소는 자신의 BoundingBox와 고유 ID를 가지고 있습니다.
struct FOctreeElement
{
    FBoundingBox Bounds;
    uint32_t Id; // 예: UUID

    FOctreeElement(const FBoundingBox& InBounds, uint32_t InId)
        : Bounds(InBounds), Id(InId)
    {}
};

// Octree 노드
template<typename T>
struct FOctreeNode  : public std::enable_shared_from_this<FOctreeNode<T>>
{
    FBoundingBox Bounds;                // 현재 노드의 영역
    TArray<T> Elements;            // 이 노드에 속하는 요소들 (leaf인 경우)
    std::shared_ptr<FOctreeNode> Children[8];            // 8개 자식 노드 (leaf가 아니라면 사용)
    bool IsLeaf;                        // leaf 여부

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
    explicit FOctree(const FBoundingBox& InBounds, size_t InMaxElementsPerLeaf = 8, int InMaxDepth = 8)
        : Root(std::make_shared<FOctreeNode<T>>(InBounds)), MaxElementsPerLeaf(InMaxElementsPerLeaf), MaxDepth(InMaxDepth)
    {}

    ~FOctree() { Root.reset(); }

    // 요소 삽입 함수: element와 해당 요소의 BoundingBox를 입력받습니다.
    void Insert(const T &element, const FBoundingBox& ElementBounds)
    {
        InsertRecursive(Root, element, ElementBounds, 0);
    }

    // 쿼리 함수: 주어진 영역과 교차하는 요소들에 대해 callback 함수를 호출합니다.
    void Query(const FBoundingBox &QueryBounds, std::function<void(const T&)> Callback) const
    {
        QueryRecursive(Root, QueryBounds, Callback);
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

    // 재귀적으로 노드를 탐색하여, TargetUUID를 가진 요소가 포함된 경로(leaf부터 루트까지)를 찾습니다.
    // 찾으면 true를 반환하고, OutPath에 해당 노드의 BoundingBox를 push_back 합니다.
    template<typename T>
    bool FindPathToUUID(const  std::shared_ptr<FOctreeNode<T>> Node, uint32 TargetUUID, TArray<FBoundingBox>& OutPath) const
    {
        if (!Node)
            return false;

        // Leaf인 경우: 요소 배열에서 해당 UUID를 검색
        if (Node->IsLeaf)
        {
            for (const T &elem : Node->Elements)
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
    void InsertRecursive(std::shared_ptr<FOctreeNode<T>> Node, const T &element, const FBoundingBox &ElementBounds, int Depth)
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
            Subdivide(Node);
            // 기존 요소들을 재삽입합니다.
            for (const T &existingElement : Node->Elements)
            {
                // 기존 요소의 바운딩 박스는 existingElement.Bounds로 가정
                int ChildIndex = GetChildIndex(Node->Bounds, existingElement.Bounds);
                if (ChildIndex >= 0)
                {
                    InsertRecursive(Node->Children[ChildIndex], existingElement, existingElement.Bounds, Depth+1);
                }
                else
                {
                    // 자식에 완전히 포함되지 않으면 그대로 현재 노드에 남깁니다.
                    // (실제 구현에서는 별도로 처리)
                }
            }
            Node->Elements.Empty();
            Node->IsLeaf = false;
        }

        // 새 요소를 삽입할 자식 노드 결정
        int ChildIndex = GetChildIndex(Node->Bounds, ElementBounds);
        if (ChildIndex >= 0)
        {
            InsertRecursive(Node->Children[ChildIndex], element, ElementBounds, Depth+1);
        }
        else
        {
            // 자식에 완전히 포함되지 않으면 현재 노드에 추가
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
        // 간단화: 요소의 중심을 기준으로 결정합니다.
        FVector elementCenter = ElementBox.GetCenter();
        FVector parentCenter = ParentBox.GetCenter();
        int index = 0;
        if (elementCenter.x >= parentCenter.x) index |= 1;
        if (elementCenter.y >= parentCenter.y) index |= 2;
        if (elementCenter.z >= parentCenter.z) index |= 4;
        // (실제 구현에서는 요소가 자식 영역에 완전히 포함되는지 여부를 검사해야 합니다.)
        return index;
    }

    // 재귀 쿼리 함수: 현재 노드의 영역이 QueryBounds와 교차하면 요소들을 Callback에 전달합니다.
    void QueryRecursive(const std::shared_ptr<FOctreeNode<T>> Node, const FBoundingBox &QueryBounds, std::function<void(const T&)> Callback) const
    {
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
                    QueryRecursive(Node->Children[i], QueryBounds, Callback);
        }
    }
};
