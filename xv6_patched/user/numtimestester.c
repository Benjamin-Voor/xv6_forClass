#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
	printf(1, "This is my PID:%d\n", numtimesgetpid());
	exit();
}
