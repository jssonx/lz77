#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#define HASH_SIZE 65535
#define MAX_OFFSET 65535
#define MAX_LEN 255
#define MAX_LIT 255
#define DECOMPRESSION_BUFFER_MULTIPLIER 10

typedef struct
{
    int *hash;
    char *input;
    char *output;
    int inputLength;
    int outputLength;
    int outputIndex;
    int literalCount;
} LZ77Context;

// Update the hash table with the current index
void UpdateHash(LZ77Context *ctx, int idx)
{
    if (idx + 2 < ctx->inputLength)
    {
        ctx->hash[((ctx->input[idx] & 0xFF) << 8) | ((ctx->input[idx + 1] & 0xFF) ^ (ctx->input[idx + 2] & 0xFF))] = idx;
    }
}

// Find a matching string in the input data
int FindMatch(LZ77Context *ctx, int idx, int *off)
{
    int len = 1;
    *off = idx - ctx->hash[((ctx->input[idx] & 0xFF) << 8) | ((ctx->input[idx + 1] & 0xFF) ^ (ctx->input[idx + 2] & 0xFF))];
    if (*off > 0 && *off < MAX_OFFSET && ctx->input[idx - *off] == ctx->input[idx])
    {
        while (idx + len < ctx->inputLength && ctx->input[idx - *off + len] == ctx->input[idx + len] && len < MAX_LEN)
        {
            len++;
        }
    }
    return len;
}

// Write a literal block to the output
void WriteLiteral(LZ77Context *ctx, int idx)
{
    int max = ctx->literalCount > MAX_LIT ? MAX_LIT : ctx->literalCount;
    ctx->literalCount -= max;
    if (ctx->output)
    {
        ctx->output[ctx->outputIndex++] = max;
        memcpy(ctx->output + ctx->outputIndex, ctx->input + idx - ctx->literalCount - max, max);
        ctx->outputIndex += max;
    }
    else
    {
        ctx->outputIndex += 1 + max;
    }
}

// Write a compressed block to the output
void WriteCompressedBlock(LZ77Context *ctx, int len, int off)
{
    if (ctx->output)
    {
        ctx->output[ctx->outputIndex++] = 0x80 | len;
        ctx->output[ctx->outputIndex++] = off >> 8;
        ctx->output[ctx->outputIndex++] = off & 0xFF;
    }
    else
    {
        ctx->outputIndex += 3;
    }
}

// LZ77 encoding function
int LZ77Encode(LZ77Context *ctx)
{
    int idx = 0, len, off;
    ctx->outputIndex = 0;
    ctx->literalCount = 0;

    while (idx <= ctx->inputLength)
    {
        if (idx + 2 < ctx->inputLength)
        {
            len = FindMatch(ctx, idx, &off);
        }
        else
        {
            len = 1;
        }

        if (len > 3 || idx == ctx->inputLength)
        {
            while (ctx->literalCount)
            {
                WriteLiteral(ctx, idx);
            }
        }

        if (len > 2 && ctx->literalCount == 0)
        {
            WriteCompressedBlock(ctx, len, off);
        }
        else
        {
            ctx->literalCount += len;
        }

        for (int i = 0; i < len; ++i)
        {
            UpdateHash(ctx, idx++);
        }
    }

    return ctx->outputIndex;
}

// LZ77 decoding function
int LZ77Decode(LZ77Context *ctx)
{
    int inIdx = 0, len, offset;
    ctx->outputIndex = 0;

    while (inIdx < ctx->inputLength)
    {
        len = ctx->input[inIdx++] & 0xFF;

        if (len > 127)
        {
            len &= 0x7F;
            offset = ((ctx->input[inIdx] & 0xFF) << 8) | (ctx->input[inIdx + 1] & 0xFF);
            inIdx += 2;

            for (int i = 0; i < len; ++i)
            {
                if (ctx->output)
                {
                    ctx->output[ctx->outputIndex] = ctx->output[ctx->outputIndex - offset];
                }
                ctx->outputIndex++;
            }
        }
        else
        {
            for (int i = 0; i < len; ++i)
            {
                if (ctx->output)
                {
                    ctx->output[ctx->outputIndex] = ctx->input[inIdx];
                }
                ctx->outputIndex++;
                inIdx++;
            }
        }
    }

    return ctx->outputIndex;
}

