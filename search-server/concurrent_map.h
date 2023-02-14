#pragma once

#include <vector>
#include <mutex>
#include <string>


using namespace std::string_literals;
using namespace std;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
        const lock_guard<mutex> guard;
        Value& ref_to_value;

    };

    explicit ConcurrentMap(size_t bucket_count) : data_(bucket_count) {


    }

    void erase(const Key& key) {
        uint64_t index =key%data_.size();
        lock_guard<mutex> guard(data_[index].mutex_val);
        data_[index].dict.erase(key);

    }

    size_t size() {
        return data_.size();

    }

    Access operator[](const Key& key) {
        uint64_t index =key % data_.size();
        return { lock_guard(data_[index].mutex_val), data_[index].dict[key] };
    }
    std::map<Key, Value> BuildOrdinaryMap() {
        uint64_t bucket_num = data_.size();
        map<Key, Value> result;



        for (uint64_t i = 0; i < bucket_num; i++) {
            lock_guard<mutex> guard(data_[i].mutex_val);
            result.insert(data_[i].dict.begin(), data_[i].dict.end());
        }
        return result;

    }

    struct Basket {
        mutex mutex_val;
        map<Key, Value> dict;

    };

private:


    vector<Basket> data_;

};