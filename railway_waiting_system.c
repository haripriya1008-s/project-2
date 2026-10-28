#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEAT_CAPACITY 3
#define MAX_WAITING 5
#define INPUT_SIZE 100

typedef struct {
    int ticketId;
    int seatNumber;
    char name[50];
    char train[50];
    char travelDate[20];
    char origin[50];
    char destination[50];
} Passenger;

/* Demo data only. A real booking service must use an authorized timetable. */
static const char *trainOptions[] = {
    "TG101 Coastliner Express",
    "TG102 Deccan Evening"
};

static Passenger seats[SEAT_CAPACITY];
static int seatOccupied[SEAT_CAPACITY] = {0};
static Passenger waitingQueue[MAX_WAITING];
static int front = 0;
static int rear = -1;
static int waitingCount = 0;
static int nextTicketId = 1001;
static char currentTrain[50];
static char currentDate[20];
static char currentOrigin[50];
static char currentDestination[50];

static int readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) return 0;
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

static int readChoice(const char *prompt, int minimum, int maximum) {
    char input[INPUT_SIZE];
    char *end;
    long value;

    for (;;) {
        if (!readLine(prompt, input, sizeof(input))) return -1;
        value = strtol(input, &end, 10);
        while (*end == ' ' || *end == '\t') end++;
        if (end != input && *end == '\0' && value >= minimum && value <= maximum)
            return (int)value;
        printf("Enter a number from %d to %d.\n", minimum, maximum);
    }
}

static int findFreeSeat(void) {
    int i;
    for (i = 0; i < SEAT_CAPACITY; i++) {
        if (!seatOccupied[i]) return i;
    }
    return -1;
}

static void addPassenger(void) {
    Passenger passenger = {0};
    int freeSeat;

    if (waitingCount == MAX_WAITING && findFreeSeat() == -1) {
        printf("All sample seats and waiting-list slots are full.\n");
        return;
    }

    if (!readLine("Passenger name: ", passenger.name, sizeof(passenger.name)) ||
        passenger.name[0] == '\0') {
        printf("A passenger name is required.\n");
        return;
    }
    passenger.ticketId = nextTicketId++;
    passenger.seatNumber = 0;
    snprintf(passenger.train, sizeof(passenger.train), "%s", currentTrain);
    snprintf(passenger.travelDate, sizeof(passenger.travelDate), "%s", currentDate);
    snprintf(passenger.origin, sizeof(passenger.origin), "%s", currentOrigin);
    snprintf(passenger.destination, sizeof(passenger.destination), "%s", currentDestination);

    freeSeat = findFreeSeat();
    if (freeSeat >= 0) {
        passenger.seatNumber = freeSeat + 1;
        seats[freeSeat] = passenger;
        seatOccupied[freeSeat] = 1;
        printf("Sample seat %d assigned. Ticket ID: %d for %s.\n",
               passenger.seatNumber, passenger.ticketId, passenger.name);
        return;
    }

    rear = (rear + 1) % MAX_WAITING;
    waitingQueue[rear] = passenger;
    waitingCount++;
    printf("No sample seat is available. %s is on waiting-list slot WL%02d. "
           "Ticket ID: %d.\n", passenger.name, waitingCount, passenger.ticketId);
}

static void displayPassengers(void) {
    int i;
    int listed = 0;

    printf("\n--- Sample seat assignments ---\n");
    for (i = 0; i < SEAT_CAPACITY; i++) {
        if (seatOccupied[i]) {
            Passenger passenger = seats[i];
            printf("Seat %d | %s | Ticket %d | %s | %s | %s to %s\n",
                   i + 1, passenger.name, passenger.ticketId, passenger.train,
                   passenger.travelDate, passenger.origin, passenger.destination);
            listed = 1;
        }
    }
    if (!listed) printf("No sample seats assigned.\n");

    printf("\n--- Waiting list (FIFO) ---\n");
    if (waitingCount == 0) {
        printf("No passengers are waiting.\n");
        return;
    }
    for (i = 0; i < waitingCount; i++) {
        int index = (front + i) % MAX_WAITING;
        Passenger passenger = waitingQueue[index];
        printf("WL%02d | %s | Ticket %d | %s | %s | %s to %s\n",
               i + 1, passenger.name, passenger.ticketId, passenger.train,
               passenger.travelDate, passenger.origin, passenger.destination);
    }
}

static void releaseSeatAndPromote(void) {
    int i;
    int promotedSeat;
    for (i = 0; i < SEAT_CAPACITY; i++) {
        if (seatOccupied[i]) break;
    }
    if (i == SEAT_CAPACITY) {
        printf("There are no assigned seats to release.\n");
        return;
    }

    printf("Sample seat %d released from %s.\n", i + 1, seats[i].name);
    seatOccupied[i] = 0;
    if (waitingCount == 0) {
        printf("No one is waiting for this sample seat.\n");
        return;
    }

    promotedSeat = i;
    seats[promotedSeat] = waitingQueue[front];
    seats[promotedSeat].seatNumber = promotedSeat + 1;
    seatOccupied[promotedSeat] = 1;
    printf("%s (Ticket %d) moved from WL01 to sample seat %d.\n",
           seats[promotedSeat].name, seats[promotedSeat].ticketId, promotedSeat + 1);
    front = (front + 1) % MAX_WAITING;
    waitingCount--;
}

int main(void) {
    int choice;
    int trainChoice;
    printf("=== Track & Go Railway Queue (sample data) ===\n");
    printf("Choose one departure. Each program run manages this train, date, and route.\n");
    printf("1. %s\n2. %s\n", trainOptions[0], trainOptions[1]);
    trainChoice = readChoice("Choose train: ", 1, 2);
    if (trainChoice == -1) return 0;
    snprintf(currentTrain, sizeof(currentTrain), "%s", trainOptions[trainChoice - 1]);
    if (!readLine("Travel date (YYYY-MM-DD): ", currentDate, sizeof(currentDate)) || currentDate[0] == '\0' ||
        !readLine("From station: ", currentOrigin, sizeof(currentOrigin)) || currentOrigin[0] == '\0' ||
        !readLine("To station: ", currentDestination, sizeof(currentDestination)) || currentDestination[0] == '\0') {
        printf("A travel date and both stations are required.\n");
        return 1;
    }
    do {
        printf("\n1. Add passenger / check seat\n");
        printf("2. Show seats and waiting list\n");
        printf("3. Release a seat and promote next passenger\n");
        printf("4. Exit\n");
        choice = readChoice("Choose an option: ", 1, 4);
        switch (choice) {
            case 1: addPassenger(); break;
            case 2: displayPassengers(); break;
            case 3: releaseSeatAndPromote(); break;
            case 4: printf("Goodbye. Sample data will be cleared.\n"); break;
            case -1: return 0;
        }
    } while (choice != 4);
    return 0;
}
