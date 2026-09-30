
#include <stdio.h>
#include <ctype.h>

int calculate(char exp[], double *answer){
    double total = 0;
    double current = 0;
    double num = 0;

    char op = '+';
    int i = 0;

    while (exp[i] != '\0'){
        if (exp[i] == ' ' || exp[i] == '\n'){
            i++;
            continue;
        }

        // Read the complete number
        if (isdigit(exp[i])){
            num = 0;

            while (isdigit(exp[i])){
                num = num * 10 + (exp[i] - '0');
                i++;
            }

            // Apply the operator that came before this number
            if (op == '+'){
                total += current;
                current = num;
            }
            else if (op == '-'){
                total += current;
                current = -num;
            }
            else if (op == '*'){
                current = current * num;
            }
            else if (op == '/'){
                if (num == 0)
                {
                    printf("Error: Cannot divide by zero.\n");
                    return 0;
                }

                current = current / num;
            }

            continue;
        }

        // Save the operator for the next number
        if (exp[i] == '+' || exp[i] == '-' || exp[i] == '*' || exp[i] == '/'){
            op = exp[i];
            i++;
        }
        else{
            printf("Invalid character: %c\n", exp[i]);
            return 0;
        }
    }

    // Add the last calculated value
    total += current;

    *answer = total;

    return 1;
}

int main(){
    char expression[100];
    double result;

    printf("Enter an expression: ");
    scanf("%s", expression);

    if (calculate(expression, &result)){
        printf("Result: %.4f\n", result);
    }

    return 0;
}

