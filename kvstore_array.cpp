#include <vector>
#include "kvstore.h"
#include <string>
#include <cstring>

std::vector<kvs_array_item> array_table;

int kvstore_array_set(std::string& key, std::string& value)
{
    if(array_table.size() > KVS_ARRAY_SIZE)
    {
        return -1;
    }

    for(auto& kv : array_table)
    {
        if(!kv.is_valid)
        {
            kv.key = key;
            kv.value = value;
            kv.is_valid = true;
            return 0;
        }
    }

    array_table.push_back({key, value, 1});
    return 0;
}

std::string kvstore_array_get(std::string& key)
{
    if(key.empty())
    {
        return std::string();
    }
    for(auto& kv : array_table)
    {
        if(kv.is_valid && kv.key == key)
        {
            return kv.value;
        }
    }
    return std::string();
}

int kvstore_array_delete(std::string& key)
{
    if(key.empty())
    {
        return -1;
    }
    for(auto& kv : array_table)
    {
        if(kv.is_valid && kv.key == key)
        {
            kv.is_valid = false;
            return 0;
        }
    }
    return 1;
}

int kvstore_array_modify(std::string& key, std::string& value)
{
    if(key.empty() || value.empty())
    {
        return -1;
    }
    for(auto& kv : array_table)
    {
        if(kv.is_valid && kv.key == key)
        {
            kv.value = value;
            return 0;
        }
    }
    return 1;
}