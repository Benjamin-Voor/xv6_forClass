#include "types.h"
#include "user.h"

int
main(void) {
    int pid = getpid();
    printf(1, "Setting priority of %d to 20\n", pid);
    setpriority(pid, 20);
    exit();
}