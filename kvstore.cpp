#include <kvstore.h>
#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <cstring>

#define KV_STORE_MAX_TOKENS 128

std::vector<std::string> commands{
    "SET",
    "GET",
    "DEL",
    "MOD"};

enum
{
    KVS_CMD_START = 0,
    KVS_CMD_SET = KVS_CMD_START,
    KVS_CMD_GET,
    KVS_CMD_DEL,
    KVS_CMD_MOD,
};

void *kvstore_malloc(size_t size)
{
    return malloc(size);
}

void kvstore_free(void *ptr)
{
    free(ptr);
}

int kvstore_split_token(std::string msg, std::vector<std::string>& tokens)
{
    
    int idx = 0;

    std::regex delim(" ");

    std::sregex_token_iterator begin(msg.begin(), msg.end(), delim, -1);

    std::sregex_token_iterator end;

    for (auto it = begin; it != end; ++it)
    {
        tokens.emplace_back(*it);
        idx++;
    }
    return idx;
}

int kvstore_parser_protocol(struct conn_item *item, std::vector<std::string>& tokens)
{
    if(item == nullptr || tokens.empty())
        return -1;

    int cmd = KVS_CMD_START;
    for (auto comm : commands)
    {
        if(comm == tokens[0])
        {
            break;
        }
        cmd++;
    }

    char *msg = item->wbuffer;
    memset(msg, 0, BUFFER_LENGTH);

    switch(cmd)
    {
        case KVS_CMD_SET:
        {
            //std::cout << "set" << std::endl;
            int res = kvstore_array_set(tokens[1], tokens[2]);
            if(!res)
            {
                snprintf(msg, BUFFER_LENGTH, "SUCCESS");
            }
            break;
        }
        case KVS_CMD_GET:
        {
            //std::cout << "get" << std::endl;
            std::string value = kvstore_array_get(tokens[1]);
            if(value.empty())
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "NO EXIST");
            }
            else
            {
                snprintf(msg, BUFFER_LENGTH, "%s", value.c_str());
            }
            break;
        }
        case KVS_CMD_DEL:
        {
            //std::cout << "del" << std::endl;
            int res = kvstore_array_delete(tokens[1]);
            if(res < 0)
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
            }
            else if(res == 0)
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
            }
            else
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "NO EXIST");
            }
            break;
        }
        case KVS_CMD_MOD:
        {
            //std::cout << "mod" << std::endl;
            int res = kvstore_array_modify(tokens[1], tokens[2]);
            if(res < 0)
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
            }
            else if(res == 0)
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
            }
            else
            {
                snprintf(msg, BUFFER_LENGTH, "%s", "NO EXIST");
            }
            break;
        }
        default:
        {
            snprintf(msg, BUFFER_LENGTH, "%s", "Unknown command!");
            break;
        }
    }
    return 1;
}

int kvstore_request(struct conn_item *item)
{
    //std::cout << "recv: " << item->rbuffer << std::endl;

    std::string msg = item->rbuffer;
    std::vector<std::string> tokens;
    int count = kvstore_split_token(msg, tokens);

    for(auto token : tokens)
    {
        //std::cout << token << std::endl;
    }

    kvstore_parser_protocol(item, tokens);
    return count;
}

int main()
{
    // 如果需要扩展，添加源文件，入口函数即可
    epoll_entry();
    return 0;
}