#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>

#define MAX_BYTES_PER_WRITE 1024 * 1024 // 1MB

enum fsync_mode {
    FSYNC_NONE = 0,
    FSYNC_EACH,
};

struct options {
    size_t write_size;
    size_t write_total;
    enum fsync_mode fsync_mode;
};

enum option_type {
    OPT_WRITE_SIZE = 256,
    OPT_WRITE_TOTAL,
    OPT_FSYNC_EACH,
};

void run_append_test(struct options *opts)
{
    printf("file append test:\n"
           "%zu bytes per write\n"
           "%zu bytes total\n"
           "fsync mode %d\n",
           opts->write_size, opts->write_total, opts->fsync_mode);

    int fd = open("diskperf.out", O_CREAT | O_TRUNC | O_WRONLY | O_APPEND, 0660);
    if (fd == -1) {
        fprintf(stderr, "open file diskperf.out failed (errno=%d)", (int)errno);
        exit(1);
    }

    static char data[MAX_BYTES_PER_WRITE];
    memset(data, 0xFE, sizeof(data));

    size_t write_num = (opts->write_total + opts->write_size - 1) / opts->write_size;

    struct timeval begin;
    gettimeofday(&begin, NULL);
    printf("begin time: %u.%u\n", (unsigned)begin.tv_sec, (unsigned)begin.tv_usec);

    for (size_t i = 0; i < write_num; ++i) {
        ssize_t written = write(fd, data, opts->write_size);
        if (written == -1) {
            fprintf(stderr, "write() failed (err=%s)\n", strerror(errno));
            exit(1);
        }

        if (opts->fsync_mode == FSYNC_EACH) {
            fsync(fd);
        }
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    printf("end time: %u.%u\n", (unsigned)end.tv_sec, (unsigned)end.tv_usec);

    close(fd);

    unsigned elapsed = (end.tv_sec - begin.tv_sec) * 1000000 +
        ((int)end.tv_usec - (int)begin.tv_usec);

    printf("elapsed time: %uus throughput: %zu bytes/second\n",
           elapsed, opts->write_size * write_num * 1000000 / elapsed);
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
    options.write_size = 4096;
    options.write_total = 1024 * 1024 * 1024; // 1GB
    options.fsync_mode = FSYNC_NONE;

    static struct option longopts[] = {
        {"write-size", required_argument, NULL, OPT_WRITE_SIZE},
        {"write-total", required_argument, NULL, OPT_WRITE_TOTAL},
        {"fsync-each", no_argument, NULL, OPT_FSYNC_EACH},
    };

    int ch = 0;
    bool ok = false;
    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (ch) {
        case OPT_WRITE_SIZE:
            ok = parse_size_t(optarg, &options.write_size);
            if (!ok) {
                fprintf(stderr, "error: invalid argument to --write-size\n");
                return 1;
            }
            break;
        case OPT_WRITE_TOTAL:
            ok = parse_size_t(optarg, &options.write_total);
            if (!ok) {
                fprintf(stderr, "error: invalid argument to --write-total\n");
                return 1;
            }
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
    if (options.write_size == 0 ||
        options.write_size > MAX_BYTES_PER_WRITE) {
        fprintf(stderr, "error: invalid write size (%zu)\n", options.write_size);
        return 1;
    }

    /* Run test */
    run_append_test(&options);

    return 0;
}
