/**
 * @ Author: SmartPolarBear
 * @ Create Time: 2019-07-19 23:20:06
 * @ Modified by: SmartPolarBear
 * @ Modified time: 2019-07-20 23:03:05
 * @ Description:buddy allocator
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define LEFTCHILD(index) ((index)*2)
#define RIGHTCHILD(index) ((index)*2 + 1)
#define PARENT(index) (((index) == 0) ? (0) : (((int)(floor((((float)(index)) - 1.0) / 2.0)))))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define POWER_OF_2(x) (((x) != 0) && !((x) & ((x)-1)))

typedef struct buddy
{
    unsigned size;
    unsigned longest[1];
} buddy_t;

struct
{
    buddy_t *buddy;
} kmem;

uint32_t fixsize(uint32_t size)
{
    unsigned int v = (unsigned int)size;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
}

void buddyinit(void *vstart1, unsigned size)
{
    unsigned nodesize = 0;

    size = size / 2 / sizeof(unsigned int);
    printf("buddyinit:size=%d\n", size);

    if (size < 1 || !POWER_OF_2(size))
    {
        printf("buddyinit fault");
        return;
    }

    kmem.buddy = (buddy_t *)vstart1;
    kmem.buddy->size = size;
    nodesize = 2 * size;

    for (int i = 0; i < 2 * size - 1; i++)
    {
        if (POWER_OF_2(i + 1))
            nodesize /= 2;

        kmem.buddy->longest[i] = nodesize;
    }
}

uint32_t buddyalloc(uint32_t size)
{
    unsigned index = 0, nodeszie = 0, offset = 0;
    if (kmem.buddy == NULL)
    {
        printf("kmem.buddy NULL");
        return -1;
    }

    if (size <= 0)
        size = 1;
    else if (!POWER_OF_2(size))
        size = fixsize(size);

    if (kmem.buddy->longest[index] < size)
        return -1;

    for (nodeszie = kmem.buddy->size; nodeszie != size; nodeszie /= 2)
    {
        index = kmem.buddy->longest[LEFTCHILD(index)] >= size
                    ? LEFTCHILD(index)
                    : RIGHTCHILD(index);
    }

    kmem.buddy->longest[index] = 0;
    offset = (index + 1) * nodeszie - kmem.buddy->size;
    while (index)
    {
        index = PARENT(index);
        kmem.buddy->longest[index] =
            MAX(kmem.buddy->longest[LEFTCHILD(index)], kmem.buddy->longest[RIGHTCHILD(index)]);
    }

    return offset;
}

void buddyfree(uint32_t offset)
{
    unsigned node_size, index = 0;
    unsigned left_longest, right_longest;

    if (kmem.buddy == NULL || offset < 0 || offset > kmem.buddy->size)
    {
        printf("error free");
        return;
    }

    node_size = 1;
    index = offset + kmem.buddy->size - 1;

    for (; kmem.buddy->longest[index]; index = PARENT(index))
    {
        node_size *= 2;
        if (index == 0)
            return;
    }

    kmem.buddy->longest[index] = node_size;

    while (index)
    {
        index = PARENT(index);
        node_size *= 2;

        left_longest = kmem.buddy->longest[LEFTCHILD(index)];
        right_longest = kmem.buddy->longest[RIGHTCHILD(index)];

        if (left_longest + right_longest == node_size)
            kmem.buddy->longest[index] = node_size;
        else
            kmem.buddy->longest[index] = MAX(left_longest, right_longest);
    }
}

typedef struct teststruct
{
    int a;
    double b;
    float c;
    long d;
    int arr[10];
} teststruct_t;

int main()
{
    const int testsize = 10240;
    void *vstart1 = malloc(testsize), *vstart2 = malloc(testsize);

    printf("init memory block at %p,size=%d\n", vstart1, testsize);
    buddyinit(vstart1, (uint32_t)testsize);
    printf("init buddy.\n");

    //test alloc
    int off1 = buddyalloc(sizeof(teststruct_t));
    printf("alloc offset at %d\n", off1);

    teststruct_t *ps1 = (teststruct_t *)kmem.buddy->longest[off1];
    printf("alloc at %p\n", (void *)ps1);

    free(vstart1);
    free(vstart2);
    printf("freed memory blocks.\n");
    return 0;
}