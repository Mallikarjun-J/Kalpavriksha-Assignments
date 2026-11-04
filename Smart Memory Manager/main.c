#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STATUS_FREE 0
#define STATUS_ALLOCATED 1

#define STRATEGY_FIRST_FIT 1
#define STRATEGY_BEST_FIT  2
#define STRATEGY_WORST_FIT 3

// Structure representing an individual memory block
struct Block
{
    int allocationId;  // Unique ID for allocated block, -1 for free block
    int startOffset;   // Starting byte offset in the pool
    int size;          // Block size in bytes
    int status;        // STATUS_FREE or STATUS_ALLOCATED
};

// Helper function to safely expand the metadata array when full
void ensureCapacity(struct Block **blockList, int *blockCount, int *blockCapacity)
{
    if (*blockCount >= *blockCapacity)
    {
        *blockCapacity *= 2;
        *blockList = (struct Block *)realloc(*blockList, (*blockCapacity) * sizeof(struct Block));
    }
}

// Initializing the byte pool and initial free block metadata
int initializeMemory(int poolSize, unsigned char **memoryPool, struct Block **blockList, int *blockCount, int *blockCapacity, int *totalPoolSize)
{
    if (poolSize < 1 || poolSize > 1000000)
    {
        printf("Invalid memory pool size. Must be between 1 and 1,000,000 bytes.\n");
        return 0;
    }

    *memoryPool = (unsigned char *)malloc(poolSize);
    if (*memoryPool == NULL)
    {
        printf("Failed to allocate memory pool.\n");
        return 0;
    }

    *blockCapacity = 2;
    *blockList = (struct Block *)malloc((*blockCapacity) * sizeof(struct Block));
    if (*blockList == NULL)
    {
        printf("Failed to allocate metadata array.\n");
        free(*memoryPool);
        return 0;
    }

    // Entire pool starts as one big free block (for start of program)
    (*blockList)[0].allocationId = -1;
    (*blockList)[0].startOffset = 0;
    (*blockList)[0].size = poolSize;
    (*blockList)[0].status = STATUS_FREE;

    *blockCount = 1;
    *totalPoolSize = poolSize;

    return 1;
}

// Set active allocation strategy
bool setAllocationStrategy(int strategyChoice, int *currentStrategy)
{
    if (strategyChoice == STRATEGY_FIRST_FIT)
    {
        *currentStrategy = STRATEGY_FIRST_FIT;
        printf("Strategy set to First Fit.\n");
    }
    else if (strategyChoice == STRATEGY_BEST_FIT)
    {
        *currentStrategy = STRATEGY_BEST_FIT;
        printf("Strategy set to Best Fit.\n");
    }
    else if (strategyChoice == STRATEGY_WORST_FIT)
    {
        *currentStrategy = STRATEGY_WORST_FIT;
        printf("Strategy set to Worst Fit.\n");
    }
    else
    {
        printf("Invalid strategy. Choose 1, 2, or 3.\n");
        return false;
    }

    return true;
}

// Find a free block based on the current strategy
int findSuitableBlock(struct Block *blockList, int blockCount, int requestedSize, int strategy)
{
    int selectedIndex = -1;

    for (int i = 0; i < blockCount; i++)
    {
        if (blockList[i].status == STATUS_FREE && blockList[i].size >= requestedSize)
        {
            if (strategy == STRATEGY_FIRST_FIT)
            {
                return i;
            }
            else if (strategy == STRATEGY_BEST_FIT)
            {
                if (selectedIndex == -1 || blockList[i].size < blockList[selectedIndex].size)
                {
                    selectedIndex = i;
                }
            }
            else if (strategy == STRATEGY_WORST_FIT)
            {
                if (selectedIndex == -1 || blockList[i].size > blockList[selectedIndex].size)
                {
                    selectedIndex = i;
                }
            }
        }
    }

    return selectedIndex;
}

