/*
 * writer_fcntl.c - append one log line to system.log using fcntl() record locks
 *
 * | Property                | flock                     | fcntl                            |
 * |-------------------------|---------------------------|----------------------------------|
 * | Lock granularity        | Whole file only           | Byte range supported             |
 * | Works over NFS          | No                        | Yes                              |
 * | Inherited across fork   | Yes                       | No                               |
 * | Best used when          | Simple local file locking | Network FS or byte-range locking |
 */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define LOG_FILE  "system.log"
#define LINE_SIZE 1024

static int format_line(char *buf, size_t size, const char *msg) {
    char ts[32];
    time_t now = time(NULL);
    struct tm tm_now;

    if (localtime_r(&now, &tm_now) == NULL)
        return -1;
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_now);

    int len = snprintf(buf, size, "[PID:%d] [%s] [INFO] %s\n",
                       (int)getpid(), ts, msg);
    if (len < 0)
        return -1;
    if ((size_t)len >= size) {      // message truncated: keep the newline
        buf[size - 2] = '\n';
        len = (int)size - 1;
    }
    return len;
}

static int write_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"message text\"\n", argv[0]);
        return 1;
    }

    char line[LINE_SIZE];
    int len = format_line(line, sizeof(line), argv[1]);
    if (len < 0) {
        fprintf(stderr, "format log line failed\n");
        return 1;
    }

    int fd = open(LOG_FILE, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct flock fl = {
        .l_type   = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start  = 0,
        .l_len    = 0,          // 0 = lock to end of file (whole file)
    };
    if (fcntl(fd, F_SETLKW, &fl) < 0) {
        perror("fcntl F_SETLKW");
        close(fd);
        return 1;
    }

    int ret = 0;
    if (write_all(fd, line, (size_t)len) < 0) {
        perror("write");
        ret = 1;
    }

    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        perror("fcntl F_UNLCK");
        ret = 1;
    }

    close(fd);
    return ret;
}
