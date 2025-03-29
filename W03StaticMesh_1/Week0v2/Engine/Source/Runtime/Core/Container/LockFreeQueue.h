#pragma once
#include <atomic>
#include <optional>
#include <memory>

// Michael-Scott 락프리 큐 구현 예제
template<typename T>
class TLockFreeQueue
{
private:
    struct Node
    {
        std::optional<T> Value;
        std::atomic<Node*> Next;

        // 더미 노드 생성용 생성자
        Node() : Next(nullptr) {}

        // 값을 가지는 노드 생성
        Node(const T& val) : Value(val), Next(nullptr) {}
        Node(T&& val) : Value(std::move(val)), Next(nullptr) {}
    };

    // Head는 항상 더미 노드를 가리키며, tail은 마지막 노드를 가리킵니다.
    std::atomic<Node*> Head;
    std::atomic<Node*> Tail;

public:
    TLockFreeQueue()
    {
        // 초기 더미 노드를 생성합니다.
        Node* dummy = new Node();
        Head.store(dummy, std::memory_order_relaxed);
        Tail.store(dummy, std::memory_order_relaxed);
    }

    ~TLockFreeQueue()
    {
        // 큐가 비워질 때까지 Dequeue를 호출합니다.
        while (Dequeue())
            ;
        Node* dummy = Head.load(std::memory_order_relaxed);
        delete dummy;
    }

    // Enqueue: 항목을 큐에 추가합니다.
    void Enqueue(const T& item)
    {
        Node* newNode = new Node(item);
        EnqueueNode(newNode);
    }

    void Enqueue(T&& item)
    {
        Node* newNode = new Node(std::move(item));
        EnqueueNode(newNode);
    }

    // Dequeue: 항목을 큐에서 제거하고 반환합니다.
    // 큐가 비어있으면 std::nullopt를 반환합니다.
    std::optional<T> Dequeue()
    {
        while (true)
        {
            Node* oldHead = Head.load(std::memory_order_acquire);
            Node* oldTail = Tail.load(std::memory_order_acquire);
            Node* next = oldHead->Next.load(std::memory_order_acquire);
            
            if (oldHead == Head.load(std::memory_order_acquire))
            {
                if (next == nullptr)
                {
                    // 큐가 비어 있음
                    return std::nullopt;
                }
                if (oldHead == oldTail)
                {
                    // Tail이 뒤처져 있는 경우 업데이트 시도
                    Tail.compare_exchange_weak(oldTail, next, std::memory_order_release);
                }
                else
                {
                    // Head를 다음 노드로 이동
                    if (Head.compare_exchange_weak(oldHead, next, std::memory_order_release))
                    {
                        std::optional<T> result = next->Value;
                        // oldHead는 더미 노드였으므로 메모리 해제
                        delete oldHead;
                        return result;
                    }
                }
            }
        }
    }

    // 큐가 비어있는지 여부 반환
    bool IsEmpty() const
    {
        Node* headNode = Head.load(std::memory_order_acquire);
        Node* next = headNode->Next.load(std::memory_order_acquire);
        return (next == nullptr);
    }

private:
    void EnqueueNode(Node* newNode)
    {
        while (true)
        {
            Node* last = Tail.load(std::memory_order_acquire);
            Node* next = last->Next.load(std::memory_order_acquire);
            if (last == Tail.load(std::memory_order_acquire))
            {
                if (next == nullptr)
                {
                    // 마지막 노드의 Next가 nullptr이면, 여기에 삽입
                    if (last->Next.compare_exchange_weak(next, newNode, std::memory_order_release))
                    {
                        // 삽입 성공 후 Tail을 업데이트 시도
                        Tail.compare_exchange_weak(last, newNode, std::memory_order_release);
                        return;
                    }
                }
                else
                {
                    // 다른 스레드가 이미 새로운 노드를 삽입했으므로 Tail 업데이트 시도
                    Tail.compare_exchange_weak(last, next, std::memory_order_release);
                }
            }
        }
    }
};
