#ifndef RECEPTIONIST_H
#define RECEPTIONIST_H

#include "patient.h"
#include "priority_queue.h"
#include "bed_manager.h"
#include "file_io.h"

/* Operational context passed to receptionist portal */
typedef struct {
    PriorityQueue *pq;
    BedManager *bm;
    int *next_patient_id;
    unsigned long *global_arrival_counter;
    int *total_registered_count;
    int *severity_counts; /* array of size 6 for levels 1..5 */
    const Patient *current_serving_patient;
} ReceptionistContext;

/* Receptionist Menu Entry Point */
void receptionist_portal_run(ReceptionistContext *ctx);

/* Specific Operations */
bool receptionist_register_patient(ReceptionistContext *ctx);
void receptionist_search_patient(ReceptionistContext *ctx);
void receptionist_view_serving_status(const ReceptionistContext *ctx);

#endif /* RECEPTIONIST_H */
