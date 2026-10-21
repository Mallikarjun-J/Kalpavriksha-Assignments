#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_EMPLOYEES 100

struct Employee
{
    int id;
    char name[50];
    char designation[50];
    struct Employee *reports[MAX_EMPLOYEES];
    int reportCount;
    struct Employee *supervisor;
};

// Search for an employee by ID in the array
struct Employee *findEmployee(struct Employee employees[], int employeeId)
{
    for (int i = 0; i < MAX_EMPLOYEES; i++)
    {
        if (employees[i].id != 0 && employees[i].id == employeeId)
        {
            return &employees[i];
        }
    }
    return NULL;
}

// Only one CEO is allowed in the organization
bool ceoExists(struct Employee employees[])
{
    for (int i = 0; i < MAX_EMPLOYEES; i++)
    {
        if (employees[i].id != 0 && strcmp(employees[i].designation, "CEO") == 0)
        {
            return true;
        }
    }
    return false;
}

// Find first free slot and link employee with supervisor
struct Employee *createEmployee(struct Employee employees[], int *employeeCount, int employeeId, char employeeName[], char designation[], struct Employee *supervisor)
{
    // Step 1: Find the first available empty slot in the array
    int slotIndex;
    for (slotIndex = 0; slotIndex < MAX_EMPLOYEES; slotIndex++)
    {
        if (employees[slotIndex].id == 0)
        {
            break;
        }
    }

    if (slotIndex == MAX_EMPLOYEES)
    {
        return NULL;
    }

    if (supervisor != NULL && supervisor->reportCount >= MAX_EMPLOYEES)
    {
        return NULL;
    }

    // Step 2: Populate employee record
    struct Employee *newEmployee = &employees[slotIndex];
    newEmployee->id = employeeId;
    strcpy(newEmployee->name, employeeName);
    strcpy(newEmployee->designation, designation);
    newEmployee->reportCount = 0;
    newEmployee->supervisor = supervisor;

    // Step 3: Link employee as a direct report to supervisor
    if (supervisor != NULL)
    {
        supervisor->reports[supervisor->reportCount++] = newEmployee;
    }

    (*employeeCount)++;
    return newEmployee;
}

void addEmployee(struct Employee employees[], int *employeeCount)
{
    // Step 1: Check list capacity
    if (*employeeCount >= MAX_EMPLOYEES)
    {
        printf("Employee list is full.\n");
        return;
    }

    int employeeId;
    char employeeName[50];
    char designation[50];
    int supervisorId;
    struct Employee *supervisor = NULL;

    // Step 2: Take and validate unique employee ID
    do
    {
        printf("Enter Employee ID: ");
        if (scanf("%d", &employeeId) != 1)
        {
            printf("Invalid input. Employee not added.\n");
            return;
        }

        if (employeeId <= 0)
        {
            printf("Employee ID must be valid.\n");
        }
        else if (findEmployee(employees, employeeId) != NULL)
        {
            printf("Employee ID already exists.\n");
        }
    } while (employeeId <= 0 || findEmployee(employees, employeeId) != NULL);

    // Step 3: Clear leftover newline and read name & designation
    int character;
    while ((character = getchar()) != '\n' && character != EOF);

    printf("Enter Employee Name: ");
    if (fgets(employeeName, sizeof(employeeName), stdin) == NULL)
    {
        printf("Invalid name. Employee not added.\n");
        return;
    }
    employeeName[strcspn(employeeName, "\n")] = '\0';

    printf("Enter Employee Designation: ");
    if (fgets(designation, sizeof(designation), stdin) == NULL)
    {
        printf("Invalid designation. Employee not added.\n");
        return;
    }
    designation[strcspn(designation, "\n")] = '\0';

    // Step 4: Validate supervisor (CEO has no supervisor; others must have one)
    if (strcmp(designation, "CEO") == 0)
    {
        if (ceoExists(employees))
        {
            printf("A CEO already exists.\n");
            return;
        }
    }
    else
    {
        do
        {
            printf("Enter Reporting Supervisor ID: ");
            if (scanf("%d", &supervisorId) != 1)
            {
                printf("Invalid input. Employee not added.\n");
                return;
            }
            // Cannot report to self
            if (supervisorId == employeeId)
            {
                supervisor = NULL;
                printf("An employee cannot report to themselves.\n");
            }
            else
            {
                supervisor = findEmployee(employees, supervisorId);
                if (supervisor == NULL)
                {
                    printf("Supervisor ID not found. Please try again.\n");
                }
            }
        } while (supervisor == NULL);
    }

    // Step 5: Create employee in array and link hierarchy
    if (createEmployee(employees, employeeCount, employeeId, employeeName, designation, supervisor) == NULL)
    {
        printf("Employee could not be added.\n");
        return;
    }
    printf("Employee added successfully.\n");
}

