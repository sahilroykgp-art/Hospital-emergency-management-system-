#ifndef TRIAGE_H
#define TRIAGE_H

#include "patient.h"

/* Clinical Triage Protocol Definition */
typedef struct {
    TriageLevel level;
    const char *title;
    const char *target_wait_time;
    const char *clinical_description;
    const char *example_conditions;
} TriageProtocol;

/* Returns protocol specification for a given level */
const TriageProtocol* triage_get_protocol(TriageLevel level);

/* Prints interactive Triage reference table */
void triage_print_guide(void);

/* Suggests a triage level based on chief complaint keywords */
TriageLevel triage_suggest_level(const char *complaint);

#endif /* TRIAGE_H */
