#include<string>

#ifndef __KVSTORE_H__
#define __KVSTORE_H__

#define BUFFER_LENGTH 1024

#ifdef ENABLE_LOG

#define LOG(_FMT, ...) fprintf(stdout, "[%s:%d]: %s_fmt, __FILE__, __LINE__, __VAR_ARGS__")

#else

#define LOG(_fmt, ...)

#endif

typedef int (*RCALLBACK)(int fd);

struct conn_item{
    int fd;

    char rbuffer[BUFFER_LENGTH];
    int rlen;
    char wbuffer[BUFFER_LENGTH];
    int wlen;

    char resource[BUFFER_LENGTH];

    union{
        RCALLBACK accept_callback;
        RCALLBACK recv_callback;
    } recv_t;
    RCALLBACK send_callback;
};

int epoll_entry(void);

int kvstore_request(struct conn_item *item);

void *kvstore_malloc(size_t size);

void kvstore_free(void *ptr);

#define ENABLE_ARRAY_KVENGINE

#ifdef ENABLE_ARRAY_KVENGINE

struct kvs_array_item{
    std::string key;
    std::string value;
    bool is_valid;
};

#define KVS_ARRAY_SIZE 1024

int kvstore_array_set(std::string& key, std::string& value);

std::string kvstore_array_get(std::string& key);

int kvstore_array_delete(std::string &key);

int kvstore_array_modify(std::string &key, std::string &value);

#endif

#endif