#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  printf(1, "The total number of system calls successfully executed so far is: %d\n", PartC());
  exit();
}