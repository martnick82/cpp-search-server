#pragma once
#include <map>
#include <vector>
#include <mutex>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap
{
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access
    {
        Access(std::map<Key, Value>& map_value, int64_t key, std::mutex& m)
            : guard(m), ref_to_value(map_value[key]) {    }
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
        : container_(bucket_count), mutex_vector_(bucket_count), size_(0) {}


    Access operator[](const Key& key)
    {
        uint64_t index = static_cast<uint64_t> (key) % container_.size();
        if (container_[index].count(key) == 0)
        {
            //container_[index].emplace(std::pair(key, Value()));
            ++size_;
        }
        return Access(ref(container_[index]), key, ref(mutex_vector_[index]));
    }

    std::map<Key, Value> BuildOrdinaryMap();

private:
    std::vector<std::map<Key, Value>> container_;
    std::vector<std::mutex> mutex_vector_;
    size_t size_ = 0;
};

template<typename Key, typename Value>
std::map<Key, Value> ConcurrentMap<Key, Value>::BuildOrdinaryMap()
{
    std::map<Key, Value> result;
    for (size_t index = 0; index < container_.size(); ++index)
    {
        std::lock_guard guard(mutex_vector_[index]);
        result.merge(container_[index]);

    }
    return result;
}