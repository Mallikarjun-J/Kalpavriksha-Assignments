#include <stdio.h>
#include <ctype.h>
#include <string.h>

int calculate(char expression[], double *result)
{
    // Using total for + and - operations and term for * and / operations
    // to handle operator precedence
    double total = 0;
    double term = 0;
    double currnumber = 0;

    char operator = '+';
    int index = 0;
    int expectingNumber = 1;

    while (expression[index] != '\0')
    {
        // Skip the whitespace
        if (isspace(expression[index]))
        {
            index++;
            continue;
        }
        // Reading the complete number
        if (isdigit(expression[index]))
        {
            currnumber = 0;
            while (isdigit(expression[index]))
            {
                currnumber = currnumber * 10 + (expression[index] - '0');
                index++;
            }
            // Apply the previous operator
            if (operator == '+')
            {
                total += term;
                term = currnumber;
            }
            else if (operator == '-')
            {
                total += term;
                term = -currnumber;
            }
            else if (operator == '*')
            {
                term *= currnumber;
            }
            else if (operator == '/')
            {
                if (currnumber == 0)
                {
                    printf("Error: Cannot divide by zero.\n");
                    return 0;
                }
                term /= currnumber;
            }
            expectingNumber = 0;
            continue;
        }
        // Read operator
        if (expression[index] == '+' || expression[index] == '-' || expression[index] == '*' || expression[index] == '/')
        {
            // Prevent consecutive operators
            if (expectingNumber)
            {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            operator = expression[index];
            expectingNumber = 1;
            index++;
        }
        else
        {
            printf("Invalid character: %c\n", expression[index]);
            return 0;
        }
    }
    // Expression cannot end with an operator
    if (expectingNumber)
    {
        printf("Error: Expression cannot end with an operator.\n");
        return 0;
    }
    // Add the final term to the total
    total += term;
    *result = total;

    return 1;
}

int main()
{
    char expression[100];
    double result;

    printf("Enter an expression: ");
    fgets(expression, sizeof(expression), stdin);
    expression[strcspn(expression, "\n")] = '\0';
    
    // Print the result only if the calculation is successful
    if (calculate(expression, &result))
    {
        printf("Result: %.4f\n", result);
    }
    return 0;
}