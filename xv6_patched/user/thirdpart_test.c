#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    printf(1, "Calling thirdpart syscall...\n");

    int result = thirdpart();   // user-level wrapper from usys.S / user.h FIXED FINALLY

    printf(1, "thirdpart syscall returned %d\n", result);

    exit();
}
