#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

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
                            fprintf(2, "%s\n", dirEntryPath);
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
    if (argc != 3)
    {
        fprintf(2, "Need exactly 2 arguments but passed %d\n", argc-1);
        return -1;
    }

    return findFile(argv[1], argv[2]);
}