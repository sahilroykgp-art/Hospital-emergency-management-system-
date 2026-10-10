#ifndef DOCTOR_H
#define DOCTOR_H

#include "patient.h"
#include "priority_queue.h"
#include "bed_manager.h"
#include "file_io.h"

/* Doctor portal context */
typedef struct {
    PriorityQueue *pq;
    BedManager *bm;
    Patient *current_serving_patient;
    int *total_served_count;
} DoctorContext;

/* Doctor Portal Menu */
void doctor_portal_run(DoctorContext *ctx);

/* Doctor Clinical Actions */
bool doctor_advance_next_patient(DoctorContext *ctx);
void doctor_manage_current_patient(DoctorContext *ctx);
void doctor_retriage_waiting_patient(DoctorContext *ctx);
void doctor_discharge_bed_patient(DoctorContext *ctx);

#endif /* DOCTOR_H */
