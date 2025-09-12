#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "Syscalls so far: %d\n", ThirdPart());
  getpid();   // mskes one syscall
  printf(1, "Syscalls so far: %d\n", ThirdPart());
  sleep(1);   // another syscall
  printf(1, "Syscalls so far: %d\n", ThirdPart());
  exit();
}
