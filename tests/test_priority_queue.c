#include "common.h"
#include "patient.h"
#include "triage.h"
#include "priority_queue.h"
#include "bed_manager.h"
#include "file_io.h"
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    tests_run++; \
    if (cond) { \
        tests_passed++; \
        printf(COLOR_GREEN "  [PASS] " COLOR_RESET "%s\n", msg); \
    } else { \
        printf(COLOR_RED "  [FAIL] " COLOR_RESET "%s (Line %d)\n", msg, __LINE__); \
    } \
} while(0)

static void test_empty_queue(void) {
    printf(COLOR_BOLD "\n--- Test Suite 1: Empty Queue Boundary Conditions ---\n" COLOR_RESET);
    PriorityQueue pq;
    pq_init(&pq, 10);

    TEST_ASSERT(pq_is_empty(&pq) == true, "New queue is empty");
    TEST_ASSERT(pq_size(&pq) == 0, "New queue size is 0");

    Patient out_p;
    TEST_ASSERT(pq_peek(&pq, &out_p) == false, "Peek on empty queue returns false");
    TEST_ASSERT(pq_dequeue(&pq, &out_p) == false, "Dequeue on empty queue returns false (No crash)");

    pq_free(&pq);
}

static void test_min_heap_priority_ordering(void) {
    printf(COLOR_BOLD "\n--- Test Suite 2: Triage Severity Priority Ordering ---\n" COLOR_RESET);
    PriorityQueue pq;
    pq_init(&pq, 10);

    /* Enqueue in reverse priority order: Level 5, then Level 3, then Level 1 */
    Patient p5, p3, p1;
    patient_init_default(&p5);
    p5.id = 1;
    strcpy(p5.token, "TK-0001");
    p5.severity = TRIAGE_NON_URGENT; /* Level 5 */
    p5.arrival_order = 1;

    patient_init_default(&p3);
    p3.id = 2;
    strcpy(p3.token, "TK-0002");
    p3.severity = TRIAGE_URGENT; /* Level 3 */
    p3.arrival_order = 2;

    patient_init_default(&p1);
    p1.id = 3;
    strcpy(p1.token, "TK-0003");
    p1.severity = TRIAGE_CRITICAL; /* Level 1 */
    p1.arrival_order = 3;

    pq_enqueue(&pq, &p5);
    pq_enqueue(&pq, &p3);
    pq_enqueue(&pq, &p1);

    TEST_ASSERT(pq_size(&pq) == 3, "Queue holds 3 patients");

    /* Dequeue should return Level 1 first, then Level 3, then Level 5 */
    Patient out1, out2, out3;
    TEST_ASSERT(pq_dequeue(&pq, &out1) == true, "First dequeue succeeds");
    TEST_ASSERT(out1.severity == TRIAGE_CRITICAL && strcmp(out1.token, "TK-0003") == 0,
                "Highest priority (Level 1 / Critical) dequeued first");

    TEST_ASSERT(pq_dequeue(&pq, &out2) == true, "Second dequeue succeeds");
    TEST_ASSERT(out2.severity == TRIAGE_URGENT && strcmp(out2.token, "TK-0002") == 0,
                "Second priority (Level 3 / Urgent) dequeued second");

    TEST_ASSERT(pq_dequeue(&pq, &out3) == true, "Third dequeue succeeds");
    TEST_ASSERT(out3.severity == TRIAGE_NON_URGENT && strcmp(out3.token, "TK-0001") == 0,
                "Lowest priority (Level 5 / Non-urgent) dequeued last");

    TEST_ASSERT(pq_is_empty(&pq) == true, "Queue is empty after all dequeues");

    pq_free(&pq);
}

static void test_fifo_tie_breaking(void) {
    printf(COLOR_BOLD "\n--- Test Suite 3: FIFO Arrival Tie-Breaking for Equal Severity ---\n" COLOR_RESET);
    PriorityQueue pq;
    pq_init(&pq, 10);

    /* Three patients all having Severity Level 2 (Emergent), but arriving at sequence 10, 11, 12 */
    Patient a, b, c;
    patient_init_default(&a);
    a.id = 10;
    strcpy(a.token, "TK-0010");
    a.severity = TRIAGE_EMERGENT;
    a.arrival_order = 10;

    patient_init_default(&b);
    b.id = 11;
    strcpy(b.token, "TK-0011");
    b.severity = TRIAGE_EMERGENT;
    b.arrival_order = 11;

    patient_init_default(&c);
    c.id = 12;
    strcpy(c.token, "TK-0012");
    c.severity = TRIAGE_EMERGENT;
    c.arrival_order = 12;

    /* Enqueue in order: b, then c, then a */
    pq_enqueue(&pq, &b);
    pq_enqueue(&pq, &c);
    pq_enqueue(&pq, &a);

    /* Dequeue order must be: a (seq 10), then b (seq 11), then c (seq 12) */
    Patient out;
    pq_dequeue(&pq, &out);
    TEST_ASSERT(out.arrival_order == 10 && strcmp(out.token, "TK-0010") == 0,
                "FIFO tie-breaker: Patient A (Arrival Seq 10) dequeued first");

    pq_dequeue(&pq, &out);
    TEST_ASSERT(out.arrival_order == 11 && strcmp(out.token, "TK-0011") == 0,
                "FIFO tie-breaker: Patient B (Arrival Seq 11) dequeued second");

    pq_dequeue(&pq, &out);
    TEST_ASSERT(out.arrival_order == 12 && strcmp(out.token, "TK-0012") == 0,
                "FIFO tie-breaker: Patient C (Arrival Seq 12) dequeued third");

    pq_free(&pq);
}

