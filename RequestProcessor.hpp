#ifndef REQUEST_PROCESSOR_HPP
#define REQUEST_PROCESSOR_HPP

#include "ReplicaManager.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <regex>

// One trace record; kept at 8 bytes because the loader reserves one record per byte of the trace file
struct Request
{
    int key;
    uint16_t replica; // 1-based replica id (processFirstFile picks a random replica instead)
    bool is_write;    // "set" in the operation column
};

class RequestProcessor
{
private:
    std::string folder_path;
    uint64_t total_requests = 0;

    bool isValidFileFormat(const std::string &filename);
    std::vector<Request> loadRequestsFromFile(const std::string &file_path);
    std::vector<Request> loadRequestsFromTracesFile(const std::string &file_path);

public:
    RequestProcessor(const std::string &folder);

    void processAllFilesParallel(ReplicaManager &manager);
    void processFirstFile(ReplicaManager &manager);
};

#endif // REQUEST_PROCESSOR_HPP
