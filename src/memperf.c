#include <sys/mman.h>
#include <sys/time.h>

#include "util.h"

#define PAGE_SIZE 4096

typedef enum {
    OP_TOUCH,
    OP_READ,
    OP_WRITE,
} op_type;

typedef enum {
    ACC_SEQ,
    ACC_RND,
} access_mode;

typedef struct {
    size_t mem_size;
    op_type op_type;
    size_t op_num;
    access_mode acc_mode;
    size_t stride;
    int round_num;
} options;

typedef struct {
    void *mem;
    struct timeval begin;
    struct timeval end;
} context;

void init_seq_map(void *mem, size_t size)
{
    /* TODO(frank): check for size % sizeof(void *) == */
    size_t nelem = size / sizeof(void *);
    void **elem = mem;    
    void **last = elem + nelem - 1;

    while (elem != last) {
        *elem = elem+1;
        ++elem;
    }
    *elem = mem;
}

void init_context(context *ctx, options *opts)
{
    ctx->mem = mmap(NULL, opts->mem_size,
                   PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE,
                    -1, 0);
    if (ctx->mem == MAP_FAILED) {
        sp_error("mmap() failed, size=%zu", opts->mem_size);
    }

    if (opts->op_type != OP_TOUCH) {
        switch (opts->acc_mode) {
        case ACC_SEQ:
            init_seq_map(ctx->mem, opts->mem_size);
            break;
        case ACC_RND:
            break;
        default:
            break;
        }
    }
}

void do_read(context *ctx, options *opts)
{
    void **elem = &ctx->mem;
    for (size_t i = 0; i < opts->op_num; ++i) {
        elem = *elem;
    }
}

void do_touch(context *ctx, options *opts)
{
    char *elem = ctx->mem;
    size_t stride = opts->stride;
    char *end = elem + opts->op_num;

    /* TODO(Yi Zhenfei): check for overrun */
    while (elem != end) {
        *elem = 0;
        elem += stride;
    }
}

void do_benchmark(context *ctx, options *opts)
{
    switch (opts->op_type) {
    case OP_TOUCH:
        return do_touch(ctx, opts);
    case OP_READ:
        return do_read(ctx, opts);
    case OP_WRITE:
        return;
    default:
        return;
    }
}

const char *opstr(op_type o)
{
    switch (o) {
    case OP_READ: return "read";
    case OP_WRITE: return "write";
    default: return "unknown";
    }
}

const char *accstr(access_mode a)
{
    switch (a) {
    case ACC_SEQ: return "seq";
    case ACC_RND: return "rnd";
    default: return "unknown";
    }
}

const char *switchstr(bool b)
{
    if (b) {
        return "on";
    } else {
        return "off";
    }
}

void echo(options *opts)
{
    sp_info("memperf o:%s/a:%s/m:%zu/n:%zu/r:%d",
            opstr(opts->op_type),
            accstr(opts->acc_mode),
            opts->mem_size,
            opts->op_num,
            opts->round_num);
}

void report(int round_num, context *ctx, options *opts)
{
    unsigned long us = sp_usec_diff(&ctx->begin, &ctx->end);
    unsigned long long ops = us == 0 ? 0 : (unsigned long long)opts->op_num * 1000000 / us;
    unsigned long long bps = us == 0 ? 0 : (unsigned long long)opts->op_num * sizeof(void *) * 1000000 / us;
    sp_info("round(%d) time: %luus ops: %llu bps: %llu", round_num, us, ops, bps);
}     

void run_benchmark(options *opts)
{
    echo(opts);
    context ctx;
    init_context(&ctx, opts);
    for (int i = 0; i < opts->round_num; ++i) {
        gettimeofday(&ctx.begin, NULL);
        do_benchmark(&ctx, opts);
        gettimeofday(&ctx.end, NULL);
        report(i, &ctx, opts);
    }
}

int main(int argc, char *argv[])
{
    options opts;
    opts.mem_size = 1024 * 1024 * 1024;
    opts.op_type = OP_TOUCH;
    opts.op_num = 256 * 1024;
    opts.acc_mode = ACC_SEQ;
    opts.stride = PAGE_SIZE;
    opts.round_num = 5;

    run_benchmark(&opts);

    return 0;
}

