#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  printf(1, "My process ID: %d\n", getpid());
  exit();
}
