#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

/*
 * Metrics to benchmark: BPS IOPS
 * Settings: Sequential/Random, Read/Write, Different IO Size, fsync() Mode, Operation Type
 */

#define MAX_BYTES_PER_OP (16 * 1024 * 1024) /* 16MiB */

enum op_type {
    OP_READ = 0,
    OP_WRITE,
};

enum access_mode {
    ACCESS_SEQ = 0,
    ACCESS_RANDOM,
};

enum fsync_mode {
    FSYNC_NONE = 0,
    FSYNC_EACH,
};

struct options {
    size_t op_size;
    size_t op_num;
    enum op_type op_type;
    enum fsync_mode fsync_mode;
    enum access_mode access_mode;
    const char *filename;
    size_t align;
};

enum option_type {
    OPT_OP_SIZE = 256,
    OPT_OP_NUM,
    OPT_WRITE,
    OPT_RANDOM,
    OPT_FSYNC_EACH,
    OPT_FILE,
    OPT_ALIGN,
};

struct context {
    int fd;
    size_t file_size;
    char buffer[MAX_BYTES_PER_OP];
    struct timeval begin;
    struct timeval end;
};

const char *op_type_str(enum op_type t)
{
    switch (t) {
    case OP_READ: return "READ";
    case OP_WRITE: return "WRITE";
    default:
        abort();
    }
}

const char *access_mode_str(enum access_mode m)
{
    switch (m) {
    case ACCESS_SEQ: return "SEQ";
    case ACCESS_RANDOM: return "RANDOM";
    default:
        abort();
    }
}

const char *fsync_mode_str(enum fsync_mode m)
{
    switch (m) {
    case FSYNC_NONE: return "NONE";
    case FSYNC_EACH: return "EACH";
    default:
        abort();
    }
}

void setup(struct context *ctx, struct options *opts)
{
    /*
     * Initialize buffer for both READ and WRITE test.
     * In READ test, buffer is used when creating file;
     * in WRITE test, buffer is used to writing to file.
     */
    memset(ctx->buffer, 0xEF, sizeof(ctx->buffer));
    
    /* Open file */
    int oflag = O_CREAT | O_RDWR;
    if (opts->op_type == OP_WRITE && opts->access_mode == ACCESS_SEQ) {
        oflag |= (O_APPEND | O_TRUNC);
    }
    ctx->fd = open(opts->filename, oflag, 0600);
    if (ctx->fd == -1) {
        fprintf(stderr, "open file %s failed (err=%s)\n",
                opts->filename, strerror(errno));
        exit(1);
    }

    /* Get file size */
    if (opts->op_type == OP_WRITE && opts->access_mode == ACCESS_SEQ) {
        ctx->file_size = 0;
    } else {
        struct stat st;
        int err = fstat(ctx->fd, &st);
        if (err == -1) {
            fprintf(stderr, "fstat() failed (err=%s)\n", strerror(errno));
            exit(1);
        }
        ctx->file_size = st.st_size;

        if (ctx->file_size < opts->op_size) {
            fprintf(stderr, "op size(%zu) is greater than file size(%zu)\n",
                    opts->op_size, ctx->file_size);
            exit(1);
        }

        if (ctx->file_size < opts->align) {
            fprintf(stderr, "alignment(%zu) is greater than file size(%zu)\n",
                    opts->align, ctx->file_size);
            exit(1);
        }
    }

    if (opts->access_mode == ACCESS_RANDOM) {
        srand(time(NULL));
    }
}

void run_benchmark(struct context *ctx, struct options *opts)
{
    gettimeofday(&ctx->begin, NULL);

    size_t i = 0;
    for (; i < opts->op_num; ++i) {
        /* Seek file if in random mode if needed */
        if (opts->access_mode == ACCESS_RANDOM) {
            size_t pos = rand() % (ctx->file_size - opts->op_size + 1);
            pos = pos - (pos % opts->align);
            off_t offset = lseek(ctx->fd, pos, SEEK_SET);
            if (offset == (off_t)-1) {
                fprintf(stderr, "lseek() failed (err=%s)\n", strerror(errno));
                exit(1);
            }
        }

        /* read or write */
        ssize_t nbytes = 0;
        if (opts->op_type == OP_READ) {
            nbytes = read(ctx->fd, ctx->buffer, opts->op_size);
        } else {
            nbytes = write(ctx->fd, ctx->buffer, opts->op_size);
        }
        if (nbytes == -1) {
            fprintf(stderr, "write()/read() failed (err=%s)\n", strerror(errno));
            exit(1);
        }

        if (opts->fsync_mode == FSYNC_EACH) {
            fsync(ctx->fd);
        }
    }

    gettimeofday(&ctx->end, NULL);
}

