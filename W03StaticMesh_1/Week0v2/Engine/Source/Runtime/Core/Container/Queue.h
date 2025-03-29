#pragma once
#include <queue>
#include <mutex>
#include <optional>

// TQueue 스타일의 인터페이스를 제공하는 std::queue 래퍼 클래스
template<typename T>
class TQueue
{
public:
    // 요소를 큐에 추가합니다.
    void Enqueue(const T& item)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        PrivateQueue.push(item);
    }

    // rvalue 요소를 큐에 추가합니다.
    void Enqueue(T&& item)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        PrivateQueue.push(std::move(item));
    }

    // 큐에서 요소를 제거하고 반환합니다.
    // 큐가 비어 있다면 std::nullopt를 반환합니다.
    std::optional<T> Dequeue()
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        if (PrivateQueue.empty())
        {
            return std::nullopt;
        }
        T FrontItem = std::move(PrivateQueue.front());
        PrivateQueue.pop();
        return FrontItem;
    }

    // 큐가 비어있는지 확인합니다.
    bool IsEmpty() const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return PrivateQueue.empty();
    }

    // 큐에 남아있는 모든 요소를 제거합니다.
    void Empty()
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        while (!PrivateQueue.empty())
        {
            PrivateQueue.pop();
        }
    }

private:
    mutable std::mutex Mutex;
    std::queue<T> PrivateQueue;
};
