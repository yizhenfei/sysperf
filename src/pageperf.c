#include "util.h"

#include <sys/mman.h>

typedef struct {
    size_t page_size; /* Page size */
    size_t mem_size; /* Total memory size to touch */
} option_t;

typedef struct {
    void *mem;
} context_t;

void init_ctx(context *ctx, options *opts)
{
    ctx->mem = mmap(NULL, opts->mem_size,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);
    if (!ctx->mem) {
        sp_errno("mmap failed for size %zu", opts->mem_size);
    }
}

void do_benchmark()
{
    
}

int main(int argc, char *argv[])
{
    option_t opts;
    opts.page_size = 4096;
    opts.mem_size = 1024 * 1024 * 1024;

    context_t ctx;
    init_ctx(&ctx, &opts);
    
    do_benchmark();
    
    return 0;
}
