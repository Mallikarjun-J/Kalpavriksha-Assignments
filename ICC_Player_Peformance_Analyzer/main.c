#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

#define MAX_TEAMS 10
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_NAME_LENGTH 50
#define ROLE_COUNT 3

enum { BATSMAN, BOWLER, ALL_ROUNDER };

// Player details structure
typedef struct
{
    int playerId;
    char name[MAX_NAME_LENGTH + 1];
    char teamName[MAX_NAME_LENGTH + 1];
    char role[20];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} PlayerInfo;

// Linked list node with three pointers for team, team role, and global role lists
typedef struct PlayerNode
{
    PlayerInfo data;
    struct PlayerNode *nextTeam;
    struct PlayerNode *nextTeamRole;
    struct PlayerNode *nextGlobalRole;
} PlayerNode;

// Team structure holding team stats and player list heads
typedef struct
{
    int teamId;
    char name[MAX_NAME_LENGTH + 1];
    int totalPlayers;
    float averageBattingStrikeRate;
    float strikeRateTotal;
    int battingPlayerCount;
    PlayerNode *playersHead;
    PlayerNode *playersTail;
    PlayerNode *roleHead[ROLE_COUNT];
} Team;

static Team teamList[MAX_TEAMS];
static int actualTeamCount = 0;
static PlayerNode *globalRoleHead[ROLE_COUNT];

// Match team name to assigned ID (1 to 10)
int getTeamId(const char *name)
{
    if (strcmp(name, "India") == 0) return 1;
    if (strcmp(name, "Australia") == 0) return 2;
    if (strcmp(name, "England") == 0) return 3;
    if (strcmp(name, "Pakistan") == 0) return 4;
    if (strcmp(name, "New Zealand") == 0) return 5;
    if (strcmp(name, "South Africa") == 0) return 6;
    if (strcmp(name, "Bangladesh") == 0) return 7;
    if (strcmp(name, "Sri Lanka") == 0) return 8;
    if (strcmp(name, "Afghanistan") == 0) return 9;
    if (strcmp(name, "West Indies") == 0) return 10;
    return -1;
}

// Convert role string to enum index
int getRoleIndex(const char *role)
{
    if (strcmp(role, "Batsman") == 0) return BATSMAN;
    if (strcmp(role, "Bowler") == 0) return BOWLER;
    if (strcmp(role, "All-rounder") == 0) return ALL_ROUNDER;
    return -1;
}

// Get display string for a role index
const char *getRoleName(int role)
{
    const char *names[ROLE_COUNT] = {"Batsman", "Bowler", "All-rounder"};
    return names[role];
}

// Calculate performance index based on player role
float calculatePerformanceIndex(PlayerInfo *p)
{
    float battingScore = (p->battingAverage * p->strikeRate) / 100.0f;
    int role = getRoleIndex(p->role);

    if (role == BATSMAN)
    {
        return battingScore;
    }

    if (role == BOWLER)
    {
        return (p->wickets * 2.0f) + (100.0f - p->economyRate);
    }

    return battingScore + (p->wickets * 2.0f);
}

// Higher performance index comes first, player ID breaks ties
int playerComesFirst(PlayerNode *a, PlayerNode *b)
{
    if (a->data.performanceIndex != b->data.performanceIndex)
    {
        return a->data.performanceIndex > b->data.performanceIndex;
    }

    return a->data.playerId < b->data.playerId;
}

// Insert player into team role list sorted by performance index
void insertTeamRole(PlayerNode **head, PlayerNode *newNode)
{
    if (*head == NULL || playerComesFirst(newNode, *head))
    {
        newNode->nextTeamRole = *head;
        *head = newNode;
        return;
    }

    PlayerNode *current = *head;

    while (current->nextTeamRole != NULL && !playerComesFirst(newNode, current->nextTeamRole))
    {
        current = current->nextTeamRole;
    }

    newNode->nextTeamRole = current->nextTeamRole;
    current->nextTeamRole = newNode;
}

// Insert player into global role list sorted by performance index
void insertGlobalRole(PlayerNode **head, PlayerNode *newNode)
{
    if (*head == NULL || playerComesFirst(newNode, *head))
    {
        newNode->nextGlobalRole = *head;
        *head = newNode;
        return;
    }

    PlayerNode *current = *head;

    while (current->nextGlobalRole != NULL && !playerComesFirst(newNode, current->nextGlobalRole))
    {
        current = current->nextGlobalRole;
    }

    newNode->nextGlobalRole = current->nextGlobalRole;
    current->nextGlobalRole = newNode;
}

