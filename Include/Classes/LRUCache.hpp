#pragma once
#include <unordered_map>
#include <list>
#include <utility>
#include <limits>
#include <optional>

using std::list, std::pair, std::unordered_map, std::optional;

template <typename KeyType, typename DataType>
struct LRUCache {
    public:
        using PairList = list<pair<KeyType, DataType>>;
        using PairListIterator = typename PairList::iterator;
        using Cache = unordered_map<KeyType, PairListIterator>;
        using CacheIterator = typename Cache::iterator;
        
        LRUCache(size_t CacheSize) : CacheSize(CacheSize) {}
        LRUCache() = default;
        
        optional<DataType> Get(KeyType Key) {
            CacheIterator CacheMapIterator = CacheMap.find(Key);
            if (CacheMapIterator == CacheMap.end()) return std::nullopt;
            MakeMostRecent(CacheMapIterator);
            return CacheMapIterator->second->second;
        }
        
        void Put(KeyType Key, DataType Data) {
            CacheIterator CacheMapIterator = CacheMap.find(Key);
            
            if (CacheMapIterator != CacheMap.end()) {
                CacheMapIterator->second->second = Data;
                MakeMostRecent(CacheMapIterator);
                return;
            }
            
            if (LRUList.size() == CacheSize) {
                KeyType OldestKey = LRUList.back().first;
                CacheMap.erase(OldestKey);
                LRUList.pop_back();
            }
            
            LRUList.emplace_front(Key, Data);
            CacheMap[Key] = LRUList.begin();
        }
        
    private:
        size_t CacheSize = std::numeric_limits<size_t>::max();
        PairList LRUList;
        Cache CacheMap;
        
        void MakeMostRecent(CacheIterator CacheMapIterator) {
            LRUList.splice(LRUList.begin(), LRUList, CacheMapIterator->second);
        }
};