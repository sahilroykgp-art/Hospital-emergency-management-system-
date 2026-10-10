#ifndef ADMIN_H
#define ADMIN_H

#include "patient.h"
#include "priority_queue.h"
#include "bed_manager.h"
#include "file_io.h"

/* Administrator Operational Context */
typedef struct {
    PriorityQueue *pq;
    BedManager *bm;
    int *total_registered_count;
    int *total_served_count;
    int *severity_counts; /* size 6 */
    Patient *current_serving_patient;
    int *next_patient_id;
    unsigned long *global_arrival_counter;
} AdminContext;

/* Administrator Menu Entry Point */
void admin_portal_run(AdminContext *ctx);

/* Admin Specific Routines */
void admin_display_dashboard(const AdminContext *ctx);
void admin_generate_report(const AdminContext *ctx);
void admin_reset_system(AdminContext *ctx);

#endif /* ADMIN_H */
