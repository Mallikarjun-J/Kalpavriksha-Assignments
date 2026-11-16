#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024

#define NODE_TYPE_FILE      0
#define NODE_TYPE_DIRECTORY 1

// One node for each free disk block
typedef struct FreeBlock
{
    int blockIndex;
    struct FreeBlock *next;
    struct FreeBlock *prev;
} FreeBlock;

// Files and directories share the same node structure
typedef struct FileNode
{
    char name[51];
    int nodeType;
    struct FileNode *parent;
    struct FileNode *children;
    struct FileNode *next;
    struct FileNode *prev;
    int *blockPointers;         // Disk block numbers used by this file
    int blockCount;
    int fileSize;
} FileNode;

// Holds the disk and the filesystem state
typedef struct VFS
{
    char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];
    FreeBlock *freeHead;
    FreeBlock *freeTail;
    FileNode *rootDirectory;
    FileNode *currentDirectory;
} VFS;

// Free all remaining nodes in the free block list
void freeFreeBlocks(VFS *vfs)
{
    FreeBlock *currentBlock = vfs->freeHead;

    while (currentBlock != NULL)
    {
        FreeBlock *nextBlock = currentBlock->next;
        free(currentBlock);
        currentBlock = nextBlock;
    }

    vfs->freeHead = NULL;
    vfs->freeTail = NULL;
}

// Create the root directory '/'
int initializeRoot(VFS *vfs)
{
    FileNode *rootNode = malloc(sizeof(FileNode));

    if (rootNode == NULL)
    {
        return 0;
    }

    strcpy(rootNode->name, "/");
    rootNode->nodeType = NODE_TYPE_DIRECTORY;
    rootNode->parent = NULL;
    rootNode->children = NULL;
    rootNode->next = NULL;
    rootNode->prev = NULL;
    rootNode->blockPointers = NULL;
    rootNode->blockCount = 0;
    rootNode->fileSize = 0;

    vfs->rootDirectory = rootNode;
    vfs->currentDirectory = rootNode;

    return 1;
}

// Add all disk blocks to the free list
int initializeFreeBlocks(VFS *vfs)
{
    vfs->freeHead = NULL;
    vfs->freeTail = NULL;

    for (int blockIndex = 0; blockIndex < NUM_BLOCKS; blockIndex++)
    {
        FreeBlock *newFreeBlock = malloc(sizeof(FreeBlock));

        if (newFreeBlock == NULL)
        {
            freeFreeBlocks(vfs);
            return 0;
        }

        newFreeBlock->blockIndex = blockIndex;
        newFreeBlock->next = NULL;
        newFreeBlock->prev = vfs->freeTail;

        if (vfs->freeHead == NULL)
        {
            vfs->freeHead = newFreeBlock;
        }
        else
        {
            vfs->freeTail->next = newFreeBlock;
        }

        vfs->freeTail = newFreeBlock;
    }

    return 1;
}

// Initialize the disk, free blocks, and root directory
int initializeVFS(VFS *vfs)
{
    vfs->freeHead = NULL;
    vfs->freeTail = NULL;
    vfs->rootDirectory = NULL;
    vfs->currentDirectory = NULL;

    memset(vfs->virtualDisk, 0, sizeof(vfs->virtualDisk));

    if (initializeFreeBlocks(vfs) == 0)
    {
        return 0;
    }

    if (initializeRoot(vfs) == 0)
    {
        freeFreeBlocks(vfs);
        return 0;
    }

    return 1;
}

// Create and initialize a file or directory node
FileNode* createNode(char nodeName[], int nodeType, FileNode *parentDirectory)
{
    FileNode *newNode = malloc(sizeof(FileNode));

    if (newNode == NULL)
    {
        return NULL;
    }

    strcpy(newNode->name, nodeName);
    newNode->nodeType = nodeType;
    newNode->parent = parentDirectory;
    newNode->children = NULL;
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->blockPointers = NULL;
    newNode->blockCount = 0;
    newNode->fileSize = 0;

    return newNode;
}

