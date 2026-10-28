#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 10

// Fill N x N matrix with random intensity values between 0 and 255
void generateRandomMatrix(int *matrix, int matrixSize)
{
    for (int row = 0; row < matrixSize; row++)
    {
        for (int column = 0; column < matrixSize; column++)
        {
            *(matrix + row * matrixSize + column) = rand() % 256;
        }
    }
}

// Display matrix using only pointer arithmetic
void printMatrix(int *matrix, int matrixSize)
{
    for (int row = 0; row < matrixSize; row++)
    {
        for (int column = 0; column < matrixSize; column++)
        {
            printf("%4d", *(matrix + row * matrixSize + column));
        }
        printf("\n");
    }
}

// Rotate matrix 90 degrees clockwise
void rotateMatrix(int *matrix, int matrixSize)
{
    int *topLeft;
    int *topRight;
    int *bottomLeft;
    int *bottomRight;
    int savedValue;

    for (int row = 0; row < matrixSize / 2; row++)
    {
        for (int column = row; column < matrixSize - 1 - row; column++)
        {
            // Calculate the addresses of the four positions
            topLeft = matrix + row * matrixSize + column;
            topRight = matrix + column * matrixSize + (matrixSize - 1 - row);
            bottomLeft = matrix + (matrixSize - 1 - column) * matrixSize + row;
            bottomRight = matrix + (matrixSize - 1 - row) * matrixSize + (matrixSize - 1 - column);

            // Save the top-left value before overwriting it
            savedValue = *topLeft;

            // Move the bottom-left value to the top-left position
            *topLeft = *bottomLeft;

            // Move the bottom-right value to the bottom-left position
            *bottomLeft = *bottomRight;

            // Move the top-right value to the bottom-right position
            *bottomRight = *topRight;

            // Move the saved top-left value to the top-right position
            *topRight = savedValue;
        }
    }
}

// Apply the 3x3 smoothing filter in-place using O(N) extra space
void smoothMatrix(int *matrix, int matrixSize)
{
    int previousRow[MAX_SIZE]={0};
    int currentRow[MAX_SIZE];

    for (int row = 0; row < matrixSize; row++)
    {
        // Save the original values of the current row
        for (int column = 0; column < matrixSize; column++)
        {
            *(currentRow + column) = *(matrix + row * matrixSize + column);
        }

        // Calculate the average for each cell
        for (int column = 0; column < matrixSize; column++)
        {
            int sum = 0;
            int count = 0;

            for (int neighborRow = row - 1; neighborRow <= row + 1; neighborRow++)
            {
                if (neighborRow < 0 || neighborRow >= matrixSize)
                {
                    continue;
                }

                for (int neighborColumn = column - 1; neighborColumn <= column + 1; neighborColumn++)
                {
                    if (neighborColumn < 0 || neighborColumn >= matrixSize)
                    {
                        continue;
                    }

                    if (neighborRow < row)
                    {
                        sum += *(previousRow + neighborColumn);
                    }
                    else if (neighborRow == row)
                    {
                        sum += *(currentRow + neighborColumn);
                    }
                    else
                    {
                        sum += *(matrix + neighborRow * matrixSize + neighborColumn);
                    }

                    count++;
                }
            }

            // Replace the cell with its average
            *(matrix + row * matrixSize + column) = sum / count;
        }

        // Save the original current row for the next iteration
        for (int column = 0; column < matrixSize; column++)
        {
            *(previousRow + column) = *(currentRow + column);
        }
    }
}

int main()
{
    int matrixSize;

    printf("Enter matrix size (2-10): ");
    do
    {
        scanf("%d", &matrixSize);
        if (matrixSize < 2 || matrixSize > 10)
        {
            printf("Invalid size. Please enter a size between 2 and 10: ");
        }
    } while (matrixSize < 2 || matrixSize > 10);

    int matrix[MAX_SIZE * MAX_SIZE];

    srand((unsigned int)time(NULL));

    generateRandomMatrix(matrix, matrixSize);

    printf("\nOriginal Randomly Generated Matrix:\n");
    printMatrix(matrix, matrixSize);

    rotateMatrix(matrix, matrixSize);

    printf("\nMatrix after 90-degree Clockwise Rotation:\n");
    printMatrix(matrix, matrixSize);

    smoothMatrix(matrix, matrixSize);

    printf("\nMatrix after Applying Smoothing Filter:\n");
    printMatrix(matrix, matrixSize);

    return 0;
}