
int kill(int pid, int sig) { (void)pid; (void)sig; return -1; }
int system(const char *command) { (void)command; return -1; }
int fork(void) { return -1; }
char *getlogin(void) { return 0; }

void *dlopen(const char *file, int mode) { (void)file; (void)mode; return 0; }
int dlclose(void *handle) { (void)handle; return -1; }
void *dlsym(void *handle, const char *symbol) { (void)handle; (void)symbol; return 0; }
char *dlerror(void) { return "dynamic loading is not supported in this build"; }
