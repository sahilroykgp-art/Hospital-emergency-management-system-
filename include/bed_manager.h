#ifndef BED_MANAGER_H
#define BED_MANAGER_H

#include "patient.h"

#define MAX_BEDS 24

typedef enum {
    BED_ICU         = 1, /* Critical / Resuscitation ICU Bed */
    BED_TRAUMA      = 2, /* Trauma Bay */
    BED_ACUTE_ER    = 3, /* Acute Emergency Room Bed */
    BED_OBSERVATION = 4  /* Observation / Semi-urgent Bed */
} BedType;

typedef struct {
    char bed_id[16];
    BedType type;
    bool is_occupied;
    char patient_token[16];
    char patient_name[64];
    int patient_severity;
    char admission_time[32];
} Bed;

typedef struct {
    Bed beds[MAX_BEDS];
    int total_beds;
} BedManager;

/* Bed Manager Lifecycle */
void bed_manager_init(BedManager *bm);
const char* bed_type_to_string(BedType type);

/* Allocation & Discharge */
bool bed_allocate(BedManager *bm, BedType preferred_type, const Patient *p, char *out_bed_id, size_t out_len);
bool bed_release(BedManager *bm, const char *bed_id);
bool bed_release_by_token(BedManager *bm, const char *token);
const Bed* bed_find(const BedManager *bm, const char *bed_id);
const Bed* bed_find_by_token(const BedManager *bm, const char *token);

/* Occupancy Stats */
int bed_get_total_count(const BedManager *bm);
int bed_get_occupied_count(const BedManager *bm);
int bed_get_available_count(const BedManager *bm);
int bed_get_available_by_type(const BedManager *bm, BedType type);

/* Display */
void bed_display_matrix(const BedManager *bm);

#endif /* BED_MANAGER_H */
