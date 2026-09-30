#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_SEATS 10
#define AGENT_NUMBER 5

typedef struct {
    int  agent_id;
    char customer[50];
    int  seats_wanted;
} BookingRequest;

int seats_available = MAX_SEATS;
pthread_mutex_t seat_lock;
pthread_mutex_t gate_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  all_ready = PTHREAD_COND_INITIALIZER;  /* agent -> main */
pthread_cond_t  gate_open = PTHREAD_COND_INITIALIZER;  /* main -> agents */
int  ready_count = 0;
bool go = false;

void *book_ticket(void *arg)
{
    BookingRequest *request = (BookingRequest *)arg;
    bool result = true;
    int needed_seats = request->seats_wanted;
    const char *temp_str = (needed_seats > 1) ? "seats" : "seat ";

    printf("[Agent %d | TID %lu] Booking %d %s for %s...\n",
           request->agent_id, (unsigned long)pthread_self(),
           needed_seats, temp_str, request->customer);

    pthread_mutex_lock(&gate_lock);
    ready_count++;
    pthread_cond_signal(&all_ready);
    while(!go)
        pthread_cond_wait(&gate_open, &gate_lock);
    pthread_mutex_unlock(&gate_lock);

    sleep(1);

    /* Check and deduct must be in ONE critical section.
     * If split into two lock/unlock blocks, another agent can deduct seats
     * in between, so our check result becomes stale.
     * Example (3 seats left): Agent 3 checks OK, Agent 5 checks OK,
     * then both deduct -> 3 - 3 - 2 = -2 (overbooked).
     * One lock around both steps makes "check + deduct" atomic. */
    pthread_mutex_lock(&seat_lock);
    if(needed_seats <= seats_available)
    {
        seats_available -= needed_seats;
        printf("[Agent %d] CONFIRMED: %d %s for %s. Remaining: %d\n",
               request->agent_id, needed_seats, temp_str, request->customer, seats_available);
    }
    else
    {
        printf("[Agent %d] SOLD OUT:  needs %d %s, only %d left - booking failed.\n",
               request->agent_id, needed_seats, temp_str, seats_available);
        result = false;
    }
    pthread_mutex_unlock(&seat_lock);
    return (void *)(intptr_t)result;
}

int main()
{
    int failed_bookings = 0;
    BookingRequest requests[AGENT_NUMBER] = {
        {1, "Nguyen Van An", 2},
        {2, "Tran Thi Bich", 1},
        {3, "Le Van Cuong", 3},
        {4, "Pham Thi Dung", 1},
        {5, "Hoang Van Em", 2}};

    pthread_t agent[AGENT_NUMBER];
    bool created[AGENT_NUMBER] = {false};
    int ret = pthread_mutex_init(&seat_lock, NULL);
    if(ret != 0)
    {
        fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(ret));
        return EXIT_FAILURE;
    }

    printf("==============================================\n");
    printf("   TICKET BOOKING SYSTEM (%d agents, %d seats)\n", AGENT_NUMBER, MAX_SEATS);
    printf("==============================================\n");

    int created_count = 0;
    for(int i = 0; i < AGENT_NUMBER; i++)
    {
        ret = pthread_create(&agent[i], NULL, book_ticket, &requests[i]);
        if(ret != 0)
        {
            fprintf(stderr, "Failed to create agent %d: %s\n",
                    requests[i].agent_id, strerror(ret));
            failed_bookings++;
            continue;
        }
        created[i] = true;
        created_count++;
    }

    pthread_mutex_lock(&gate_lock);
    while(ready_count < created_count)
        pthread_cond_wait(&all_ready, &gate_lock);

    printf("\n--- [all agents reach critical section after sleep(1)] ---\n\n");

    go = true;
    pthread_cond_broadcast(&gate_open);
    pthread_mutex_unlock(&gate_lock);

    for(int i = 0; i < AGENT_NUMBER; i++)
    {
        void *retval;
        if(!created[i]) continue;
        ret = pthread_join(agent[i], &retval);
        if(ret != 0)
        {
            fprintf(stderr, "Failed to join agent %d: %s\n",
                    requests[i].agent_id, strerror(ret));
            failed_bookings++;
            continue;
        }
        bool booking_res = (bool)(intptr_t)retval;
        if(!booking_res) failed_bookings++;
    }

    printf("\n================ SUMMARY ================\n");
    printf("  Total seats     : %d\n", MAX_SEATS);
    printf("  Seats sold      : %d\n", MAX_SEATS - seats_available);
    printf("  Seats remaining : %d\n", seats_available);
    printf("  Failed bookings : %d\n", failed_bookings);
    printf("=========================================\n");

    pthread_mutex_destroy(&seat_lock);
    pthread_mutex_destroy(&gate_lock);
    pthread_cond_destroy(&all_ready);
    pthread_cond_destroy(&gate_open);
    return 0;
}
