#!/bin/bash
# Checks that "set" records take the write path. Usage: ./test_writes.sh [path/to/CacheSimulator]
# Every case is independent of which replica the simulator picks for a read.

SIM=$(realpath "${1:-./build/CacheSimulator}")
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
FAILED=0

# run_case <name> <replicas> <rdma> <process_writes> <tiny_queue_fraction> <expected latency> <expected writes> <trace lines...>
run_case() {
    local name=$1 replicas=$2 rdma=$3 writes=$4 tq=$5 want_latency=$6 want_writes=$7
    shift 7
    mkdir -p "$TMP/$name/traces/test/$name"
    printf '%s\n' "$@" >"$TMP/$name/traces/test/$name/seq.txt"
    cat >"$TMP/$name/config.json" <<EOF
{
  "num_threads": 1, "num_replicas": $replicas, "total_dataset_size": 10, "requests_per_thread": 10,
  "cache_percentage": 0.3, "rdma_enabled": $rdma, "enable_cba": false, "enable_de_duplication": false,
  "is_access_rate_fixed": false, "fixed_access_rate_value": 1000000, "cba_update_interval": 1000,
  "latency_local": 1, "latency_rdma": 19, "latency_disk": 296,
  "workload_folder": "$TMP/$name/traces/test/$name", "cache_type": "LRU",
  "process_writes": $writes, "tiny_queue_fraction": $tq
}
EOF
    (cd "$TMP/$name" && "$SIM" >/dev/null 2>&1)
    local out
    out=$(ls "$TMP/$name"/workload/test/"$name"/workload_*.txt 2>/dev/null)
    local latency got_writes
    latency=$(awk -F': ' '/^Average Latency/ {print $2}' "$out" 2>/dev/null)
    got_writes=$(awk -F': ' '/^Total Writes/ {print $2}' "$out" 2>/dev/null)
    if [[ "$latency" == "$want_latency" && "${got_writes:-0}" == "$want_writes" ]]; then
        echo "PASS $name (latency $latency, writes ${got_writes:-0})"
    else
        echo "FAIL $name: latency '$latency' (want $want_latency), writes '${got_writes:-0}' (want $want_writes)"
        FAILED=1
    fi
}

# Old behaviour: without process_writes every record is a read (one replica, so no random choice):
# miss, miss, hit -> (296 + 296 + 1) / 3
run_case writes_off 1 false false 0.0001 197.667 0 "1 set" "2 set" "1"
# OrigCache: a write brings the object into every replica's cache, so the read hits locally
run_case orig_write_then_read 3 false true 0.0001 1 2 "1 set" "2 set" "1"
# Ldc: writes go to the tiny queue (1 entry here), so key 1 is demoted by the next write and the read goes to disk
run_case ldc_write_demoted 3 true true 0.0001 296 2 "1 set" "2 set" "1"
# Ldc: a read of an object still in the tiny queue is a local hit
run_case ldc_read_from_queue 3 true true 0.0001 1 1 "1 set" "1" "1"
# Ldc without the tiny queue (LDC-notq) writes through the main cache like OrigCache
run_case ldc_no_queue 3 true true 0 1 2 "1 set" "2 set" "1"

exit $FAILED