static void test_retriage_elevation(void) {
    printf(COLOR_BOLD "\n--- Test Suite 4: Clinical Deterioration & Heap Re-balancing ---\n" COLOR_RESET);
    PriorityQueue pq;
    pq_init(&pq, 10);

    Patient p_stable, p_critical;
    patient_init_default(&p_stable);
    strcpy(p_stable.token, "TK-2001");
    p_stable.severity = TRIAGE_LESS_URGENT; /* Level 4 */
    p_stable.arrival_order = 1;

    patient_init_default(&p_critical);
    strcpy(p_critical.token, "TK-2002");
    p_critical.severity = TRIAGE_EMERGENT; /* Level 2 */
    p_critical.arrival_order = 2;

    pq_enqueue(&pq, &p_stable);
    pq_enqueue(&pq, &p_critical);

    /* Patient 2001 suddenly deteriorates to Level 1 (Cardiac arrest) */
    bool updated = pq_update_severity(&pq, "TK-2001", TRIAGE_CRITICAL, "Sudden collapse");
    TEST_ASSERT(updated == true, "Severity update function returns true");

    /* Now TK-2001 should be at the front of queue despite previously being Level 4! */
    Patient top;
    pq_peek(&pq, &top);
    TEST_ASSERT(strcmp(top.token, "TK-2001") == 0 && top.severity == TRIAGE_CRITICAL,
                "Re-triaged patient correctly elevated to top of queue");

    pq_free(&pq);
}

static void test_bed_manager(void) {
    printf(COLOR_BOLD "\n--- Test Suite 5: Emergency Bed Allocation Lifecycle ---\n" COLOR_RESET);
    BedManager bm;
    bed_manager_init(&bm);

    TEST_ASSERT(bm.total_beds == 22, "Bed manager initialized with 22 beds");
    TEST_ASSERT(bed_get_occupied_count(&bm) == 0, "All beds initially vacant");

    Patient crit_patient;
    patient_init_default(&crit_patient);
    strcpy(crit_patient.token, "TK-3001");
    strcpy(crit_patient.name, "Critical Patient");
    crit_patient.severity = TRIAGE_CRITICAL;

    char bed_id[16];
    bool alloc = bed_allocate(&bm, BED_ICU, &crit_patient, bed_id, sizeof(bed_id));
    TEST_ASSERT(alloc == true, "ICU bed allocated successfully");
    TEST_ASSERT(strcmp(bed_id, "ICU-01") == 0, "Allocated bed is ICU-01");
    TEST_ASSERT(bed_get_occupied_count(&bm) == 1, "Occupied count is 1");

    /* Release bed */
    bool rel = bed_release(&bm, "ICU-01");
    TEST_ASSERT(rel == true, "ICU-01 released successfully");
    TEST_ASSERT(bed_get_occupied_count(&bm) == 0, "Occupied count is 0 after release");

    /* Second release should fail */
    TEST_ASSERT(bed_release(&bm, "ICU-01") == false, "Releasing already vacant bed returns false");
}

int main(void) {
    init_terminal_colors();
    printf(COLOR_BOLD "========================================================\n" COLOR_RESET);
    printf(COLOR_BOLD " HOSPITAL EMERGENCY MANAGEMENT SYSTEM - DSA TEST SUITE  \n" COLOR_RESET);
    printf(COLOR_BOLD "========================================================\n" COLOR_RESET);

    test_empty_queue();
    test_min_heap_priority_ordering();
    test_fifo_tie_breaking();
    test_retriage_elevation();
    test_bed_manager();

    printf("\n" COLOR_BOLD "========================================================\n" COLOR_RESET);
    printf(" Results: %d / %d Tests Passed (%.1f%%)\n",
           tests_passed, tests_run, ((double)tests_passed / tests_run) * 100.0);
    printf("========================================================\n" COLOR_RESET);

    return (tests_passed == tests_run) ? 0 : 1;
}