// Checks for external fragmentation when a request fails
void detectExternalFragmentation(struct Block *blockList, int blockCount, int requestedSize)
{
    int totalFreeBytes = 0;
    int largestFreeBlock = 0;

    for (int i = 0; i < blockCount; i++)
    {
        if (blockList[i].status == STATUS_FREE)
        {
            totalFreeBytes += blockList[i].size;
            if (blockList[i].size > largestFreeBlock)
            {
                largestFreeBlock = blockList[i].size;
            }
        }
    }

    if (totalFreeBytes >= requestedSize && largestFreeBlock < requestedSize)
    {
        printf("External fragmentation detected: %d bytes free in total, but largest block is %d bytes.\n", totalFreeBytes, largestFreeBlock);
        printf("Individual free blocks: ");
        for (int i = 0; i < blockCount; i++)
        {
            if (blockList[i].status == STATUS_FREE)
            {
                printf("%d ", blockList[i].size);
            }
        }
        printf("\n");
    }
}

// Allocate a block from the memory pool
int allocateMemory(struct Block **blockList, int *blockCount, int *blockCapacity, int requestedSize, int totalPoolSize, int strategy, int *nextAllocationId)
{
    if (requestedSize <= 0 || requestedSize > totalPoolSize)
    {
        printf("Invalid allocation size.\n");
        return -1;
    }

    int targetIndex = findSuitableBlock(*blockList, *blockCount, requestedSize, strategy);

    if (targetIndex == -1)
    {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        detectExternalFragmentation(*blockList, *blockCount, requestedSize);
        return -1;
    }

    int startOffset = (*blockList)[targetIndex].startOffset;
    int originalSize = (*blockList)[targetIndex].size;
    int assignedId = *nextAllocationId;

    // If free block is larger then split it
    if (originalSize > requestedSize)
    {
        ensureCapacity(blockList, blockCount, blockCapacity);

        for (int i = *blockCount; i > targetIndex + 1; i--)
        {
            (*blockList)[i] = (*blockList)[i - 1];
        }

        (*blockList)[targetIndex].allocationId = assignedId;
        (*blockList)[targetIndex].size = requestedSize;
        (*blockList)[targetIndex].status = STATUS_ALLOCATED;

        (*blockList)[targetIndex + 1].allocationId = -1;
        (*blockList)[targetIndex + 1].startOffset = startOffset + requestedSize;
        (*blockList)[targetIndex + 1].size = originalSize - requestedSize;
        (*blockList)[targetIndex + 1].status = STATUS_FREE;

        (*blockCount)++;
    }
    else
    {
        (*blockList)[targetIndex].allocationId = assignedId;
        (*blockList)[targetIndex].status = STATUS_ALLOCATED;
    }

    (*nextAllocationId)++;

    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n", assignedId, startOffset, requestedSize);

    return assignedId;
}

// Merge adjacent free blocks together
void mergeFreeBlocks(struct Block *blockList, int *blockCount)
{
    for (int i = 0; i < *blockCount - 1;)
    {
        if (blockList[i].status == STATUS_FREE && blockList[i + 1].status == STATUS_FREE)
        {
            blockList[i].size += blockList[i + 1].size;

            for (int j = i + 1; j < *blockCount - 1; j++)
            {
                blockList[j] = blockList[j + 1];
            }
            (*blockCount)--;
        }
        else
        {
            i++;
        }
    }
}

