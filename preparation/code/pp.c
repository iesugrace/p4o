/*
 * Author:   Joshua Chen <iesugrace@gmail.com>
 * Date:     2016-06-19 16:21:36 
 * Location: Shenzhen
 * Desc:     Get all parent processes information
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

struct line {
    char *data;
    struct line *next;
    struct line *prev;
};

char *skip_lspace(char *str) {
    while(*str == ' ') str++;
    return str;
}

char *skip_lnonspace(char *str) {
    while(*str != ' ') str++;
    return str;
}

void getparents(pid_t pid) {
    char *name;
    size_t len = 0;
    ssize_t read;
    struct line *first;
    struct line *info;
    struct line *keep_info;
    struct line *info_new;
    struct line *res_info;
    struct line *res_info_new;
    char *p;
    FILE *ofile;
    pid_t _pid, ppid;
    int ends[2];
    pid_t child;
    int status;

    // fetch the process information
    if (pipe(ends) != 0) {
        fprintf(stderr, "failed to create pipe\n");
        return;
    }
    if ((child = fork()) == -1) {
        fprintf(stderr, "failed to fork\n");
        return;
    } else if (child == 0) {    // child
        dup2(ends[1], 1);
        //execlp("ps", "ps", "-eo", "pid,ppid,comm", (char *)NULL);
        execlp("cat", "cat", "/tmp/ps", (char *)NULL);
    }
    wait(&status);
    //debug: while it blocks?
    //close(ends[1]);
    ofile = fdopen(ends[0], "r");
    info  = calloc(1, sizeof(struct line));
    first = info;
    while ((read = getline(&(info->data), &len, ofile)) != -1) {
        info_new  = calloc(1, sizeof(struct line));
        info->next = info_new;
        info = info_new;
    }

    // find the parents
    res_info  = calloc(1, sizeof(struct line));
    while (1) {
        info = first;
        while (info != (struct line *)NULL) {
            p = skip_lspace(info->data);
            _pid = atoi(p);
            if (_pid == pid) {
                p = skip_lnonspace(p);
                p = skip_lspace(p);
                ppid = atoi(p);
                p = skip_lnonspace(p);
                name = skip_lspace(p);
                name[strlen(name)-1] = '\0';
                keep_info->next = info->next;   // skip this one for next loop
                break;
            }
            keep_info = info;
            info = info->next;
        }
        res_info->data = malloc(256);
        snprintf(res_info->data, 256, "%d (%s)", pid, name);
        if (ppid == 0)
            break;
        pid = ppid;
        res_info_new   = calloc(1, sizeof(struct line));
        res_info_new->prev = res_info;
        res_info = res_info_new;
    }
    // free process info memory
    info = first;
    while (info != (struct line *)NULL) {
        free(info->data);
        keep_info = info;
        info = info->next;
        free(keep_info);
    }

    // output, and free memory
    while (res_info != (struct line *)NULL) {
        printf("%s\n", res_info->data);
        free(res_info->data);
        keep_info = res_info;
        res_info = res_info->prev;
        free(keep_info);
    }
}

int main(int argc, char **argv) {
    pid_t pid;
    if (argc == 1) {
        pid = getpid();
    } else if (argc == 2) {
        char *c = argv[1];
        while (*c) {
            if (*c < '0' || *c > '9') {
                fprintf(stderr, "invalid pid: %d\n", pid);
                exit(1);
            }
            c++;
        }
        pid = atoi(argv[1]);
        char cmd[1024];
        sprintf(cmd, "ps -eo pid | grep -qw %d", pid);
        if (system(cmd) != 0) {
            fprintf(stderr, "%d not exists\n", pid);
            exit(1);
        }
    } else {
        char *bname;
        bname = rindex(argv[0], '/');
        if (bname == NULL)
            bname = argv[0];
        else
            bname++;
        fprintf(stderr, "Usage: %s [pid]\n", bname);
        exit(1);
    }

    int i;
    for (i = 0; i < 200; i++) {
        getparents(pid);
    }
}
