#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#define MAX_QUEUE 5
#define PRODUCER_NUMBER 3
#define DOCS_PER_PRODUCER 3
#define SUBMIT_GAP_MS 100

typedef struct {
    int  doc_id;
    char filename[60];
    int  pages;
} Document;

Document queue[MAX_QUEUE];
int head = 0, tail = 0, count = 0;
int all_sent = 0;           /* set to 1 by main after joining all producers */

int docs_submitted = 0;
int docs_printed = 0;
int pages_printed = 0;

pthread_mutex_t q_lock;
pthread_cond_t  not_full;   /* producers wait here when count == 5 */
pthread_cond_t  not_empty;  /* printer  waits here when count == 0 */

static const Document documents[PRODUCER_NUMBER][DOCS_PER_PRODUCER] = {
    {{0, "report_Q1.pdf", 12}, {0, "slides.pdf", 20},  {0, "summary.pdf", 4}},
    {{0, "contract.pdf", 5},   {0, "memo.pdf", 2},     {0, "budget.pdf", 7}},
    {{0, "invoice.pdf", 3},    {0, "proposal.pdf", 8}, {0, "notes.pdf", 5}}};

static void check_pthread(int error, const char *operation)
{
    if (error != 0) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        exit(EXIT_FAILURE);
    }
}

void *producer(void *arg)
{
    int producer_id = (int)(intptr_t)arg;
    const struct timespec submit_gap = {0, SUBMIT_GAP_MS * 1000000L};

    for(int i = 0; i < DOCS_PER_PRODUCER; i++)
    {
        Document doc = documents[producer_id - 1][i];

        check_pthread(pthread_mutex_lock(&q_lock), "pthread_mutex_lock");

        /* pthread_cond_wait() must be inside a while loop, not an if.
         * Waking up does not mean the condition is true:
         * 1. Between the signal and the moment this thread re-acquires
         *    q_lock, another thread can run first and change the state.
         *    Example: the printer frees one slot and signals not_full, but
         *    another producer takes q_lock first and fills that slot.
         *    With an "if" this producer would enqueue into a full queue
         *    and overwrite a document that has not been printed yet.
         * 2. Spurious wakeup: pthread_cond_wait() is allowed to return even
         *    though no thread called signal/broadcast on that condition
         *    variable (POSIX permits this, it makes the implementation
         *    cheaper on multiprocessor systems).
         * The while loop re-checks the condition after every wakeup, so the
         * thread only continues when the queue state really allows it. */
        while(count == MAX_QUEUE)
        {
            printf("[Producer %d] Queue full — waiting...\n", producer_id);
            check_pthread(pthread_cond_wait(&not_full, &q_lock), "pthread_cond_wait");
        }

        doc.doc_id = ++docs_submitted;
        queue[tail] = doc;
        tail = (tail + 1) % MAX_QUEUE;
        count++;
        printf("[Producer %d] Submitting: %-14s (%2d pages) — queue: %d/%d\n",
               producer_id, doc.filename, doc.pages, count, MAX_QUEUE);

        check_pthread(pthread_cond_signal(&not_empty), "pthread_cond_signal");
        check_pthread(pthread_mutex_unlock(&q_lock), "pthread_mutex_unlock");

        /* give the other producers a turn before submitting the next one */
        nanosleep(&submit_gap, NULL);
    }
    return NULL;
}

void *printer(void *arg)
{
    (void)arg;

    while(true)
    {
        check_pthread(pthread_mutex_lock(&q_lock), "pthread_mutex_lock");
        while(count == 0 && !all_sent)
        {
            check_pthread(pthread_cond_wait(&not_empty, &q_lock), "pthread_cond_wait");
        }

        if(count == 0 && all_sent)
        {
            check_pthread(pthread_mutex_unlock(&q_lock), "pthread_mutex_unlock");
            break;
        }

        Document doc = queue[head];
        head = (head + 1) % MAX_QUEUE;
        count--;
        docs_printed++;
        pages_printed += doc.pages;
        printf("[Printer]    Printing:   %-14s (%2d pages) — queue: %d/%d\n",
               doc.filename, doc.pages, count, MAX_QUEUE);

        check_pthread(pthread_cond_signal(&not_full), "pthread_cond_signal");
        check_pthread(pthread_mutex_unlock(&q_lock), "pthread_mutex_unlock");

        sleep(1);
    }

    printf("[Printer]    All documents printed. Exiting.\n");
    return NULL;
}

int main()
{
    pthread_t printer_thread;
    pthread_t producers[PRODUCER_NUMBER];

    check_pthread(pthread_mutex_init(&q_lock, NULL), "pthread_mutex_init");
    check_pthread(pthread_cond_init(&not_full, NULL), "pthread_cond_init");
    check_pthread(pthread_cond_init(&not_empty, NULL), "pthread_cond_init");

    printf("==============================================\n");
    printf("   OFFICE PRINT QUEUE (%d producers, 1 printer)\n", PRODUCER_NUMBER);
    printf("   Queue capacity: %d documents\n", MAX_QUEUE);
    printf("==============================================\n\n");

    check_pthread(pthread_create(&printer_thread, NULL, printer, NULL), "pthread_create");
    for(int i = 0; i < PRODUCER_NUMBER; i++)
    {
        check_pthread(pthread_create(&producers[i], NULL, producer, (void *)(intptr_t)(i + 1)),
                      "pthread_create");
    }

    for(int i = 0; i < PRODUCER_NUMBER; i++)
    {
        check_pthread(pthread_join(producers[i], NULL), "pthread_join");
    }

    check_pthread(pthread_mutex_lock(&q_lock), "pthread_mutex_lock");
    all_sent = 1;
    check_pthread(pthread_cond_broadcast(&not_empty), "pthread_cond_broadcast");
    check_pthread(pthread_mutex_unlock(&q_lock), "pthread_mutex_unlock");

    check_pthread(pthread_join(printer_thread, NULL), "pthread_join");

    printf("\n================ SUMMARY ================\n");
    printf("  Documents submitted : %d\n", docs_submitted);
    printf("  Documents printed   : %d\n", docs_printed);
    printf("  Total pages printed : %d\n", pages_printed);
    printf("=========================================\n");

    pthread_mutex_destroy(&q_lock);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    return 0;
}