// Remove from supervisor's reports list and shift array left
void removeFromSupervisorList(struct Employee *supervisor, struct Employee *employee)
{
    if (supervisor == NULL)
    {
        return;
    }

    // Step 1: Find the employee in the supervisor's direct reports list
    for (int reportIndex = 0; reportIndex < supervisor->reportCount; reportIndex++)
    {
        if (supervisor->reports[reportIndex] == employee)
        {
            // Step 2: Shift remaining reports to the left to fill the gap
            for (int nextIndex = reportIndex; nextIndex < supervisor->reportCount - 1; nextIndex++)
            {
                supervisor->reports[nextIndex] = supervisor->reports[nextIndex + 1];
            }

            // Step 3: Decrement count and clear last pointer
            supervisor->reportCount--;
            supervisor->reports[supervisor->reportCount] = NULL;
            return;
        }
    }
}

// Pick sibling with the least reports under same supervisor
struct Employee *findSiblingWithFewestReports(struct Employee *employee)
{
    struct Employee *supervisor = employee->supervisor;
    struct Employee *siblingWithFewestReports = NULL;

    if (supervisor == NULL)
    {
        return NULL;
    }

    // Search among peers under the same supervisor for the one with least reports
    for (int reportIndex = 0; reportIndex < supervisor->reportCount; reportIndex++)
    {
        struct Employee *sibling = supervisor->reports[reportIndex];
        if (sibling == employee)
        {
            continue;
        }

        if (siblingWithFewestReports == NULL || sibling->reportCount < siblingWithFewestReports->reportCount)
        {
            siblingWithFewestReports = sibling;
        }
    }
    return siblingWithFewestReports;
}

void removeEmployee(struct Employee employees[], int *employeeCount)
{
    // Step 1: Check if there are any employees to remove
    if (*employeeCount == 0)
    {
        printf("No employees available.\n");
        return;
    }

    int employeeId;
    printf("Enter Employee ID to remove: ");
    if (scanf("%d", &employeeId) != 1)
    {
        printf("Invalid input.\n");
        return;
    }

    // Step 2: Search employee by ID and ensure it is not the CEO
    struct Employee *employeeToRemove = findEmployee(employees, employeeId);
    if (employeeToRemove == NULL)
    {
        printf("Employee not found.\n");
        return;
    }
    if (employeeToRemove->supervisor == NULL)
    {
        printf("The CEO cannot be removed.\n");
        return;
    }

    // Step 3: Select new supervisor (sibling with least reports, or fallback to supervisor)
    struct Employee *newSupervisor = findSiblingWithFewestReports(employeeToRemove);
    if (newSupervisor == NULL)
    {
        newSupervisor = employeeToRemove->supervisor;
    }

    // Step 4: Detach employee from supervisor and reassign all their direct reports
    removeFromSupervisorList(employeeToRemove->supervisor, employeeToRemove);

    for (int reportIndex = 0; reportIndex < employeeToRemove->reportCount; reportIndex++)
    {
        struct Employee *report = employeeToRemove->reports[reportIndex];
        newSupervisor->reports[newSupervisor->reportCount++] = report;
        report->supervisor = newSupervisor;
    }

    // Step 5: Clear employee record and decrement total count
    employeeToRemove->id = 0;
    employeeToRemove->name[0] = '\0';
    employeeToRemove->designation[0] = '\0';
    employeeToRemove->reportCount = 0;
    employeeToRemove->supervisor = NULL;
    (*employeeCount)--;
    printf("Employee removed successfully.\n");
}

