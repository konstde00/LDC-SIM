#include "Replica.hpp"

Replica::Replica(int replica_id, size_t cache_size, const std::string &cache_type, size_t write_queue_size)
    : id(replica_id)
{
    // The tiny queue is carved from the cache, so the total cache size stays the same
    if (write_queue_size > 0)
    {
        write_queue = std::make_unique<LRUCache>(write_queue_size);
        cache_size -= write_queue_size;
    }
    if (cache_type == "S3FIFO")
    {
        cache = std::make_unique<S3FIFOCache>(cache_size);
        std::cout << "Replica " << id << "created with S3FIFO cache\n";
    }
    else if (cache_type == "LRU")
    {
        cache = std::make_unique<LRUCache>(cache_size);
        std::cout << "Replica " << id << "created with LRU cache\n";
    }
    else
    {
        throw std::invalid_argument("Unsupported cache type: " + cache_type);
    }
}

int Replica::processRequest(int key)
{
    return cache->get(key);
}

bool Replica::hasKey(int key)
{
    return cache->contains(key);
}
