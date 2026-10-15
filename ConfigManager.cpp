#include "ConfigManager.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

ConfigManager::ConfigManager(const std::string &config_file)
{
    loadConfig(config_file);
}

void ConfigManager::loadConfig(const std::string &config_file)
{
    std::ifstream file(config_file);
    if (!file)
    {
        std::cerr << "Error: Could not open config file " << config_file << "\n";
        exit(1);
    }

    json config;
    file >> config;

    num_threads = config["num_threads"];
    num_replicas = config["num_replicas"];
    total_dataset_size = config["total_dataset_size"];
    requests_per_thread = config["requests_per_thread"];
    cache_percentage = config["cache_percentage"];
    rdma_enabled = config["rdma_enabled"];
    enable_cba = config["enable_cba"];
    enable_de_duplication = config["enable_de_duplication"];
    is_access_rate_fixed = config["is_access_rate_fixed"];
    fixed_access_rate = config["fixed_access_rate_value"];
    // Optional: apply the "set" records of the trace as writes instead of treating every record as a get
    process_writes = config.value("process_writes", false);
    // Optional: size of Ldc's per-replica tiny write queue as a fraction of the cache (paper: 0.01%)
    tiny_queue_fraction = config.value("tiny_queue_fraction", 0.0001);
    update_interval = config.value("cba_update_interval", 10);
    latency_local = config["latency_local"];
    latency_rdma = config["latency_rdma"];
    latency_disk = config["latency_disk"];
    workload_folder = config["workload_folder"];
    cache_type = config["cache_type"];

    updateCacheSize();
}

void ConfigManager::printConfig()
{
    std::cout << "Configuration:\n"
              << "  Number of threads: " << num_threads << "\n"
              << "  Number of replicas: " << num_replicas << "\n"
              << "  Total dataset size: " << total_dataset_size << "\n"
              << "  Requests per thread: " << requests_per_thread << "\n"
              << "  Cache percentage: " << cache_percentage << "\n"
              << "  RDMA enabled: " << (rdma_enabled ? "true" : "false") << "\n"
              << "  CBA enabled: " << (enable_cba ? "true" : "false") << "\n"
              << "  De-duplication enabled: " << (enable_de_duplication ? "true" : "false") << "\n"
              << "  Fixed access rate: " << (is_access_rate_fixed ? std::to_string(fixed_access_rate) : "false") << "\n"
              << "  Process writes: " << (process_writes ? "true" : "false") << "\n"
              << "  Tiny queue fraction: " << tiny_queue_fraction << "\n"
              << "  CBA update interval: " << update_interval << " seconds\n"
              << "  Local latency: " << latency_local << " us\n"
              << "  RDMA latency: " << latency_rdma << " us\n"
              << "  Disk latency: " << latency_disk << " us\n"
              << "  Workload folder: " << workload_folder << "\n";
}

void ConfigManager::updateCacheSize()
{
    cache_size = static_cast<int>(cache_percentage * total_dataset_size);
}