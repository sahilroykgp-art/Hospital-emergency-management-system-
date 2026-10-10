#include "priority_queue.h"

/* Comparator: Returns true if patient 'a' has strictly higher priority than patient 'b'
 * 1. Lower severity value = higher medical priority (1 is Critical, 5 is Non-urgent)
 * 2. Equal severity = earlier arrival_order has higher priority (Strict FIFO fairness)
 */
static bool has_higher_priority(const Patient *a, const Patient *b) {
    if (a->severity != b->severity) {
        return a->severity < b->severity;
    }
    return a->arrival_order < b->arrival_order;
}

static void swap_nodes(Patient *a, Patient *b) {
    Patient temp = *a;
    *a = *b;
    *b = temp;
}

static void sift_up(PriorityQueue *pq, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (has_higher_priority(&pq->heap[idx], &pq->heap[parent])) {
            swap_nodes(&pq->heap[idx], &pq->heap[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

static void sift_down(PriorityQueue *pq, int idx) {
    while (idx * 2 + 1 < pq->size) {
        int left = idx * 2 + 1;
        int right = idx * 2 + 2;
        int best = idx;

        if (left < pq->size && has_higher_priority(&pq->heap[left], &pq->heap[best])) {
            best = left;
        }
        if (right < pq->size && has_higher_priority(&pq->heap[right], &pq->heap[best])) {
            best = right;
        }

        if (best != idx) {
            swap_nodes(&pq->heap[idx], &pq->heap[best]);
            idx = best;
        } else {
            break;
        }
    }
}

bool pq_init(PriorityQueue *pq, int initial_capacity) {
    if (!pq) return false;
    if (initial_capacity <= 0) initial_capacity = 32;
    pq->heap = (Patient *)malloc(sizeof(Patient) * (size_t)initial_capacity);
    if (!pq->heap) {
        pq->size = 0;
        pq->capacity = 0;
        return false;
    }
    pq->size = 0;
    pq->capacity = initial_capacity;
    return true;
}

bool pq_enqueue(PriorityQueue *pq, const Patient *p) {
    if (!pq || !p) return false;

    /* Dynamic resize if capacity reached */
    if (pq->size >= pq->capacity) {
        int new_capacity = pq->capacity * 2;
        Patient *new_heap = (Patient *)realloc(pq->heap, sizeof(Patient) * (size_t)new_capacity);
        if (!new_heap) return false;
        pq->heap = new_heap;
        pq->capacity = new_capacity;
    }

    pq->heap[pq->size] = *p;
    sift_up(pq, pq->size);
    pq->size++;
    return true;
}

bool pq_dequeue(PriorityQueue *pq, Patient *out_p) {
    if (!pq || pq->size == 0) return false;

    if (out_p) {
        *out_p = pq->heap[0];
    }

    pq->size--;
    if (pq->size > 0) {
        pq->heap[0] = pq->heap[pq->size];
        sift_down(pq, 0);
    }
    return true;
}

bool pq_peek(const PriorityQueue *pq, Patient *out_p) {
    if (!pq || pq->size == 0) return false;
    if (out_p) {
        *out_p = pq->heap[0];
    }
    return true;
}

bool pq_is_empty(const PriorityQueue *pq) {
    return (!pq || pq->size == 0);
}

int pq_size(const PriorityQueue *pq) {
    return pq ? pq->size : 0;
}

void pq_clear(PriorityQueue *pq) {
    if (pq) {
        pq->size = 0;
    }
}

void pq_free(PriorityQueue *pq) {
    if (pq) {
        if (pq->heap) {
            free(pq->heap);
            pq->heap = NULL;
        }
        pq->size = 0;
        pq->capacity = 0;
    }
}

bool pq_find_by_token(const PriorityQueue *pq, const char *token, Patient *out_p) {
    if (!pq || !token) return false;
    for (int i = 0; i < pq->size; i++) {
        if (strcmp(pq->heap[i].token, token) == 0) {
            if (out_p) *out_p = pq->heap[i];
            return true;
        }
    }
    return false;
}

bool pq_remove_by_token(PriorityQueue *pq, const char *token, Patient *out_p) {
    if (!pq || !token || pq->size == 0) return false;

    int target_idx = -1;
    for (int i = 0; i < pq->size; i++) {
        if (strcmp(pq->heap[i].token, token) == 0) {
            target_idx = i;
            break;
        }
    }

    if (target_idx == -1) return false;

    if (out_p) {
        *out_p = pq->heap[target_idx];
    }

    pq->size--;
    if (target_idx < pq->size) {
        pq->heap[target_idx] = pq->heap[pq->size];
        sift_down(pq, target_idx);
        sift_up(pq, target_idx);
    }
    return true;
}

bool pq_update_severity(PriorityQueue *pq, const char *token, TriageLevel new_severity, const char *reason) {
    if (!pq || !token) return false;
    for (int i = 0; i < pq->size; i++) {
        if (strcmp(pq->heap[i].token, token) == 0) {
            pq->heap[i].severity = new_severity;
            if (reason && strlen(reason) > 0) {
                snprintf(pq->heap[i].triage_notes, sizeof(pq->heap[i].triage_notes), "Re-triaged: %s", reason);
            }
            sift_up(pq, i);
            sift_down(pq, i);
            return true;
        }
    }
    return false;
}

/* Returns a copy of patients sorted in strict medical priority order */
int pq_get_sorted_list(const PriorityQueue *pq, Patient *out_array, int max_count) {
    if (!pq || !out_array || pq->size == 0 || max_count <= 0) return 0;

    int count = pq->size < max_count ? pq->size : max_count;
    /* Create a temporary copy of the heap to extract in priority order */
    PriorityQueue temp;
    if (!pq_init(&temp, pq->size)) return 0;

    for (int i = 0; i < pq->size; i++) {
        temp.heap[i] = pq->heap[i];
    }
    temp.size = pq->size;

    for (int i = 0; i < count; i++) {
        pq_dequeue(&temp, &out_array[i]);
    }

    pq_free(&temp);
    return count;
}

void pq_display_table(const PriorityQueue *pq) {
    if (!pq || pq->size == 0) {
        printf("\n" COLOR_YELLOW "  [INFO] No patients currently waiting in the emergency queue.\n" COLOR_RESET);
        return;
    }

    Patient *sorted = (Patient *)malloc(sizeof(Patient) * (size_t)pq->size);
    if (!sorted) return;

    int total = pq_get_sorted_list(pq, sorted, pq->size);

    printf("\n" COLOR_BOLD "========================================================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                             LIVE EMERGENCY PRIORITY QUEUE (%d Waiting)                                 \n" COLOR_RESET, total);
    printf("========================================================================================================\n");
    printf(COLOR_BOLD "%-4s | %-9s | %-20s | %-4s | %-12s | %-24s | %-19s\n" COLOR_RESET,
           "Rank", "Token", "Patient Name", "Age", "Severity", "Chief Complaint", "Arrival Time");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < total; i++) {
        const char *col = triage_level_color(sorted[i].severity);
        char sev_label[16];
        snprintf(sev_label, sizeof(sev_label), "Level %d", sorted[i].severity);

        printf("%-4d | %-9s | %-20.20s | %-4d | %s%-12s" COLOR_RESET " | %-24.24s | %-19s\n",
               i + 1,
               sorted[i].token,
               sorted[i].name,
               sorted[i].age,
               col,
               sev_label,
               sorted[i].chief_complaint,
               sorted[i].arrival_time);
    }
    printf("========================================================================================================\n");
    free(sorted);
}
