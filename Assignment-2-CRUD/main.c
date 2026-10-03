#include <stdio.h>
#include <string.h>

#define MAX_USERS 100
#define NAME_SIZE 20

struct User
{
    int id;
    char name[NAME_SIZE];
    int age;
};

/* Check whether the user ID already exists */
int idExists(int id)
{
    FILE *file = fopen("users.txt", "r");
    struct User user;

    if (file == NULL)
        return 0;

    while (fscanf(file, "%d %19s %d", &user.id, user.name, &user.age) == 3)
    {
        if (user.id == id)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

/* Add a new user */
int addUser(int id, char name[], int age)
{
    if (idExists(id))
    {
        printf("User with ID %d already exists.\n", id);
        return 0;
    }

    FILE *file = fopen("users.txt", "a");

    if (file == NULL)
    {
        printf("Unable to open file.\n");
        return 0;
    }

    fprintf(file, "%d %s %d\n", id, name, age);
    fclose(file);
    return 1;
}

/* Display all users */
void displayUsers()
{
    FILE *file = fopen("users.txt", "r");
    struct User user;

    if (file == NULL)
    {
        printf("No records found.\n");
        return;
    }

    printf("\nID\tName\tAge\n");
    printf("-------------------\n");

    // Read the user's ID, name, and age; continue only when all three values are read successfully.
    while (fscanf(file, "%d %19s %d", &user.id, user.name, &user.age) == 3)
        printf("%d\t%s\t%d\n", user.id, user.name, user.age);

    fclose(file);
}

/* Update a user */
int updateUser(int id, char name[], int age)
{
    struct User users[MAX_USERS];
    FILE *file = fopen("users.txt", "r");
    int userCount = 0;
    int found = 0;
    int maxUsers = sizeof(users) / sizeof(users[0]);

    if (file == NULL)
    {
        printf("No records found.\n");
        return 0;
    }

    while (userCount < maxUsers &&
           fscanf(file, "%d %19s %d", &users[userCount].id, users[userCount].name, &users[userCount].age) == 3)
        userCount++;

    fclose(file);

    /* Find the user using the ID */
    for (int index = 0; index < userCount; index++)
    {
        if (users[index].id == id)
        {
            strcpy(users[index].name, name);
            users[index].age = age;
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("User with ID %d not found.\n", id);
        return 0;
    }

    /* Rewrite the file with updated data */
    file = fopen("users.txt", "w");

    if (file == NULL)
    {
        printf("Unable to open file.\n");
        return 0;
    }

    for (int index = 0; index < userCount; index++)
        fprintf(file, "%d %s %d\n",
                users[index].id,
                users[index].name,
                users[index].age);

    fclose(file);
    return 1;
}

/* Delete a user */
int deleteUser(int id)
{
    struct User users[MAX_USERS];
    FILE *file = fopen("users.txt", "r");
    int userCount = 0;
    int found = 0;
    int maxUsers = sizeof(users) / sizeof(users[0]);

    if (file == NULL)
    {
        printf("No records found.\n");
        return 0;
    }

    while (userCount < maxUsers &&
           fscanf(file, "%d %19s %d", &users[userCount].id, users[userCount].name, &users[userCount].age) == 3)
        userCount++;

    fclose(file);

    /* Find the user using the ID */
    for (int index = 0; index < userCount; index++)
    {
        if (users[index].id == id)
        {
            /* Shift remaining users one position left */
            for (int nextIndex = index; nextIndex < userCount - 1; nextIndex++)
                users[nextIndex] = users[nextIndex + 1];

            userCount--;
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("User with ID %d not found.\n", id);
        return 0;
    }

    /* Rewrite the file without the deleted user */
    file = fopen("users.txt", "w");

    if (file == NULL)
    {
        printf("Unable to open file.\n");
        return 0;
    }

    for (int index = 0; index < userCount; index++)
        fprintf(file, "%d %s %d\n",
                users[index].id,
                users[index].name,
                users[index].age);

    fclose(file);
    return 1;
}

int main()
{
    int choice, id, age;
    char name[NAME_SIZE];

    do
    {
        printf("\n===== USER MANAGEMENT =====\n");
        printf("1. Add User\n");
        printf("2. Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("\n============================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                printf("Enter ID: ");
                scanf("%d", &id);
                printf("Enter Name: ");
                scanf("%19s", name);
                printf("Enter Age: ");
                scanf("%d", &age);

                if (addUser(id, name, age))
                    printf("User added successfully.\n");
                break;

            case 2:
                displayUsers();
                break;

            case 3:
                printf("Enter ID to update: ");
                scanf("%d", &id);
                printf("Enter new name: ");
                scanf("%19s", name);
                printf("Enter new age: ");
                scanf("%d", &age);

                if (updateUser(id, name, age))
                    printf("User updated successfully.\n");
                break;

            case 4:
                printf("Enter ID to delete: ");
                scanf("%d", &id);

                if (deleteUser(id))
                    printf("User deleted successfully.\n");
                break;

            case 5:
                printf("Program ended.\n");
                break;

            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 5);

    return 0;
}

