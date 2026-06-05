#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    // Exit if there ain't exactly 1 arg
    if (argc != 2)
    {
        //fprintf(fp, "format", arg1, ...)
        fprintf(2, "Need exactly 1 argument but passed %d\n", argc-1);
        return -1;
    }

    int sleepTime = atoi(argv[1]);

    fprintf(2, "Sleeping for %d ticks\n", sleepTime);

    if (pause(sleepTime) == -1)
    {
        fprintf(2, "pause() failed\n");
        return -1;
    }

    return 0;
}