// Release allocated memory by ID
int releaseMemory(struct Block *blockList, int *blockCount, int targetId)
{
    int targetIndex = -1;

    for (int i = 0; i < *blockCount; i++)
    {
        if (blockList[i].allocationId == targetId && blockList[i].status == STATUS_ALLOCATED)
        {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex == -1)
    {
        printf("Invalid allocation ID or block already free.\n");
        return 0;
    }

    blockList[targetIndex].allocationId = -1;
    blockList[targetIndex].status = STATUS_FREE;

    mergeFreeBlocks(blockList, blockCount);

    printf("Allocation released.\n");
    return 1;
}

// Resize an existing allocation by ID
int resizeMemory(unsigned char *memoryPool, struct Block **blockList, int *blockCount, int *blockCapacity, int totalPoolSize, int targetId, int newRequestedSize, int strategy)
{
    if (newRequestedSize <= 0 || newRequestedSize > totalPoolSize)
    {
        printf("Invalid requested size.\n");
        return 0;
    }

    int targetIndex = -1;
    for (int i = 0; i < *blockCount; i++)
    {
        if ((*blockList)[i].allocationId == targetId && (*blockList)[i].status == STATUS_ALLOCATED)
        {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex == -1)
    {
        printf("Allocation ID not found.\n");
        return 0;
    }

    int currentStart = (*blockList)[targetIndex].startOffset;
    int currentSize = (*blockList)[targetIndex].size;

    if (newRequestedSize == currentSize)
    {
        printf("Allocation size is already unchanged.\n");
        return 1;
    }

    // case 1: Shrink
    if (newRequestedSize < currentSize)
    {
        ensureCapacity(blockList, blockCount, blockCapacity);

        for (int i = *blockCount; i > targetIndex + 1; i--)
        {
            (*blockList)[i] = (*blockList)[i - 1];
        }

        (*blockList)[targetIndex].size = newRequestedSize;

        (*blockList)[targetIndex + 1].allocationId = -1;
        (*blockList)[targetIndex + 1].startOffset = currentStart + newRequestedSize;
        (*blockList)[targetIndex + 1].size = currentSize - newRequestedSize;
        (*blockList)[targetIndex + 1].status = STATUS_FREE;

        (*blockCount)++;

        mergeFreeBlocks(*blockList, blockCount);
        printf("Allocation resized successfully.\n");
        return 1;
    }

    // case 2: Expand in place
    int extraNeeded = newRequestedSize - currentSize;

    if (targetIndex + 1 < *blockCount && (*blockList)[targetIndex + 1].status == STATUS_FREE && (*blockList)[targetIndex + 1].size >= extraNeeded)
    {
        (*blockList)[targetIndex].size = newRequestedSize;

        (*blockList)[targetIndex + 1].startOffset += extraNeeded;
        (*blockList)[targetIndex + 1].size -= extraNeeded;

        if ((*blockList)[targetIndex + 1].size == 0)
        {
            for (int i = targetIndex + 1; i < *blockCount - 1; i++)
            {
                (*blockList)[i] = (*blockList)[i + 1];
            }
            (*blockCount)--;
        }

        printf("Allocation expanded in place successfully.\n");
        return 1;
    }

    // case 3: Move to a free block
    int newIndex = findSuitableBlock(*blockList, *blockCount, newRequestedSize, strategy);

    if (newIndex == -1)
    {
        printf("Resize failed: no contiguous free block is large enough.\n");
        detectExternalFragmentation(*blockList, *blockCount, newRequestedSize);
        return 0;
    }

    int newStart = (*blockList)[newIndex].startOffset;
    int destinationSize = (*blockList)[newIndex].size;

    memmove(memoryPool + newStart, memoryPool + currentStart, currentSize);

    for (int i = 0; i < *blockCount; i++)
    {
        if ((*blockList)[i].startOffset == currentStart)
        {
            (*blockList)[i].allocationId = -1;
            (*blockList)[i].status = STATUS_FREE;
            break;
        }
    }

    (*blockList)[newIndex].allocationId = targetId;
    (*blockList)[newIndex].status = STATUS_ALLOCATED;
    (*blockList)[newIndex].size = newRequestedSize;

    if (destinationSize > newRequestedSize)
    {
        ensureCapacity(blockList, blockCount, blockCapacity);

        for (int i = *blockCount; i > newIndex + 1; i--)
        {
            (*blockList)[i] = (*blockList)[i - 1];
        }

        (*blockList)[newIndex + 1].allocationId = -1;
        (*blockList)[newIndex + 1].startOffset = newStart + newRequestedSize;
        (*blockList)[newIndex + 1].size = destinationSize - newRequestedSize;
        (*blockList)[newIndex + 1].status = STATUS_FREE;

        (*blockCount)++;
    }

    mergeFreeBlocks(*blockList, blockCount);

    printf("Allocation moved and resized successfully.\n");
    return 1;
}

// Display memory map
void displayMemoryMap(struct Block *blockList, int blockCount, int totalPoolSize)
{
    printf("\nMemory Map (%d bytes):\n", totalPoolSize);

    for (int i = 0; i < blockCount; i++)
    {
        if (blockList[i].status == STATUS_FREE)
        {
            printf("[Free: %d bytes, Start: %d]\n", blockList[i].size, blockList[i].startOffset);
        }
        else
        {
            printf("[Allocated #%d: %d bytes, Start: %d]\n", blockList[i].allocationId, blockList[i].size, blockList[i].startOffset);
        }
    }
}

// Display memory statistics
void displayMemoryStatistics(struct Block *blockList, int blockCount, int totalPoolSize)
{
    int totalAllocated = 0;
    int totalFree = 0;
    int allocatedBlocks = 0;
    int freeBlocks = 0;
    int largestFreeBlock = 0;

    for (int i = 0; i < blockCount; i++)
    {
        if (blockList[i].status == STATUS_ALLOCATED)
        {
            totalAllocated += blockList[i].size;
            allocatedBlocks++;
        }
        else
        {
            totalFree += blockList[i].size;
            freeBlocks++;

            if (blockList[i].size > largestFreeBlock)
            {
                largestFreeBlock = blockList[i].size;
            }
        }
    }

    printf("\nTotal memory: %d bytes\n", totalPoolSize);
    printf("Allocated: %d bytes\n", totalAllocated);
    printf("Free: %d bytes\n", totalFree);
    printf("Allocated blocks: %d\n", allocatedBlocks);
    printf("Free blocks: %d\n", freeBlocks);
    printf("Largest free block: %d bytes\n", largestFreeBlock);
}

int main()
{
    unsigned char *memoryPool = NULL;
    struct Block *blockList = NULL;

    int blockCount = 0;
    int blockCapacity = 0;
    int totalPoolSize = 0;

    int currentStrategy = STRATEGY_FIRST_FIT;
    int nextAllocationId = 1;

    int inputPoolSize;
    int strategyChoice;
    int menuChoice;
    int requestedSize;
    int targetId;
    int newRequestedSize;

    printf("Enter memory pool size: ");
    if (scanf("%d", &inputPoolSize) != 1)
    {
        printf("Invalid input.\n");
        return 1;
    }

    if (!initializeMemory(inputPoolSize, &memoryPool, &blockList, &blockCount, &blockCapacity, &totalPoolSize))
    {
        return 1;
    }

    do
    {
        printf("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
        if (scanf("%d", &strategyChoice) != 1)
        {
            printf("Invalid input.\n");
            free(memoryPool);
            free(blockList);
            return 1;
        }
    } while (!setAllocationStrategy(strategyChoice, &currentStrategy));

    do
    {
        printf("\nMenu:\n");
        printf("1. Allocate memory\n");
        printf("2. Release memory\n");
        printf("3. Resize allocation\n");
        printf("4. Display memory map\n");
        printf("5. Display memory statistics\n");
        printf("6. Change allocation strategy\n");
        printf("7. Exit\n");
        printf("Choice: ");

        if (scanf("%d", &menuChoice) != 1)
        {
            printf("Invalid input. Exiting...\n");
            break;
        }

        switch (menuChoice)
        {
            case 1:
                printf("Requested size: ");
                scanf("%d", &requestedSize);
                allocateMemory(&blockList, &blockCount, &blockCapacity, requestedSize, totalPoolSize, currentStrategy, &nextAllocationId);
                break;

            case 2:
                printf("Allocation ID: ");
                scanf("%d", &targetId);
                releaseMemory(blockList, &blockCount, targetId);
                break;

            case 3:
                printf("Allocation ID: ");
                scanf("%d", &targetId);
                printf("New size: ");
                scanf("%d", &newRequestedSize);
                resizeMemory(memoryPool, &blockList, &blockCount, &blockCapacity, totalPoolSize, targetId, newRequestedSize, currentStrategy);
                break;

            case 4:
                displayMemoryMap(blockList, blockCount, totalPoolSize);
                break;

            case 5:
                displayMemoryStatistics(blockList, blockCount, totalPoolSize);
                break;

            case 6:
                printf("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
                scanf("%d", &strategyChoice);
                setAllocationStrategy(strategyChoice, &currentStrategy);
                break;

            case 7:
                printf("Exiting Smart Memory Manager...\n");
                break;

            default:
                printf("Invalid choice. Please select from 1 to 7.\n");
                break;
        }
    } while (menuChoice != 7);

    free(memoryPool);
    free(blockList);

    return 0;
}