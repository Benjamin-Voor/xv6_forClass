#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[])
{
    struct pstat mypointer;

    mypointer.inuse[0]=5;
    printf(1, "Use: %d\n", mypointer.inuse[0]);

    exit();
}