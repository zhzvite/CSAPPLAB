#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
int h, v, s, E, b, S;
int hit_count, miss_count, eviction_count;
char t[1000];
typedef struct
{
    int valid_bits;
    int tag;
    int stamp;
} cache_line, *cache_f, **cache;
cache cache_ = NULL;
void printUsage()
{
    printf("Fuck you Vite ,Do it quickly ,Don't play game! OK?\n"
           "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           "  -h         Print this help message.\n"
           "  -v         Optional verbose flag.\n"
           "  -s <num>   Number of set index bits.\n"
           "  -E <num>   Number of lines per set.\n"
           "  -b <num>   Number of block offset bits.\n"
           "  -t <file>  Trace file.\n\n"
           "Examples:\n"
           "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
           "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_cache()
{
    cache_ = (cache)malloc(sizeof(cache_f) * S);
    for (int i = 0; i < S; i++)
    {
        cache_[i] = (cache_f)malloc(sizeof(cache_line) * E);
        for (int j = 0; j < E; j++)
        {
            cache_[i][j].valid_bits = 0;
            cache_[i][j].tag = -1;
            cache_[i][j].stamp = -1;
        }
    }
}
void update(unsigned int address)
{
    unsigned set_address = ((address >> b) & ((-1U) >> (64 - s)));
    int cache_tag = address >> (s + b);
    //
    for (int i = 0; i < E; i++)
    {
        if (cache_[set_address][i].tag == cache_tag)
        {
            hit_count++;
            cache_[set_address][i].stamp = -1;
            return;
        }
    }
    //
    for (int i = 0; i < E; i++)
    {
        if (cache_[set_address][i].valid_bits == 0)
        {
            miss_count++;
            cache_[set_address][i].valid_bits = 1;
            cache_[set_address][i].stamp = 0;
            cache_[set_address][i].tag = cache_tag;
            return;
        }
    }
    //
    int max_stamp = -1;
    int waste = -1;
    miss_count++;
    eviction_count++;
    for (int i = 0; i < E; i++)
    {
        if (cache_[set_address][i].stamp > max_stamp)
        {
            max_stamp = cache_[set_address][i].stamp;
            waste = i;
        }
    }
    cache_[set_address][waste].stamp = 0;
    cache_[set_address][waste].tag = cache_tag;
}
void update_stamp()
{
    for (int i = 0; i < S; i++)
    {
        for (int j = 0; j < E; j++)
            if (cache_[i][j].valid_bits == 1)
                ++cache_[i][j].stamp;
    }
}
void simulate_cache()
{
    FILE *fp = NULL;
    fp = fopen(t, "r");
    if (fp == NULL)
    {
        printf("Fuck you Open error!!!");
        exit(-1);
    }
    char operation;
    unsigned int address;
    int size;
    while (fscanf(fp, "%c %xu,%d\n", &operation, &address, &size) > 0)
    {
        switch (operation)
        {
        case 'L':
            update(address);
            break;
        case 'M':
            update(address);
        case 'S':
            update(address);
            break;
        }
        update_stamp();
    }
    fclose(fp);
    for (int i = 0; i < S; i++)
        free(cache_[i]);
    free(cache_);
}
void sf(int argc,char ** argv){
    h = 0;
    v = 0;
    int opt;
    // 拆分输入的规定
    while (-1 != (opt = (getopt(argc, argv, "hvs:E:b:t:"))))
    {
        switch (opt)
        {
        case 'h':
            h = 1;
            printUsage();
            break;
        case 'v':
            v = 1;
            printUsage();
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            strcpy(t, optarg);
            break;
        default:
            printUsage();
            break;
        }
    }
    if (s <= 0 || E <= 0 || b <= 0 || t == NULL)
        exit(-1);
    S = 1 << s;
}
int main(int argc, char **argv)
{
    sf (argc,argv);
    init_cache();
    simulate_cache();
    printSummary(hit_count, miss_count, eviction_count);
}