#ifndef PATIENT_H
#define PATIENT_H

#include "common.h"

/* Triage Severity Levels (Emergency Severity Index) */
typedef enum {
    TRIAGE_CRITICAL    = 1, /* Level 1: Immediate life threat (Resuscitation) */
    TRIAGE_EMERGENT    = 2, /* Level 2: Emergent, high risk (Minutes matter) */
    TRIAGE_URGENT      = 3, /* Level 3: Urgent, stable vitals, complex care */
    TRIAGE_LESS_URGENT = 4, /* Level 4: Less urgent, single resource needed */
    TRIAGE_NON_URGENT  = 5  /* Level 5: Non-urgent, routine evaluation */
} TriageLevel;

/* Patient Care Lifecycle Status */
typedef enum {
    STATUS_WAITING         = 0,
    STATUS_TRIAGED         = 1,
    STATUS_UNDER_TREATMENT = 2,
    STATUS_ADMITTED_BED    = 3,
    STATUS_DISCHARGED      = 4,
    STATUS_TRANSFERRED     = 5
} PatientStatus;

/* Core Patient Record Structure */
typedef struct {
    int id;                        /* Unique numerical record ID */
    char token[16];                /* Formatted Emergency Token (e.g., TK-1001) */
    char name[64];                 /* Full patient name */
    int age;                       /* Patient age in years */
    char gender[16];               /* Gender description */
    char contact[24];              /* Phone number / emergency contact */
    char chief_complaint[128];     /* Chief presenting complaint */
    TriageLevel severity;          /* Priority Level: 1 (Highest) to 5 (Lowest) */
    PatientStatus status;          /* Current treatment lifecycle status */
    unsigned long arrival_order;   /* Strict monotonically increasing sequence for FIFO tie-break */
    char arrival_time[32];         /* Timestamp at intake (YYYY-MM-DD HH:MM:SS) */
    char triage_notes[128];        /* Clinical triage assessment notes */
    char assigned_bed_id[16];      /* Bed identifier if admitted (or "NONE") */
    char doctor_notes[256];        /* Physician examination and diagnosis notes */
} Patient;

/* Function Prototypes */
void patient_init_default(Patient *p);
bool patient_validate(const Patient *p, char *error_msg, size_t err_len);
const char* triage_level_to_string(TriageLevel level);
const char* triage_level_color(TriageLevel level);
const char* patient_status_to_string(PatientStatus status);
void patient_format_card(const Patient *p);
void patient_generate_token(int id, char *out_token, size_t out_len);

#endif /* PATIENT_H */
