#pragma once

#include <map>
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstddef>

#include "Pair.h"

/**
 * TMultiMap
 *
 * Unreal Engine의 TMapBase를 기반으로 한 멀티맵 컨테이너로,
 * 하나의 키에 여러 값을 저장할 수 있습니다.
 */
template<typename KeyType, typename ValueType, typename Compare = std::less<KeyType>>
class TMultiMap
{
public:
    using MultimapType = std::multimap<KeyType, ValueType, Compare>;
    using Iterator = typename MultimapType::iterator;
    using ConstIterator = typename MultimapType::const_iterator;
    using PairType = TPair<const KeyType, ValueType>;

    TMultiMap() = default;
    TMultiMap(TMultiMap&&) = default;
    TMultiMap(const TMultiMap&) = default;
    TMultiMap& operator=(TMultiMap&&) = default;
    TMultiMap& operator=(const TMultiMap&) = default;

    // 다른 비교자(Allocator가 아닌) 타입과의 이동 및 복사 생성자
    template<typename OtherCompare>
    TMultiMap(TMultiMap<KeyType, ValueType, OtherCompare>&& Other)
        : Container(std::move(Other.Container))
    {
    }

    template<typename OtherCompare>
    TMultiMap(const TMultiMap<KeyType, ValueType, OtherCompare>& Other)
        : Container(Other.Container)
    {
    }

    // 초기화 리스트를 통한 생성자
    TMultiMap(std::initializer_list<TPair<KeyType, ValueType>> InitList)
    {
        Reserve(InitList.size());
        for (const auto& Element : InitList)
        {
            Add(Element.first, Element.second);
        }
    }

    //////////////////////////////////////////////////
    // Intrusive TOptional<TMultiMap> state (optional) //
    //////////////////////////////////////////////////
    // Unreal의 TMultiMap은 TOptional과의 호환성을 위해 특수 생성자를 제공하지만,
    // 여기서는 단순화를 위해 별도 구현하지 않습니다.

    // Assignment operator from initializer list.
    TMultiMap& operator=(std::initializer_list<TPair<KeyType, ValueType>> InitList)
    {
        Empty();
        Reserve(InitList.size());
        for (const auto& Element : InitList)
        {
            Add(Element.first, Element.second);
        }
        return *this;
    }

    /**
    * Add a key-value association to the map.
    * 항상 새로운 항목을 추가합니다.
    *
    * @param InKey The key to add.
    * @param InValue The value to add.
    * @return A reference to the value added.
    */
    ValueType& Add(const KeyType& InKey, const ValueType& InValue)
    {
        auto iter = Container.insert(std::make_pair(InKey, InValue));
        return iter->second;
    }
    ValueType& Add(KeyType&& InKey, ValueType&& InValue)
    {
        auto iter = Container.emplace(std::move(InKey), std::move(InValue));
        return iter->second;
    }
    
    /**
     * Finds all values associated with the specified key.
     *
     * @param Key The key to find associated values for.
     * @param OutValues Upon return, contains the values associated with the key.
     * @param bMaintainOrder true if the values should be in the same order as stored.
     */
    template<typename Allocator = std::allocator<ValueType>>
    void MultiFind(const KeyType& Key, std::vector<ValueType, Allocator>& OutValues, bool bMaintainOrder = false) const
    {
        auto range = Container.equal_range(Key);
        for (auto it = range.first; it != range.second; ++it)
        {
            OutValues.push_back(it->second);
        }

        if (bMaintainOrder)
        {
            std::reverse(OutValues.begin(), OutValues.end());
        }
    }

    /**
     * Finds all values associated with the specified key (pointers version).
     *
     * @param Key The key to find associated values for.
     * @param OutValues Upon return, contains pointers to the values associated with the key.
     * @param bMaintainOrder true if the values should be in the same order as stored.
     */
    template<typename Allocator = std::allocator<const ValueType*>>
    void MultiFindPointer(const KeyType& Key, std::vector<const ValueType*, Allocator>& OutValues, bool bMaintainOrder = false) const
    {
        auto range = Container.equal_range(Key);
        for (auto it = range.first; it != range.second; ++it)
        {
            OutValues.push_back(&(it->second));
        }

        if (bMaintainOrder)
        {
            std::reverse(OutValues.begin(), OutValues.end());
        }
    }

    template<typename Allocator = std::allocator<ValueType*>>
    void MultiFindPointer(const KeyType& Key, std::vector<ValueType*, Allocator>& OutValues, bool bMaintainOrder = false)
    {
        auto range = Container.equal_range(Key);
        for (auto it = Container.begin(); it != Container.end(); ++it)
        {
            // 직접 반복자를 새로 생성하여 범위 내의 요소만 대상으로 합니다.
            if (it->first == Key)
            {
                OutValues.push_back(&(it->second));
            }
        }

        if (bMaintainOrder)
        {
            std::reverse(OutValues.begin(), OutValues.end());
        }
    }

