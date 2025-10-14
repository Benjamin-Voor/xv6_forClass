#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    // Call the kernel-provided ps syscall
    ps();
    exit();
    return 0;
}