// Add the node to the parent's circular doubly linked list
void insertNode(FileNode *parentDirectory, FileNode *newNode)
{
    if (parentDirectory->children == NULL)
    {
        // First child points to itself in both directions
        newNode->next = newNode;
        newNode->prev = newNode;
        parentDirectory->children = newNode;
    }
    else
    {
        // Add the new node at the end of the circular list
        FileNode *lastChild = parentDirectory->children->prev;

        lastChild->next = newNode;
        newNode->prev = lastChild;
        newNode->next = parentDirectory->children;
        parentDirectory->children->prev = newNode;
    }
}

// Search the circular list for a matching name
FileNode* findNode(FileNode *parentDirectory, char targetName[])
{
    if (parentDirectory == NULL || parentDirectory->children == NULL)
    {
        return NULL;
    }

    FileNode *currentNode = parentDirectory->children;

    do
    {
        if (strcmp(currentNode->name, targetName) == 0)
        {
            return currentNode;
        }
        currentNode = currentNode->next;
    } while (currentNode != parentDirectory->children);

    return NULL;
}

// Create a directory in the current directory
int createDirectory(VFS *vfs, char directoryName[])
{
    if (vfs->currentDirectory == NULL)
    {
        return 0;
    }

    if (findNode(vfs->currentDirectory, directoryName) != NULL)
    {
        printf("Directory or file already exists!\n");
        return 0;
    }

    FileNode *newDirectory = createNode(directoryName, NODE_TYPE_DIRECTORY, vfs->currentDirectory);

    if (newDirectory == NULL)
    {
        return 0;
    }

    insertNode(vfs->currentDirectory, newDirectory);
    printf("Directory '%s' created successfully.\n", directoryName);

    return 1;
}

// Create an empty file in the current directory
int createFile(VFS *vfs, char fileName[])
{
    if (vfs->currentDirectory == NULL)
    {
        return 0;
    }

    if (findNode(vfs->currentDirectory, fileName) != NULL)
    {
        printf("File or directory already exists!\n");
        return 0;
    }

    FileNode *newFile = createNode(fileName, NODE_TYPE_FILE, vfs->currentDirectory);

    if (newFile == NULL)
    {
        return 0;
    }

    insertNode(vfs->currentDirectory, newFile);
    printf("File '%s' created successfully.\n", fileName);

    return 1;
}

