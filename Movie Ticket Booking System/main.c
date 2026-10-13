#include <stdio.h>
#include <string.h>
#include <stdbool.h>

const double STANDARD_PRICE = 150.00;
const double PREMIUM_PRICE  = 250.00;
const double RECLINER_PRICE = 400.00;

// Check if ticket count is within valid limit (1 - 20)
bool isValidTicketCount(int tickets)
{
    return (tickets >= 1 && tickets <= 20);
}

// Assign price and category based on user choice
void selectTicketCategory(int choice, double *ticketPrice, char category[])
{
    switch (choice)
    {
        case 1:
            *ticketPrice = STANDARD_PRICE;
            strcpy(category, "Standard");
            break;

        case 2:
            *ticketPrice = PREMIUM_PRICE;
            strcpy(category, "Premium");
            break;

        case 3:
            *ticketPrice = RECLINER_PRICE;
            strcpy(category, "Recliner");
            break;

        default:
            break;
    }
}

// 10% discount if buying 5 or more tickets
double calculateDiscount(int tickets, double subtotal)
{
    if (tickets >= 5)
        return subtotal * 0.10;

    return 0.0;
}

void displayBill(char customerName[], char category[], int tickets, double subtotal, double discount, double finalBill)
{
    printf("\n--- Booking Details ---\n");
    printf("Customer: %s\n", customerName);
    printf("Ticket Category: %s\n", category);
    printf("Tickets: %d\n", tickets);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", finalBill);
}

int main()
{
    char customerName[20];
    char ticketCategory[10];
    int numberOfTickets; 
    int choice;
    double ticketPrice = 0.0;
    double subtotal, discount, finalBill;

    printf("Enter customer name: ");
    scanf("%s", customerName);

    printf("Enter number of tickets (1-20): ");
    scanf("%d", &numberOfTickets);

    if (!isValidTicketCount(numberOfTickets))
    {
        printf("Error: Invalid number of tickets. Choose between 1 and 20.\n");
        return 1;
    }

    printf("\nSelect ticket category:\n");
    printf("1. Standard (150.00)\n");
    printf("2. Premium (250.00)\n");
    printf("3. Recliner (400.00)\n");
    printf("Enter your choice (1-3): ");
    scanf("%d", &choice);

    if (choice < 1 || choice > 3)
    {
        printf("Error: Invalid choice. Choose 1, 2, or 3.\n");
        return 1;
    }

    selectTicketCategory(choice, &ticketPrice, ticketCategory);

    // Bill calculations
    subtotal = numberOfTickets * ticketPrice;
    discount = calculateDiscount(numberOfTickets, subtotal);
    finalBill = subtotal - discount;

    displayBill(customerName, ticketCategory, numberOfTickets, subtotal, discount, finalBill);

    return 0;
}