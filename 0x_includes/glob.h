#ifndef GLOB_H
#define GLOB_H

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdint.h>
#include <errno.h>
#include <netdb.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#elif __linux__
#include <dirent.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif

typedef enum{
    ARGS,
}   err_code;

void    errors(err_code err, char *reason);
void    ensure_dump_folder(void);
void    write_to_file(const char *path, const char *content);
void    append_to_file(const char *path, const char *content);

#endif