// Load file into a dynamically allocated buffer
long LoadFile(const char *fileName, char **buffer)
{
    FILE *file = fopen(fileName, "rb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s: %s\n", fileName, strerror(errno));
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char *)malloc(length);
    if (!*buffer)
    {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return -1;
    }

    size_t bytesRead = fread(*buffer, 1, length, file);
    if (bytesRead != length)
    {
        fprintf(stderr, "Failed to read the file %s\n", fileName);
        free(*buffer);
        fclose(file);
        return -1;
    }

    fclose(file);
    return length;
}

// Save buffer to file
int SaveFile(const char *fileName, const char *buffer, int count)
{
    FILE *file = fopen(fileName, "wb");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s for writing: %s\n", fileName, strerror(errno));
        return -1;
    }

    fwrite(buffer, 1, count, file);
    fclose(file);
    return count;
}

// Handle compression
int CompressFile(const char *inputFileName, const char *outputFileName)
{
    LZ77Context ctx;
    ctx.inputLength = LoadFile(inputFileName, &ctx.input);
    if (ctx.inputLength < 0)
        return -1;

    clock_t start, end;
    double cpu_time_used, speed;

    start = clock();

    ctx.hash = calloc(HASH_SIZE, sizeof(int));
    if (!ctx.hash)
    {
        free(ctx.input);
        fprintf(stderr, "Hash allocation failed\n");
        return -1;
    }

    ctx.output = NULL;
    ctx.outputLength = ctx.inputLength;
    ctx.output = (char *)malloc(ctx.outputLength);
    if (!ctx.output)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(ctx.input);
        free(ctx.hash);
        return -1;
    }

    int outputLength = LZ77Encode(&ctx);
    SaveFile(outputFileName, ctx.output, outputLength);

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    speed = (ctx.inputLength / (1024.0 * 1024.0)) / cpu_time_used; // MB/s

    printf("raw size: %d\n", ctx.inputLength);
    printf("lz77 size: %d\n", outputLength);
    printf("ratio: %.2f\n", (double)ctx.inputLength / outputLength);
    printf("speed: %.2f MB/s\n", speed);

    free(ctx.input);
    free(ctx.output);
    free(ctx.hash);
    return 0;
}

// Handle decompression
int DecompressFile(const char *inputFileName, const char *outputFileName)
{
    LZ77Context ctx;

    ctx.inputLength = LoadFile(inputFileName, &ctx.input);
    if (ctx.inputLength < 0)
    {
        fprintf(stderr, "Failed to load input file\n");
        return -1;
    }

    clock_t start, end;
    double cpu_time_used, speed;

    start = clock();

    ctx.output = (char *)malloc(ctx.inputLength * DECOMPRESSION_BUFFER_MULTIPLIER);
    if (!ctx.output)
    {
        fprintf(stderr, "Memory allocation for output failed\n");
        free(ctx.input);
        return -1;
    }

    ctx.outputLength = LZ77Decode(&ctx);

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    speed = (ctx.outputLength / (1024.0 * 1024.0)) / cpu_time_used; // MB/s

    printf("lz77 size: %d\n", ctx.inputLength);
    printf("raw size: %d\n", ctx.outputLength);
    printf("speed: %.2f MB/s\n", speed);

    if (SaveFile(outputFileName, ctx.output, ctx.outputLength) < 0)
    {
        fprintf(stderr, "Failed to save output file\n");
        free(ctx.input);
        free(ctx.output);
        return -1;
    }

    free(ctx.input);
    free(ctx.output);
    return 0;
}

// Print usage information
void PrintUsage(const char *programName)
{
    fprintf(stderr, "Usage for compression: %s -encode [input file] [output file]\n", programName);
    fprintf(stderr, "Usage for decompression: %s -decode [input file] [output file]\n", programName);
}

// Main function
int main(int argc, char **argv)
{
    if (argc == 4 && strcmp(argv[1], "-encode") == 0)
    {
        // Compression with -encode flag
        char *inputFile = argv[2];
        char *outputFile = argv[3];
        return CompressFile(inputFile, outputFile) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else if (argc == 4 && strcmp(argv[1], "-decode") == 0)
    {
        // Decompression with -decode flag
        char *inputFile = argv[2];
        char *outputFile = argv[3];
        return DecompressFile(inputFile, outputFile) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    else
    {
        // Incorrect arguments
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }
}
