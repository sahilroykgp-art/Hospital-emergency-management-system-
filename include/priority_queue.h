#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "patient.h"

/* Min-Heap Priority Queue Structure for Emergency Triage
 * Ordered by: (severity ASC, arrival_order ASC)
 * Level 1 (Critical) has highest priority over Level 5 (Non-urgent).
 * FIFO tie-breaker ensures fairness for equal triage levels.
 */
typedef struct {
    Patient *heap;
    int size;
    int capacity;
} PriorityQueue;

/* Core Heap API */
bool pq_init(PriorityQueue *pq, int initial_capacity);
bool pq_enqueue(PriorityQueue *pq, const Patient *p);
bool pq_dequeue(PriorityQueue *pq, Patient *out_p);
bool pq_peek(const PriorityQueue *pq, Patient *out_p);
bool pq_is_empty(const PriorityQueue *pq);
int  pq_size(const PriorityQueue *pq);
void pq_clear(PriorityQueue *pq);
void pq_free(PriorityQueue *pq);

/* Search & Queue Maintenance */
bool pq_find_by_token(const PriorityQueue *pq, const char *token, Patient *out_p);
bool pq_remove_by_token(PriorityQueue *pq, const char *token, Patient *out_p);
bool pq_update_severity(PriorityQueue *pq, const char *token, TriageLevel new_severity, const char *reason);

/* Queue Inspection & Display */
int  pq_get_sorted_list(const PriorityQueue *pq, Patient *out_array, int max_count);
void pq_display_table(const PriorityQueue *pq);

#endif /* PRIORITY_QUEUE_H */
