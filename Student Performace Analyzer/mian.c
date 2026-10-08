#include <stdio.h>

#define MAX_STUDENTS 100

struct Student
{
    int rollNumber;
    char name[100];
    int marks[3];
};

//To Calculate the total marks of a student
int calculateTotal(struct Student student)
{
    return student.marks[0] + student.marks[1] + student.marks[2];
}

// To Calculate the average marks
float calculateAverage(int totalMarks)
{
    return totalMarks / 3.0f;
}

// To Determine the grade based on average marks
char calculateGrade(float averageMarks)
{
    if (averageMarks >= 85)
        return 'A';
    else if (averageMarks >= 70)
        return 'B';
    else if (averageMarks >= 50)
        return 'C';
    else if (averageMarks >= 35)
        return 'D';
    else
        return 'F';
}

// To Print performance stars based on the grade
void printPerformanceStars(char grade)
{
    switch (grade)
    {
        case 'A':
            printf("*****\n");
            break;

        case 'B':
            printf("****\n");
            break;

        case 'C':
            printf("***\n");
            break;

        case 'D':
            printf("**\n");
            break;

        default:
            break;
    }
}

// Recursively print roll numbers of all students
void printRollNumbers(struct Student students[], int index, int studentCount)
{
    if (index == studentCount) return;
    printf(" %d", students[index].rollNumber);
    printRollNumbers(students, index + 1, studentCount);
}

int main()
{
    int studentCount;
    struct Student students[MAX_STUDENTS];

    printf("Enter number of students: ");
    scanf("%d", &studentCount);

    // Check whether the number of students exceeds the limit
    if (studentCount > MAX_STUDENTS)
    {
        printf("Invalid number of students\n");
        return 0;
    }

    for (int i = 0; i < studentCount; i++)
    {
        printf("Enter roll number for student %d: ", i + 1);
        scanf("%d", &students[i].rollNumber);

        printf("Enter name for student %d: ", i + 1);
        scanf("%s", students[i].name);

        printf("Enter marks for student %d (3 subjects): ", i + 1);

        // Read marks for three subjects
        for (int j = 0; j < 3; j++)
        {
            scanf("%d", &students[i].marks[j]);
        }

        printf("\n");
    }

    // Calculate and display results for each student
    for (int i = 0; i < studentCount; i++)
    {
        int totalMarks = calculateTotal(students[i]);
        float averageMarks = calculateAverage(totalMarks);
        char grade = calculateGrade(averageMarks);

        printf("\n");
        printf("Roll : %d\n", students[i].rollNumber);
        printf("Name : %s\n", students[i].name);
        printf("Total : %d\n", totalMarks);
        printf("Average : %.2f\n", averageMarks);
        printf("Grade : %c\n", grade);

        // Display performance stars if the student has passed
        if (grade != 'F')
        {
            printf("Performance: ");
            printPerformanceStars(grade);
            printf("\n");
        }
    }

    printf("Roll numbers of all students (via recursion): ");
    printRollNumbers(students, 0, studentCount);
    printf("\n");

    return 0;
}