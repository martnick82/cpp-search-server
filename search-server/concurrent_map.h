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
            : guard(m), ref_to_value(map_value[key]) {}
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count)
        : container_(bucket_count) {}


    Access operator[](const Key& key)
    {
        uint64_t index = static_cast<uint64_t> (key) % container_.size();
        return Access(ref(container_[index].data), key, ref(container_[index].protection));
    }

    std::map<Key, Value> BuildOrdinaryMap();

private:
 //ЗАМЕЧАНИЕ: чтобы столько контейнеров не создавать, порекомендую словарь и мьютексы объединить в структуру, 
 // тогда у вас будет один контейнер со всеми данными.
 // Пояснение:Не стал так делать, боясь запутаться в задании и структуре данных. А так получается лаконичнее
//    
    struct Bucket
    {
        std::map<Key, Value> data;
        std::mutex protection;
    };
    std::vector<Bucket> container_;  
//РЕКОМЕНДАЦИЯ: а количество бакетов можно узнать по методу size()
// Пояснение: В самом начале задания, при разработке контейнера на автомате добавил поле size_, 
 //   не удалял его, думая, что возможно в курсе будет развитие контейнера. 
 // Чтобы получить количество элементов, нужно было бы проитерироваться по всем бакетам и суммировать их сайзы,
//

};

template<typename Key, typename Value>
std::map<Key, Value> ConcurrentMap<Key, Value>::BuildOrdinaryMap()
{
    std::map<Key, Value> result;
    for (size_t index = 0; index < container_.size(); ++index)
    {
        std::lock_guard guard(container_[index].protection);
        result.merge(container_[index].data);

    }
    return result;
}