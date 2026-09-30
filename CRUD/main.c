
#include <stdio.h>
#include <string.h>

struct User{
    int id;
    char name[20];
    int age;
};


/* Add a new user */
void addUser(int id, char name[], int age){
    FILE *fp = fopen("users.txt", "a");

    if (fp == NULL){
        printf("Unable to open file.\n");
        return;
    }

    fprintf(fp, "%d %s %d\n", id, name, age);

    fclose(fp);
}

/* Display all users */
void displayUsers(){
    FILE *fp = fopen("users.txt", "r");
    struct User user;

    if (fp == NULL){
        printf("No records found.\n");
        return;
    }

    printf("\nID\tName\tAge\n");
    printf("-------------------\n");

    while (fscanf(fp, "%d %s %d", &user.id, &user.name, &user.age) == 3){
        printf("%d\t%s\t%d\n", user.id, user.name, user.age);
    }

    fclose(fp);
}

/* Update a user using ID */
void updateUser(int id, char name[], int age){
    struct User users[100];
    FILE *fp;
    int count = 0;
    int found = 0;

    fp = fopen("users.txt", "r");

    if (fp == NULL){
        printf("No records found.\n");
        return;
    }

    while (count < 100 && fscanf(fp, "%d %s %d", &users[count].id, &users[count].name, &users[count].age) == 3) count++;
    
    fclose(fp);

    /* Find the user */
    for (int i = 0; i < count; i++){
        if (users[i].id == id){
            strcpy(users[i].name, name);
            users[i].age = age;
            found = 1;
            break;
        }
    }

    if (!found){
        printf("User with ID %d not found.\n", id);
        return;
    }

    /* Rewrite the file with updated data */
    fp = fopen("users.txt", "w");

    if (fp == NULL){
        printf("Unable to open file.\n");
        return;
    }

    for (int i = 0; i < count; i++){
        fprintf(fp, "%d %s %d\n", users[i].id, users[i].name, users[i].age);
    }

    fclose(fp);

    printf("User updated successfully.\n");
}

/* Delete a user using ID */
void deleteUser(int id){
    struct User users[100];
    FILE *fp;
    int count = 0;
    int found = 0;

    fp = fopen("users.txt", "r");

    if (fp == NULL){
        printf("No records found.\n");
        return;
    }

    while (count < 100 && fscanf(fp, "%d %s %d", &users[count].id, &users[count].name, &users[count].age) == 3) count++;
    

    fclose(fp);

    /* Find the user */
    for (int i = 0; i < count; i++){
        if (users[i].id == id){
            /* Shift remaining users one position left */
            for (int j = i; j < count - 1; j++){
                users[j] = users[j + 1];
            }

            count--;
            found = 1;
            break;
        }
    }

    if (!found){
        printf("User with ID %d not found.\n", id);
        return;
    }

    /* Rewrite the file without the deleted user */
    fp = fopen("users.txt", "w");

    if (fp == NULL){
        printf("Unable to open file.\n");
        return;
    }

    for (int i = 0; i < count; i++){
        fprintf(fp, "%d %s %d\n", users[i].id, users[i].name, users[i].age);
    }

    fclose(fp);

    printf("User deleted successfully.\n");
}

int main()
{
    int choice;
    int id;
    int age;
    char name[20];

    do{
        printf("\n===== USER MANAGEMENT =====\n");
        printf("1. Add User\n");
        printf("2. Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("\n============================\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice){
            case 1:
                printf("Enter ID: ");
                scanf("%d", &id);

                printf("Enter Name: ");
                scanf("%s", name);

                printf("Enter Age: ");
                scanf("%d", &age);

                addUser(id, name, age);
                printf("User added successfully.\n");
                break;

            case 2:
                displayUsers();
                break;

            case 3:
                printf("Enter ID to update: ");
                scanf("%d", &id);

                printf("Enter new name: ");
                scanf("%s", name);

                printf("Enter new age: ");
                scanf("%d", &age);

                updateUser(id, name, age);
                break;

            case 4:
                printf("Enter ID to delete: ");
                scanf("%d", &id);

                deleteUser(id);
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