// Add player to team list and update team stats
void addPlayerToTeam(Team *team, PlayerInfo player)
{
    PlayerNode *node = malloc(sizeof(PlayerNode));
    int role = getRoleIndex(player.role);

    if (node == NULL)
    {
        printf("Memory allocation failed. Player was not added.\n");
        return;
    }

    node->data = player;
    node->nextTeam = NULL;
    node->nextTeamRole = NULL;
    node->nextGlobalRole = NULL;

    // Add to team main linked list
    if (team->playersTail == NULL)
    {
        team->playersHead = node;
        team->playersTail = node;
    }
    else
    {
        team->playersTail->nextTeam = node;
        team->playersTail = node;
    }

    // Insert into sorted role lists
    insertTeamRole(&team->roleHead[role], node);
    insertGlobalRole(&globalRoleHead[role], node);

    team->totalPlayers++;

    if (role == BATSMAN || role == ALL_ROUNDER)
    {
        team->strikeRateTotal += player.strikeRate;
        team->battingPlayerCount++;
        team->averageBattingStrikeRate = team->strikeRateTotal / team->battingPlayerCount;
    }
}

// Binary search to find team by ID
Team *findTeam(int id)
{
    int low = 0;
    int high = actualTeamCount - 1;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;

        if (teamList[mid].teamId == id)
        {
            return &teamList[mid];
        }

        if (teamList[mid].teamId < id)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    return NULL;
}

// Load teams from header data
void initializeTeams()
{
    for (int i = 0; i < teamCount; i++)
    {
        int id = getTeamId(teams[i]);

        if (id < 1 || id > MAX_TEAMS)
        {
            printf("Invalid team name in Players_data.h: %s\n", teams[i]);
            exit(1);
        }

        teamList[id - 1].teamId = id;
        strcpy(teamList[id - 1].name, teams[i]);
    }

    actualTeamCount = teamCount;
}

// Load all players from header data and insert into teams
void initializePlayers()
{
    for (int i = 0; i < playerCount; i++)
    {
        PlayerInfo p;
        int id = getTeamId(players[i].team);
        Team *team = findTeam(id);

        if (team == NULL)
        {
            continue;
        }

        p.playerId = players[i].id;
        strcpy(p.name, players[i].name);
        strcpy(p.teamName, players[i].team);
        strcpy(p.role, players[i].role);
        p.totalRuns = players[i].totalRuns;
        p.battingAverage = players[i].battingAverage;
        p.strikeRate = players[i].strikeRate;
        p.wickets = players[i].wickets;
        p.economyRate = players[i].economyRate;
        p.performanceIndex = calculatePerformanceIndex(&p);

        addPlayerToTeam(team, p);
    }
}

// Discard extra characters from stdin
void clearInputLine()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

// Read integer safely
int readInt(const char *prompt, int *value)
{
    printf("%s", prompt);

    if (scanf("%d", value) != 1)
    {
        clearInputLine();
        return 0;
    }

    clearInputLine();
    return 1;
}

// Read float safely
int readFloat(const char *prompt, float *value)
{
    printf("%s", prompt);

    if (scanf("%f", value) != 1)
    {
        clearInputLine();
        return 0;
    }

    clearInputLine();
    return 1;
}

// Read line of text safely
void readText(const char *prompt, char *text, int size)
{
    printf("%s", prompt);

    if (fgets(text, size, stdin) == NULL)
    {
        text[0] = '\0';
        return;
    }

    text[strcspn(text, "\n")] = '\0';
}

