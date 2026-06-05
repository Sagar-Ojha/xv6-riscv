#include <stdbool.h>
#include <stddef.h>

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        //fprintf(fp, "format", arg1, ...)
        fprintf(2, "Need at least 1 argument but passed %d\n", argc-1);
        return -1;
    }

    for (int i = 1; i < argc; ++i)
    {
        int fd= open(argv[i], O_RDONLY);

        if (fd == -1)
        {
            fprintf(2, "open() failed for %s\n", argv[i]);
            return -1;
        }

        // Store the data into a string buffer and then process it
        int bufSize = 512;
        char dataBuf[bufSize + 1];
        const char delimiters[] = {' ', '-', '\r', '\t', '\n', '.', '/', ','};

        /*
            Because of the way I initially thought about this problem, some simplyfying
            assumptions are made. One of the major assumptions is that the contents of the
            file will fit in bufSize. Without this assumption, I'll have to tackle a whole
            different beast just to parse and collect characters and I don't want to lose
            any hair follicles over this assignment. You'll run into theoretically
            unsolvable edge cases along with practically annoying ones without this assumption.
            Another assumption is that the data doesn't have a null-character. Hence, I can
            append a null-character at the end after reading off the file and make the
            string null-terminated.

            Having said that, had I started tackling this problem by reading file from the disk
            character-by-character, then it'd have been a lot more easier and straightforware tbh.
        */

        int totalBytesRead = read(fd, dataBuf, bufSize);

        if (totalBytesRead == -1)
        {
            fprintf(2, "read() failed");
            return -1;
        }

        dataBuf[totalBytesRead] = '\0';

        // Start and end of the chunks of data
        char* start = dataBuf;
        char* end = dataBuf;

        while (end != &dataBuf[totalBytesRead])
        {
            // If the delimiters aren't found, then end will point to the null character
            end = &dataBuf[totalBytesRead];

            // End of the chunk is marked by the closest delimiter
            int closestDelimiterIndex = bufSize;

            for (int i = 0; i < sizeof(delimiters); ++i)
            {
                char* delimiterPtr = strchr(start, delimiters[i]);

                if (delimiterPtr != NULL)
                {
                    int delimiterIndex = delimiterPtr - dataBuf;

                    if (delimiterIndex < closestDelimiterIndex)
                    {
                        closestDelimiterIndex = delimiterIndex;
                        end = delimiterPtr;
                    }
                }
            }

            // Discard the chunk if there are any invalid char
            char* currChar = start;
            bool isValidChunk = true;

            while ((currChar != end) && (isValidChunk))
            {
                if ((*currChar < 48) || (*currChar > 57))
                {
                    isValidChunk = false;
                }

                if (isValidChunk) { currChar++; }
            }

            if ((isValidChunk) && (start != end))
            {
                char chunkBuf[end - start + 1];
                memcpy(chunkBuf, start, end - start);
                chunkBuf[end - start] = '\0'; // Necessary for atoi to work properly

                // Check if the chunk is divisible by 5 or 6
                int num = atoi(chunkBuf);

                if (((num % 5) == 0) || ((num % 6) == 0))
                {
                    fprintf(2, "%d\n", num);
                }
            }

            start = end + 1;
        }

        close(fd);
    }

    return 0;
}