// Print the path from the root to the current directory
void printWorkingDirectory(VFS *vfs)
{
    if (vfs->currentDirectory == vfs->rootDirectory)
    {
        printf("/\n");
        return;
    }

    FileNode *currentNode = vfs->currentDirectory;
    int directoryDepth = 0;

    // Walk up to root to count path depth
    while (currentNode != NULL && currentNode != vfs->rootDirectory)
    {
        directoryDepth++;
        currentNode = currentNode->parent;
    }

    FileNode **pathHierarchy = malloc(directoryDepth * sizeof(FileNode *));

    if (pathHierarchy == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    currentNode = vfs->currentDirectory;

    for (int i = 0; i < directoryDepth; i++)
    {
        pathHierarchy[i] = currentNode;
        currentNode = currentNode->parent;
    }

    // Print path from root down to current directory
    printf("/");
    for (int i = directoryDepth - 1; i >= 0; i--)
    {
        printf("%s", pathHierarchy[i]->name);
        if (i > 0)
        {
            printf("/");
        }
    }
    printf("\n");

    free(pathHierarchy);
}

// Move to the parent, root, or a child directory
int changeDirectory(VFS *vfs, char targetDirectoryName[])
{
    if (strcmp(targetDirectoryName, "..") == 0)
    {
        if (vfs->currentDirectory->parent != NULL)
        {
            vfs->currentDirectory = vfs->currentDirectory->parent;
        }
        printf("Moved to ");
        printWorkingDirectory(vfs);
        return 1;
    }

    if (strcmp(targetDirectoryName, "/") == 0)
    {
        vfs->currentDirectory = vfs->rootDirectory;
        printf("Moved to /\n");
        return 1;
    }

    FileNode *targetNode = findNode(vfs->currentDirectory, targetDirectoryName);

    if (targetNode == NULL)
    {
        printf("Directory not found!\n");
        return 0;
    }

    if (targetNode->nodeType != NODE_TYPE_DIRECTORY)
    {
        printf("Cannot enter a file!\n");
        return 0;
    }

    vfs->currentDirectory = targetNode;
    printf("Moved to ");
    printWorkingDirectory(vfs);

    return 1;
}

// Display the entries in the current directory
void listFiles(VFS *vfs)
{
    if (vfs->currentDirectory->children == NULL)
    {
        printf("(empty)\n");
        return;
    }

    FileNode *currentNode = vfs->currentDirectory->children;

    do
    {
        printf("%s", currentNode->name);
        if (currentNode->nodeType == NODE_TYPE_DIRECTORY)
        {
            printf("/");
        }
        printf("\n");
        currentNode = currentNode->next;
    } while (currentNode != vfs->currentDirectory->children);
}

// Delete a file and return its blocks to the free list
int deleteFile(VFS *vfs, char fileName[])
{
    FileNode *targetFile = findNode(vfs->currentDirectory, fileName);

    if (targetFile == NULL)
    {
        printf("File not found!\n");
        return 0;
    }

    if (targetFile->nodeType != NODE_TYPE_FILE)
    {
        printf("Cannot delete a directory using delete!\n");
        return 0;
    }

    // Append the file's blocks directly back to the free list
    for (int i = 0; i < targetFile->blockCount; i++)
    {
        FreeBlock *recycledBlock = malloc(sizeof(FreeBlock));

        if (recycledBlock == NULL)
        {
            break;
        }

        recycledBlock->blockIndex = targetFile->blockPointers[i];
        recycledBlock->next = NULL;
        recycledBlock->prev = vfs->freeTail;

        if (vfs->freeHead == NULL)
        {
            vfs->freeHead = recycledBlock;
        }
        else
        {
            vfs->freeTail->next = recycledBlock;
        }

        vfs->freeTail = recycledBlock;
    }

    // Remove the file from its parent's circular list
    if (targetFile->next == targetFile)
    {
        targetFile->parent->children = NULL;
    }
    else
    {
        targetFile->prev->next = targetFile->next;
        targetFile->next->prev = targetFile->prev;

        if (targetFile->parent->children == targetFile)
        {
            targetFile->parent->children = targetFile->next;
        }
    }

    free(targetFile->blockPointers);
    free(targetFile);
    printf("File deleted successfully.\n");

    return 1;
}

// Remove a directory only when it is empty
int removeDirectory(VFS *vfs, char directoryName[])
{
    FileNode *targetDirectory = findNode(vfs->currentDirectory, directoryName);

    if (targetDirectory == NULL)
    {
        printf("Directory not found!\n");
        return 0;
    }

    if (targetDirectory->nodeType != NODE_TYPE_DIRECTORY)
    {
        printf("This is not a directory!\n");
        return 0;
    }

    if (targetDirectory->children != NULL)
    {
        printf("Directory is not empty!\n");
        return 0;
    }

    if (targetDirectory->next == targetDirectory)
    {
        targetDirectory->parent->children = NULL;
    }
    else
    {
        targetDirectory->prev->next = targetDirectory->next;
        targetDirectory->next->prev = targetDirectory->prev;

        if (targetDirectory->parent->children == targetDirectory)
        {
            targetDirectory->parent->children = targetDirectory->next;
        }
    }

    free(targetDirectory);
    printf("Directory removed successfully.\n");

    return 1;
}

// Write new content to a file and update its allocated blocks
int writeFile(VFS *vfs, char fileName[], char content[])
{
    FileNode *targetFile = findNode(vfs->currentDirectory, fileName);

    if (targetFile == NULL)
    {
        printf("File not found!\n");
        return 0;
    }

    if (targetFile->nodeType != NODE_TYPE_FILE)
    {
        printf("Cannot write into a directory!\n");
        return 0;
    }

    int contentLength = strlen(content);
    int requiredBlocks = (contentLength + BLOCK_SIZE - 1) / BLOCK_SIZE;

    int availableBlocks = 0;
    FreeBlock *currentBlock = vfs->freeHead;

    while (currentBlock != NULL)
    {
        availableBlocks++;
        currentBlock = currentBlock->next;
    }

    if (requiredBlocks > availableBlocks + targetFile->blockCount)
    {
        printf("Not enough disk space!\n");
        return 0;
    }

    int *newPointers = NULL;

    if (requiredBlocks > 0)
    {
        newPointers = malloc(requiredBlocks * sizeof(int));
        if (newPointers == NULL)
        {
            return 0;
        }
    }

    // Return the file's old blocks directly to the free list
    for (int i = 0; i < targetFile->blockCount; i++)
    {
        FreeBlock *recycledBlock = malloc(sizeof(FreeBlock));

        if (recycledBlock == NULL)
        {
            break;
        }

        recycledBlock->blockIndex = targetFile->blockPointers[i];
        recycledBlock->next = NULL;
        recycledBlock->prev = vfs->freeTail;

        if (vfs->freeHead == NULL)
        {
            vfs->freeHead = recycledBlock;
        }
        else
        {
            vfs->freeTail->next = recycledBlock;
        }

        vfs->freeTail = recycledBlock;
    }

    free(targetFile->blockPointers);
    targetFile->blockPointers = newPointers;
    targetFile->blockCount = requiredBlocks;
    targetFile->fileSize = contentLength;

    // Take blocks from the free-list head and copy content to disk
    for (int i = 0; i < requiredBlocks; i++)
    {
        FreeBlock *allocatedBlock = vfs->freeHead;

        newPointers[i] = allocatedBlock->blockIndex;
        vfs->freeHead = allocatedBlock->next;

        if (vfs->freeHead != NULL)
        {
            vfs->freeHead->prev = NULL;
        }
        else
        {
            vfs->freeTail = NULL;
        }

        free(allocatedBlock);

        int byteOffset = i * BLOCK_SIZE;
        int remainingBytes = contentLength - byteOffset;
        int bytesToCopy = (remainingBytes < BLOCK_SIZE) ? remainingBytes : BLOCK_SIZE;

        memset(vfs->virtualDisk[newPointers[i]], 0, BLOCK_SIZE);
        memcpy(vfs->virtualDisk[newPointers[i]], content + byteOffset, bytesToCopy);
    }

    printf("Data written successfully (size=%d bytes).\n", contentLength);

    return 1;
}

// Read the file content from its allocated disk blocks
void readFile(VFS *vfs, char fileName[])
{
    FileNode *targetFile = findNode(vfs->currentDirectory, fileName);

    if (targetFile == NULL)
    {
        printf("File not found!\n");
        return;
    }

    if (targetFile->nodeType != NODE_TYPE_FILE)
    {
        printf("Cannot read a directory!\n");
        return;
    }

    if (targetFile->fileSize == 0)
    {
        printf("File is empty.\n");
        return;
    }

    int remainingBytes = targetFile->fileSize;

    for (int i = 0; i < targetFile->blockCount; i++)
    {
        int bytesToRead = (remainingBytes > BLOCK_SIZE) ? BLOCK_SIZE : remainingBytes;
        fwrite(vfs->virtualDisk[targetFile->blockPointers[i]], 1, bytesToRead, stdout);
        remainingBytes -= bytesToRead;
    }

    printf("\n");
}

// Display total, used, and free disk blocks
void displayDiskUsage(VFS *vfs)
{
    int freeBlockCount = 0;
    int usedBlockCount;
    FreeBlock *currentBlock = vfs->freeHead;

    while (currentBlock != NULL)
    {
        freeBlockCount++;
        currentBlock = currentBlock->next;
    }

    usedBlockCount = NUM_BLOCKS - freeBlockCount;
    float diskUsagePercentage = (usedBlockCount * 100.0f) / NUM_BLOCKS;

    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", usedBlockCount);
    printf("Free Blocks: %d\n", freeBlockCount);
    printf("Disk Usage: %.2f%%\n", diskUsagePercentage);
}

// Free all file, directory, and free-block nodes
void exitVFS(VFS *vfs)
{
    FileNode *currentNode = vfs->rootDirectory;

    while (currentNode != NULL)
    {
        if (currentNode->children != NULL)
        {
            currentNode = currentNode->children;
        }
        else
        {
            FileNode *parentNode = currentNode->parent;

            if (parentNode != NULL)
            {
                if (currentNode->next == currentNode)
                {
                    parentNode->children = NULL;
                }
                else
                {
                    currentNode->prev->next = currentNode->next;
                    currentNode->next->prev = currentNode->prev;
                    if (parentNode->children == currentNode)
                    {
                        parentNode->children = currentNode->next;
                    }
                }
            }

            free(currentNode->blockPointers);
            free(currentNode);
            currentNode = parentNode;
        }
    }

    freeFreeBlocks(vfs);
    vfs->rootDirectory = NULL;
    vfs->currentDirectory = NULL;

    printf("Memory released. Exiting program...\n");
}

int main(void)
{
    // Keep the large VFS structure on the heap
    VFS *vfs = malloc(sizeof(VFS));

    char input[2048];
    char command[20];
    char target[51];

    const char *commandList[] = {
        "mkdir", "create", "cd", "ls", "pwd",
        "delete", "rmdir", "read", "write", "df", "exit"
    };

    int commandCount = sizeof(commandList) / sizeof(commandList[0]);

    if (vfs == NULL)
    {
        printf("VFS memory allocation failed!\n");
        return 1;
    }

    if (initializeVFS(vfs) == 0)
    {
        printf("VFS initialization failed!\n");
        free(vfs);
        return 1;
    }

    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    printf("\n========== VFS COMMANDS ==========\n");
    printf("mkdir <name>        - Create directory\n");
    printf("create <name>       - Create file\n");
    printf("write <name> <data> - Write file content\n");
    printf("read <name>         - Read file content\n");
    printf("delete <name>       - Delete file\n");
    printf("rmdir <name>        - Remove empty directory\n");
    printf("ls                  - List files and directories\n");
    printf("cd <name>           - Change directory\n");
    printf("cd ..               - Go to parent directory\n");
    printf("cd /                - Go to root directory\n");
    printf("pwd                 - Show current path\n");
    printf("df                  - Show disk usage\n");
    printf("exit                - Exit VFS\n");
    printf("==================================\n\n");

    while (1)
    {
        if (vfs->currentDirectory == vfs->rootDirectory)
        {
            printf("/ > ");
        }
        else
        {
            printf("%s > ", vfs->currentDirectory->name);
        }
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            exitVFS(vfs);
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        // Read the command and its first argument
        int argumentCount = sscanf(input, "%19s %50s", command, target);
        if (argumentCount < 1)
        {
            continue;
        }

        int matchedIndex = -1;
        for (int i = 0; i < commandCount; i++)
        {
            if (strcmp(command, commandList[i]) == 0)
            {
                matchedIndex = i;
                break;
            }
        }

        switch (matchedIndex)
        {
            case 0:
                if (argumentCount == 2)
                {
                    createDirectory(vfs, target);
                }
                else
                {
                    printf("Enter directory name.\n");
                }
                break;

            case 1:
                if (argumentCount == 2)
                {
                    createFile(vfs, target);
                }
                else
                {
                    printf("Enter file name.\n");
                }
                break;

            case 2:
                if (argumentCount == 2)
                {
                    changeDirectory(vfs, target);
                }
                else
                {
                    printf("Enter directory name.\n");
                }
                break;

            case 3:
                listFiles(vfs);
                break;

            case 4:
                printWorkingDirectory(vfs);
                break;

            case 5:
                if (argumentCount == 2)
                {
                    deleteFile(vfs, target);
                }
                else
                {
                    printf("Enter file name.\n");
                }
                break;

            case 6:
                if (argumentCount == 2)
                {
                    removeDirectory(vfs, target);
                }
                else
                {
                    printf("Enter directory name.\n");
                }
                break;

            case 7:
                if (argumentCount == 2)
                {
                    readFile(vfs, target);
                }
                else
                {
                    printf("Enter file name.\n");
                }
                break;

            case 8:
            {
                if (argumentCount != 2)
                {
                    printf("Enter file name and content.\n");
                    break;
                }

                // Move past the command and file name to reach content
                char *content = strstr(input, target) + strlen(target);
                while (*content == ' ' || *content == '\t')
                {
                    content++;
                }

                if (*content == '\0')
                {
                    printf("Enter file content.\n");
                    break;
                }

                // Remove surrounding quotes if present
                if (*content == '"' || *content == '\'')
                {
                    char quote = *content;
                    content++;
                    char *closingQuote = strrchr(content, quote);
                    if (closingQuote != NULL)
                    {
                        *closingQuote = '\0';
                    }
                }

                writeFile(vfs, target, content);
                break;
            }

            case 9:
                displayDiskUsage(vfs);
                break;

            case 10:
                exitVFS(vfs);
                free(vfs);
                return 0;

            default:
                printf("Invalid command!\n");
        }
    }

    free(vfs);
    return 0;
}