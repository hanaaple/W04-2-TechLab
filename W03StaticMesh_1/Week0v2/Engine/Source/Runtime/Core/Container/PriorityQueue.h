#pragma once

#include <queue>
#include <vector>
#include "HAL/PlatformType.h"

template<typename T, typename Comparator = std::less<T>>
class TPriorityQueue
{
public:
    TPriorityQueue() = default;

    // 항목 추가
    FORCEINLINE void Push(const T& Element)
    {
        Queue.push(Element);
    }

    // 최상위 항목 제거
    FORCEINLINE void Pop()
    {
        Queue.pop();
    }

    // 최상위 항목 참조
    FORCEINLINE const T& Top() const
    {
        return Queue.top();
    }

    // 큐가 비어있는지 여부
    FORCEINLINE bool IsEmpty() const
    {
        return Queue.empty();
    }

    // 큐 내 항목의 수
    FORCEINLINE int32 Num() const
    {
        return static_cast<int32>(Queue.size());
    }

private:
    std::priority_queue<T, std::vector<T>, Comparator> Queue;
};