// Count total subordinates recursively (sub-tree size)
int countSubordinates(struct Employee *employee)
{
    int totalSubordinates = 0;
    for (int reportIndex = 0; reportIndex < employee->reportCount; reportIndex++)
    {
        totalSubordinates += 1 + countSubordinates(employee->reports[reportIndex]);
    }
    return totalSubordinates;
}

// Print hierarchy tree with indentation (DFS traversal)
void displayHierarchy(struct Employee *employee, int indentLevel)
{
    // Step 1: Print spaces per indentation level
    for (int space = 0; space < indentLevel; space++)
    {
        printf("    ");
    }

    // Step 2: Print current employee info
    printf("%d - %s - %s\n", employee->id, employee->name, employee->designation);

    // Step 3: Recursively display direct reports
    for (int reportIndex = 0; reportIndex < employee->reportCount; reportIndex++)
    {
        displayHierarchy(employee->reports[reportIndex], indentLevel + 1);
    }
}

void displayEmployeeHierarchy(struct Employee employees[], int employeeCount)
{
    if (employeeCount == 0)
    {
        printf("No employees available.\n");
        return;
    }

    int selectedEmployeeId;
    printf("Enter Employee ID to display hierarchy: ");
    if (scanf("%d", &selectedEmployeeId) != 1)
    {
        printf("Invalid input.\n");
        return;
    }

    struct Employee *selectedEmployee = findEmployee(employees, selectedEmployeeId);
    if (selectedEmployee == NULL)
    {
        printf("Employee not found.\n");
        return;
    }

    printf("\n===== EMPLOYEE HIERARCHY =====\n");
    displayHierarchy(selectedEmployee, 0);
    printf("\nTotal Employees Under %d: %d\n", selectedEmployeeId, countSubordinates(selectedEmployee));
}

// Print complete tree starting from all top-level roots
void displayCompleteHierarchy(struct Employee employees[], int employeeCount)
{
    printf("\n===== ORGANIZATION HIERARCHY =====\n");
    if (employeeCount == 0)
    {
        printf("No employees available.\nTotal Employees: 0\n");
        return;
    }

    for (int i = 0; i < MAX_EMPLOYEES; i++)
    {
        if (employees[i].id != 0 && employees[i].supervisor == NULL)
        {
            displayHierarchy(&employees[i], 0);
        }
    }
    printf("\nTotal Employees: %d\n", employeeCount);
}

// Load default sample data given in problem statement, testing purpose only
bool initializeEmployees(struct Employee employees[], int *employeeCount)
{
    struct Employee *rahul = createEmployee(employees, employeeCount, 101, "Rahul", "CEO", NULL);
    if (rahul == NULL)
        return false;

    struct Employee *amit = createEmployee(employees, employeeCount, 102, "Amit", "Engineering Manager", rahul);
    if (amit == NULL)
        return false;

    struct Employee *priya = createEmployee(employees, employeeCount, 103, "Priya", "HR Manager", rahul);
    if (priya == NULL)
        return false;

    if (createEmployee(employees, employeeCount, 104, "Neha", "Developer", amit) == NULL)
        return false;
    if (createEmployee(employees, employeeCount, 105, "Raj", "Developer", amit) == NULL)
        return false;
    if (createEmployee(employees, employeeCount, 106, "Karan", "HR Executive", priya) == NULL)
        return false;

    return true;
}

int main()
{
    //Initialize storage and preload sample organization data
    struct Employee employees[MAX_EMPLOYEES] = {0};
    int employeeCount = 0;

    if (!initializeEmployees(employees, &employeeCount))
    {
        printf("Could not initialize employees.\n");
        return 1;
    }

    int choice = 0;
    do
    {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Exiting the program.\n");
            break;
        }

        switch (choice)
        {
            case 1:
                addEmployee(employees, &employeeCount);
                break;
            case 2:
                removeEmployee(employees, &employeeCount);
                break;
            case 3:
                displayEmployeeHierarchy(employees, employeeCount);
                break;
            case 4:
                displayCompleteHierarchy(employees, employeeCount);
                break;
            case 5:
                printf("Exiting the program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 5);

    return 0;
}