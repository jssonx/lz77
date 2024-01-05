#include <stdio.h>
#include <stdlib.h>

/*
 * Helper functions:
 * - createRandomFile: create a file of random bytes of a given size (in bytes)
 * - createRepeatAsFile: create a file of a given size (in bytes) with all bytes set to 'A'
 */

void createRandomFile(const char *filename, size_t size)
{
    FILE *fp;
    unsigned char byte;
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; i++)
    {
        byte = rand() % 256;
        fwrite(&byte, sizeof(byte), 1, fp);
    }

    fclose(fp);
}

void createRepeatAsFile(const char *filename, size_t size)
{
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; i++)
    {
        fputc('A', fp);
    }

    fclose(fp);
}

int main()
{
    // Optional: set the seed for rand()
    // srand((unsigned int)time(NULL));

    createRandomFile("./test_data/random.bin", 1000000);
    createRepeatAsFile("./test_data/repeat_As.txt", 1000000);

    return 0;
}