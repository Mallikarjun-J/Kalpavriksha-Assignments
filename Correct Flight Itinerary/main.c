
#include <stdio.h>
#include <string.h>

//I have not validated the input,
//I have completed the logic for finding the strating point where there are no incoming flights
//and also printing the itinary of the flights in order.
int main()
{
    int n;

    printf("Enter number of tickets: ");
    scanf("%d", &n);

    char source[1000][101], destination[1000][101];

    for (int i = 0; i < n; i++)
    {
        printf("Enter the source and destination: ");
        scanf("%s -> %s", source[i], destination[i]);
    }

    //Finding the starting point where there are no incoming flights
    int start = 0;
    for(int i = 0; i < n; i++)
    {
        int found = 0;
        for (int j = 0; j < n; j++)
        {
            if (strcmp(source[i], destination[j]) == 0)
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            start = i;
            break;
        }
    }

    printf("The starting point: %s\n", source[start]);

    //Printing the itinerary of the flights in order
    for (int i = 0; i < n; i++)
    {
        printf("%s->%s", source[start], destination[start]);

        if (i != n - 1)
            printf(", ");

        for (int j = 0; j < n; j++)
        {
            if (strcmp(destination[start], source[j]) == 0)
            {
                start = j;
                break;
            }
        }
    }

    return 0;
}