void report(struct context *ctx, struct options *opts)
{
    unsigned elapsed = (ctx->end.tv_sec - ctx->begin.tv_sec) * 1000000 +
        ((int)ctx->end.tv_usec - (int)ctx->begin.tv_usec);

    size_t total_size = opts->op_size * opts->op_num;
    size_t bps = total_size * 1000000 / elapsed;
    size_t iops = opts->op_num * 1000000 / elapsed;

    printf("bps: %zu iops: %zu\n", bps, iops);
}

void teardown(struct context *ctx)
{
    close(ctx->fd);
}

void run_append_test(struct options *opts)
{
    /* Print parameters */
    printf("op %s size %zu num %zu access %s fsync %s\n",
           op_type_str(opts->op_type),
           opts->op_size, opts->op_num,
           access_mode_str(opts->access_mode),
           fsync_mode_str(opts->fsync_mode));

    /* Setup context */
    static struct context ctx;
    setup(&ctx, opts);
    
    /* Run benchmark */
    run_benchmark(&ctx, opts);

    /* Report results */
    report(&ctx, opts);
    
    /* Teardown context */
    teardown(&ctx);
}

bool parse_size_t(const char *arg, size_t *rslt)
{
    char *endptr = NULL;
    long long n = strtoll(arg, &endptr, 0);
    if (*endptr != '\0') return false;
    if (n == 0) {
        if (errno == EINVAL ||
            errno == ERANGE) return false;
    }
    if (n < 0) return false;

    *rslt = (size_t)n;
    return true;
}

int main(int argc, char *argv[])
{
    struct options options;
    options.op_size = 4096;
    options.op_num = 128 * 1024;
    options.op_type = OP_READ;
    options.access_mode = ACCESS_SEQ;
    options.fsync_mode = FSYNC_NONE;
    options.filename = strdup("diskperf.data");
    options.align = 1;

    static struct option longopts[] = {
        {"op-size", required_argument, NULL, OPT_OP_SIZE},
        {"op-num", required_argument, NULL, OPT_OP_NUM},
        {"write", no_argument, NULL, OPT_WRITE},
        {"random", no_argument, NULL, OPT_RANDOM},
        {"fsync-each", no_argument, NULL, OPT_FSYNC_EACH},
        {"file", required_argument, NULL, OPT_FILE},
        {"align", required_argument, NULL, OPT_ALIGN},
    };

    int ch = 0;
    bool ok = false;
    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (ch) {
        case OPT_OP_SIZE:
            ok = parse_size_t(optarg, &options.op_size);
            if (!ok) {
                fprintf(stderr, "error: invalid argument to --op-size\n");
                return 1;
            }
            break;
        case OPT_OP_NUM:
            ok = parse_size_t(optarg, &options.op_num);
            if (!ok) {
                fprintf(stderr, "error: invalid argument to --op-num\n");
                return 1;
            }
            break;
        case OPT_ALIGN:
            ok = parse_size_t(optarg, &options.align);
            if (!ok || options.align == 0) {
                fprintf(stderr, "error: invalid argument to --align\n");
                return 1;
            }
            break;
        case OPT_FILE:
            free((void *)options.filename);
            options.filename = strdup(optarg);
            break;
        case OPT_WRITE:
            options.op_type = OP_WRITE;
            break;
        case OPT_RANDOM:
            options.access_mode = ACCESS_RANDOM;
            break;
        case OPT_FSYNC_EACH:
            options.fsync_mode = FSYNC_EACH;
            break;
        default:
            /* TODO(Yi Zhenfei): more serious error handling */
            continue;
        }
    }

    /* Extra option check */
    if (options.op_size == 0 ||
        options.op_size > MAX_BYTES_PER_OP) {
        fprintf(stderr, "error: invalid argument to --op-size (%zu)\n", options.op_size);
        return 1;
    }

    /* Run test */
    run_append_test(&options);

    /* Free option */
    free((void *)options.filename);

    return 0;
}
