#include <unordered_map>
#include <vector>
#include <memory>
#include <sys/time.h>

#include "util.h"

typedef std::unordered_map<int, int> int_map;

enum operation_type {
    OT_INSERT = 0,
    OT_DELETE,
    OT_INSDEL,
    OT_HIT,
    OT_MISS,
};

struct options {
    int map_size = 128 * 1024 * 1024;
    int op_num = 128 * 1024 * 1024;
    int round = 5;
    operation_type op_type = OT_INSERT;
};

struct context {
    std::vector<std::unique_ptr<int_map>> maps;
    unsigned long total_us = 0;
};

void do_creation(int_map &map, int size)
{
    for (int i = 0; i < size; ++i) {
        map[i] = i;
    }
}

void setup_context(context &ctx, options &opts)
{
    for (int i = 0; i < opts.round; ++i) {
        ctx.maps.push_back(std::make_unique<int_map>());
    }
}

const char *op_str(operation_type op)
{
    switch (op) {
    case OT_INSERT: return "INSERT";
    case OT_DELETE: return "DELETE";
    case OT_INSDEL: return "INSDEL";
    case OT_HIT: return "HIT";
    case OT_MISS: return "MISS";
    default: return "UNKNOWN";
    }
}

void report(context &ctx, options &opts)
{
    unsigned long avg_round_time = ctx.total_us / opts.round;
    unsigned long long op_num = opts.op_type == OT_INSERT ?
        opts.map_size : opts.op_num;
    auto ops = op_num * opts.round * 1000000 / ctx.total_us;

    sp_info("std::unordered_set benchmark op: %s op_num: %d map_size: %d "
            "round: %d\n"
            "total_time: %lu avg_round_time: %lu ops: %lu",
            op_str(opts.op_type), opts.op_num, opts.map_size, opts.round,
            ctx.total_us, avg_round_time, ops);
}

void benchmark_insert(context &ctx, options &opts)
{
    setup_context(ctx, opts);

    struct timeval begin;
    struct timeval end;
    gettimeofday(&begin, NULL);

    for (int i = 0; i < opts.round; ++i) {
        do_creation(*ctx.maps[i].get(), opts.map_size);
    }
    
    gettimeofday(&end, NULL);
    ctx.total_us = sp_usec_diff(&begin, &end);

    report(ctx, opts);
}

int main(int argc, char *argv[])
{
    options opts;
    context ctx;

    switch (opts.op_type) {
    case OT_INSERT:
        benchmark_insert(ctx, opts);
        break;
    case OT_DELETE:
    case OT_INSDEL:
    case OT_HIT:
    case OT_MISS:
        break;
    }

    return 0;
}
