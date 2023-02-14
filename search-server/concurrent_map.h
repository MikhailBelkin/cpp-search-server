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


        // Access( const lock_guard<mutex>& mut, Value& val) :guard(mut), ref_to_value(val)  {
        // }



    };

    explicit ConcurrentMap(size_t bucket_count) : data_(bucket_count) {


    }

    void erase(const Key& key) {
        uint64_t index =/*static_cast<uint64_t>*/(key) % data_.size();
        lock_guard<mutex> guard(data_[index].mutex_val);
        data_[index].dict.erase(key);

    }

    size_t size() {
        return data_.size();

    }

    Access operator[](const Key& key) {
        uint64_t index =/*static_cast<uint64_t>*/(key) % data_.size();
        //{
          //  lock_guard guard(data_[key % bucket_num].mutex_val);
            //if (data_[key % bucket_num].dict.count(key) == 0) {
            //    data_[key % bucket_num].dict.emplace(key, Value());
            //}
        //}
        //return Access(guard, data_[key%bucket_num].dict[key]);
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
    //size_t bucket_num;



};