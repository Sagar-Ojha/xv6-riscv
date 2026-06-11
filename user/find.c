#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include <stdbool.h>

static bool execOption = false;  // Flag for handling exec option. Set to "true" if "-exec" is found
static char* execCmd;       // Points to the command exec takes
static char cmdArg[512];    // At max, 512 bytes "should" be passed as the argument to the process run by exec
                            // TODO: Implement overflow protection

int findFile(char* path, char* fileName)
{
    /*
    Get the file descriptor and check the type of file. If it's a directory,
    then check the name and explore the files listed in the directory.
    */

    int fd = open(path, O_RDONLY);

    if (fd < 0)
    {
        fprintf(2, "Cannot open: %s\n", path);
        return -1;
    }

    struct stat fileStat;

    if (fstat(fd, &fileStat) < 0)
    {
        fprintf(2, "Can't stat: %d\n", fd);
        close(fd);
        return -1;
    }

    switch (fileStat.type)
    {
        case T_DEVICE:
            break;

        case T_FILE:
            break;

        case T_DIR:
            // Get the files in the directory and run findFile() for them

            struct dirent dirEntry; // sizeof(struct dirent) == 16

            while (read(fd, &dirEntry, sizeof(struct dirent)) == sizeof(struct dirent))
            {
                // Skip empty inums to prevent infinite recursion while reading directory contents
                if (dirEntry.inum == 0)
                {
                    continue;
                }

                // Add a null char at the end of the entry name
                char dirEntryName[DIRSIZ + 1];
                memmove(dirEntryName, dirEntry.name, DIRSIZ);
                dirEntryName[DIRSIZ] = '\0';

                if ((strcmp(dirEntryName, ".") != 0) && (strcmp(dirEntryName, "..") != 0))
                {
                    // Append the name to the path and perform recursive call to findFile()
                    char dirEntryPath[512];

                    if ((strlen(path) + strlen(dirEntryName) + 2) < sizeof(dirEntryPath))
                    {
                        strcpy(dirEntryPath, path);
                        dirEntryPath[strlen(path)] = '/';
                        char* tempPtr = dirEntryPath + strlen(path) + 1; // points to char to the right of '/'
                        strcpy(tempPtr, dirEntryName);

                        if (strcmp(dirEntryName, fileName) == 0)
                        {
                            // Support for exec option
                            if (execOption)
                            {
                                int pid= fork();

                                if (pid > 0)
                                {
                                    // Wait for the child to finish up
                                    wait(0);
                                }
                                else if (pid == 0)
                                {
                                    // Start the new program
                                    char* execArgv[] = {execCmd, cmdArg, dirEntryPath, 0};
                                    exec(execCmd, execArgv);
                                }
                                else
                                {
                                    fprintf(2, "Fork failed");
                                    return -1;
                                }
                            }
                            else
                            {
                                // Simply print the path
                                fprintf(2, "%s\n", dirEntryPath);
                            }
                        }

                        findFile(dirEntryPath, fileName);
                    }
                }
            }

            break;

        default:
            fprintf(2, "Incorrect file type!!");
            return -1;
    }

    close(fd);
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        fprintf(2, "Need at least 2 arguments but passed %d\n", argc-1);
        return -1;
    }
    else if(argc > 3)
    {
        // Only process exec
        if (strcmp(argv[3], "-exec") == 0)
        {
            execOption = true;

            if ((argc - 4) <= 0)
            {
                fprintf(2, "Missing the actual command\n");
                return -1;
            }
            else
            {
                execCmd = argv[4];

                // Build part of second arg to execCmd
                // In other words, stich the args into one arg
                {
                    char* currChar = cmdArg;

                    for (int i = 5; i < argc; ++i)  // Args start at argv[5]
                    {
                        // TODO: Check for overflow before these actions
                        strcpy(currChar, argv[i]);
                        currChar += strlen(argv[i]);

                        // Overwrite the copied '\0' with ' ' while stitching the args
                        // Note that we still need the arg to be null terminated
                        if (i < (argc - 1))
                        {
                            *(currChar) = ' ';
                        }

                        ++currChar;
                    }
                }
            }
        }
    }

    return findFile(argv[1], argv[2]);
}