// Prompt user for role selection
int readRole()
{
    int choice;

    if (!readInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", &choice))
    {
        return -1;
    }

    if (choice < 1 || choice > 3)
    {
        return -1;
    }

    return choice - 1;
}

// Check if player ID already exists across all teams
int playerIdExists(int id)
{
    for (int i = 0; i < actualTeamCount; i++)
    {
        PlayerNode *current = teamList[i].playersHead;

        while (current != NULL)
        {
            if (current->data.playerId == id)
            {
                return 1;
            }
            current = current->nextTeam;
        }
    }

    return 0;
}

// Add a new player via interactive console prompts
void addNewPlayer()
{
    int teamId;
    int role;
    Team *team;
    PlayerInfo p;

    if (!readInt("Enter Team ID: ", &teamId))
    {
        printf("Invalid Team ID.\n");
        return;
    }

    team = findTeam(teamId);

    if (team == NULL)
    {
        printf("Team not found.\n");
        return;
    }

    if (team->totalPlayers >= MAX_PLAYERS_PER_TEAM)
    {
        printf("This team already has the maximum of %d players.\n", MAX_PLAYERS_PER_TEAM);
        return;
    }

    if (!readInt("Player ID (1-1000): ", &p.playerId) || p.playerId < 1 || p.playerId > 1000 || playerIdExists(p.playerId))
    {
        printf("Invalid or duplicate Player ID.\n");
        return;
    }

    readText("Name: ", p.name, sizeof(p.name));

    if (p.name[0] == '\0')
    {
        printf("Name cannot be empty.\n");
        return;
    }

    role = readRole();

    if (role == -1)
    {
        printf("Invalid role.\n");
        return;
    }

    strcpy(p.role, getRoleName(role));
    strcpy(p.teamName, team->name);

    if (!readInt("Total Runs: ", &p.totalRuns) || p.totalRuns < 0)
    {
        printf("Invalid statistics. Values must be non-negative.\n");
        return;
    }

    if (!readFloat("Batting Average: ", &p.battingAverage) || p.battingAverage < 0.0f)
    {
        printf("Invalid statistics. Values must be non-negative.\n");
        return;
    }

    if (!readFloat("Strike Rate: ", &p.strikeRate) || p.strikeRate < 0.0f)
    {
        printf("Invalid statistics. Values must be non-negative.\n");
        return;
    }

    if (!readInt("Wickets: ", &p.wickets) || p.wickets < 0)
    {
        printf("Invalid statistics. Values must be non-negative.\n");
        return;
    }

    if (!readFloat("Economy Rate: ", &p.economyRate) || p.economyRate < 0.0f)
    {
        printf("Invalid statistics. Values must be non-negative.\n");
        return;
    }

    p.performanceIndex = calculatePerformanceIndex(&p);
    addPlayerToTeam(team, p);
    printf("Player added successfully to Team %s.\n", team->name);
}

// Print formatted table header
void printTableHeader(int showTeam)
{
    if (showTeam)
    {
        printf("%-6s %-25s %-16s %-13s %8s %8s %8s %7s %7s %10s\n",
               "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "Econ", "Index");
    }
    else
    {
        printf("%-6s %-25s %-13s %8s %8s %8s %7s %7s %10s\n",
               "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "Econ", "Index");
    }

    printf("---------------------------------------------------------------------------------------------------------------\n");
}

// Print single player stats row
void printPlayer(PlayerInfo *p, int showTeam)
{
    if (showTeam)
    {
        printf("%-6d %-25s %-16s %-13s %8d %8.1f %8.1f %7d %7.1f %10.2f\n",
               p->playerId, p->name, p->teamName, p->role, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets,
               p->economyRate, p->performanceIndex);
    }
    else
    {
        printf("%-6d %-25s %-13s %8d %8.1f %8.1f %7d %7.1f %10.2f\n",
               p->playerId, p->name, p->role, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets,
               p->economyRate, p->performanceIndex);
    }
}

// Display all players belonging to a team
void displayTeamPlayers()
{
    int id;
    Team *team;
    PlayerNode *current;

    if (!readInt("Enter Team ID: ", &id))
    {
        printf("Invalid Team ID.\n");
        return;
    }

    team = findTeam(id);

    if (team == NULL)
    {
        printf("Team not found.\n");
        return;
    }

    printf("\nPlayers of Team %s\n", team->name);
    printTableHeader(0);

    current = team->playersHead;

    while (current != NULL)
    {
        printPlayer(&current->data, 0);
        current = current->nextTeam;
    }

    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", team->averageBattingStrikeRate);
}

// Comparison function for sorting teams by average strike rate descending
int teamComesFirst(Team *a, Team *b)
{
    if (a->averageBattingStrikeRate != b->averageBattingStrikeRate)
    {
        return a->averageBattingStrikeRate > b->averageBattingStrikeRate;
    }

    return a->teamId < b->teamId;
}

// Merge two sorted team subarrays
void mergeTeams(Team *list[], Team *temp[], int left, int mid, int right)
{
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right)
    {
        if (teamComesFirst(list[i], list[j]))
        {
            temp[k++] = list[i++];
        }
        else
        {
            temp[k++] = list[j++];
        }
    }

    while (i <= mid)
    {
        temp[k++] = list[i++];
    }

    while (j <= right)
    {
        temp[k++] = list[j++];
    }

    for (i = left; i <= right; i++)
    {
        list[i] = temp[i];
    }
}

// Recursive merge sort for teams array
void mergeSortTeams(Team *list[], Team *temp[], int left, int right)
{
    if (left >= right)
    {
        return;
    }

    int mid = left + (right - left) / 2;

    mergeSortTeams(list, temp, left, mid);
    mergeSortTeams(list, temp, mid + 1, right);
    mergeTeams(list, temp, left, mid, right);
}

// Display teams sorted by average strike rate
void displayTeamsByAverage()
{
    Team *sorted[MAX_TEAMS];
    Team *temp[MAX_TEAMS];

    for (int i = 0; i < actualTeamCount; i++)
    {
        sorted[i] = &teamList[i];
    }

    mergeSortTeams(sorted, temp, 0, actualTeamCount - 1);

    printf("\nTeams by Average Batting Strike Rate (Descending)\n");
    printf("%-8s %-20s %15s %15s\n", "Team ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("---------------------------------------------------------------------\n");

    for (int i = 0; i < actualTeamCount; i++)
    {
        printf("%-8d %-20s %15.2f %15d\n", sorted[i]->teamId,
               sorted[i]->name, sorted[i]->averageBattingStrikeRate,
               sorted[i]->totalPlayers);
    }
}

// Display top K players in a team for a specific role
void displayTopKPlayers()
{
    int id;
    int role;
    int k;
    int count = 0;
    Team *team;
    PlayerNode *current;

    if (!readInt("Enter Team ID: ", &id))
    {
        printf("Invalid Team ID.\n");
        return;
    }

    team = findTeam(id);

    if (team == NULL)
    {
        printf("Team not found.\n");
        return;
    }

    role = readRole();

    if (role == -1)
    {
        printf("Invalid role.\n");
        return;
    }

    if (!readInt("Enter number of players (K): ", &k) || k <= 0)
    {
        printf("K must be a positive number.\n");
        return;
    }

    printf("\nTop %d %s players of Team %s\n", k, getRoleName(role), team->name);
    printTableHeader(0);

    current = team->roleHead[role];

    while (current != NULL && count < k)
    {
        printPlayer(&current->data, 0);
        current = current->nextTeamRole;
        count++;
    }

    printf("Players displayed: %d\n", count);
}

// Display all players of a selected role across all teams
void displayAllPlayersOfRole()
{
    int role = readRole();
    PlayerNode *current;
    int count = 0;

    if (role == -1)
    {
        printf("Invalid role.\n");
        return;
    }

    printf("\nAll %s players across all teams\n", getRoleName(role));
    printTableHeader(1);

    current = globalRoleHead[role];

    while (current != NULL)
    {
        printPlayer(&current->data, 1);
        current = current->nextGlobalRole;
        count++;
    }

    printf("Total players displayed: %d\n", count);
}

// Free all allocated player nodes across all teams
void freePlayers()
{
    for (int i = 0; i < actualTeamCount; i++)
    {
        PlayerNode *current = teamList[i].playersHead;

        while (current != NULL)
        {
            PlayerNode *next = current->nextTeam;
            free(current);
            current = next;
        }
    }
}

// Display main menu options
void displayMenu()
{
    printf("\n====================================================\n");
    printf("        ICC ODI PLAYER PERFORMANCE ANALYZER\n");
    printf("====================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Team by Role\n");
    printf("5. Display All Players of a Role Across Teams\n");
    printf("6. Exit\n");
    printf("====================================================\n");
}

int main()
{
    int choice;

    initializeTeams();
    initializePlayers();

    printf("Loaded %d players across %d teams.\n", playerCount, actualTeamCount);
    printf("Team IDs: 1-India, 2-Australia, 3-England, 4-Pakistan, 5-New Zealand,\n");
    printf("          6-South Africa, 7-Bangladesh, 8-Sri Lanka, 9-Afghanistan, 10-West Indies.\n");

    while (1)
    {
        displayMenu();

        if (!readInt("Enter your choice: ", &choice))
        {
            printf("Please enter a valid number.\n");
            continue;
        }

        switch (choice)
        {
            case 1:
                addNewPlayer();
                break;

            case 2:
                displayTeamPlayers();
                break;

            case 3:
                displayTeamsByAverage();
                break;

            case 4:
                displayTopKPlayers();
                break;

            case 5:
                displayAllPlayersOfRole();
                break;

            case 6:
                freePlayers();
                printf("Program ended.\n");
                return 0;

            default:
                printf("Invalid choice. Please choose from 1 to 6.\n");
        }
    }

    return 0;
}