    /**
     * Add a key-value association to the map. If an identical association already exists,
     * no new association is made and the existing association's value is returned.
     */
    ValueType& AddUnique(const KeyType& InKey, const ValueType& InValue)
    {
        return EmplaceUnique(InKey, InValue);
    }
    ValueType& AddUnique(const KeyType& InKey, ValueType&& InValue)
    {
        return EmplaceUnique(InKey, std::move(InValue));
    }
    ValueType& AddUnique(KeyType&& InKey, const ValueType& InValue)
    {
        return EmplaceUnique(std::move(InKey), InValue);
    }
    ValueType& AddUnique(KeyType&& InKey, ValueType&& InValue)
    {
        return EmplaceUnique(std::move(InKey), std::move(InValue));
    }

    /**
     * Emplace a key-value association to the map.
     * If the same key-value pair exists, return the existing association.
     */
    template <typename InitKeyType, typename InitValueType>
    ValueType& EmplaceUnique(InitKeyType&& InKey, InitValueType&& InValue)
    {
        if (ValueType* Found = FindPair(InKey, InValue))
        {
            return *Found;
        }
        auto iter = Container.emplace(std::forward<InitKeyType>(InKey), std::forward<InitValueType>(InValue));
        return iter->second;
    }

    /**
     * Remove all value associations for a key.
     *
     * @param InKey The key to remove associated values for.
     * @return The number of values that were associated with the key.
     */
    std::size_t Remove(const KeyType& InKey)
    {
        return Container.erase(InKey);
    }

    /**
     * Remove associations between the specified key and value from the map.
     *
     * @param InKey The key of the association to remove.
     * @param InValue The value of the association to remove.
     * @return The number of associations removed.
     */
    std::size_t Remove(const KeyType& InKey, const ValueType& InValue)
    {
        std::size_t NumRemovedPairs = 0;
        auto range = Container.equal_range(InKey);
        for (auto it = range.first; it != range.second; )
        {
            if (it->second == InValue)
            {
                it = Container.erase(it);
                ++NumRemovedPairs;
            }
            else
            {
                ++it;
            }
        }
        return NumRemovedPairs;
    }

    /**
     * Remove the first association between the specified key and value from the map.
     *
     * @param InKey The key of the association to remove.
     * @param InValue The value of the association to remove.
     * @return The number of associations removed (0 or 1).
     */
    std::size_t RemoveSingle(const KeyType& InKey, const ValueType& InValue)
    {
        auto range = Container.equal_range(InKey);
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == InValue)
            {
                Container.erase(it);
                return 1;
            }
        }
        return 0;
    }

    /**
     * Find an association between the specified key and value (const version).
     *
     * @param Key The key to find.
     * @param Value The value to find.
     * @return Pointer to the found value, or nullptr if not found.
     */
    const ValueType* FindPair(const KeyType& Key, const ValueType& Value) const
    {
        return const_cast<TMultiMap*>(this)->FindPair(Key, Value);
    }

    /**
     * Find an association between the specified key and value.
     *
     * @param Key The key to find.
     * @param Value The value to find.
     * @return Pointer to the found value, or nullptr if not found.
     */
    ValueType* FindPair(const KeyType& Key, const ValueType& Value)
    {
        auto range = Container.equal_range(Key);
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == Value)
            {
                return &(it->second);
            }
        }
        return nullptr;
    }

    /**
     * Returns the number of values associated with the specified key.
     */
    std::size_t Num(const KeyType& Key) const
    {
        return Container.count(Key);
    }

    /**
     * Returns the total number of associations in the map.
     */
    std::size_t Num() const
    {
        return Container.size();
    }

    /**
     * Move all items from another map into our map (if any keys are in both,
     * the value from the other map wins) and empty the other map.
     */
    template<typename OtherCompare>
    void Append(TMultiMap<KeyType, ValueType, OtherCompare>&& OtherMultiMap)
    {
        Container.insert(std::make_move_iterator(OtherMultiMap.Container.begin()),
                         std::make_move_iterator(OtherMultiMap.Container.end()));
        OtherMultiMap.Container.clear();
    }

    /**
     * Add all items from another map to our map (if any keys are in both,
     * the value from the other map wins).
     */
    template<typename OtherCompare>
    void Append(const TMultiMap<KeyType, ValueType, OtherCompare>& OtherMultiMap)
    {
        Container.insert(OtherMultiMap.Container.begin(), OtherMultiMap.Container.end());
    }

    // Iterator 지원
    Iterator begin() { return Container.begin(); }
    Iterator end() { return Container.end(); }
    ConstIterator begin() const { return Container.begin(); }
    ConstIterator end() const { return Container.end(); }

    /**
     * Reserve space for at least Count elements.
     * std::multimap은 reserve()를 지원하지 않으므로, 해당 함수는 no-op입니다.
     */
    void Reserve(std::size_t /*Count*/) {}

    /**
     * Remove all elements from the map.
     */
    void Empty()
    {
        Container.clear();
    }

    /**
     * Returns a pair of iterators delimiting the range of elements matching the specified key.
     *
     * @param Key The key to find.
     * @return A pair of iterators {first, second} corresponding to the beginning and end of the range.
     */
    std::pair<Iterator, Iterator> EqualRange(const KeyType& Key)
    {
        return Container.equal_range(Key);
    }

    /**
     * Const version of EqualRange.
     */
    std::pair<ConstIterator, ConstIterator> EqualRange(const KeyType& Key) const
    {
        return Container.equal_range(Key);
    }


private:
    MultimapType Container;
};

