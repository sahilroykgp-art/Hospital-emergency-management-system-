#include "patient.h"

void patient_init_default(Patient *p) {
    if (!p) return;
    memset(p, 0, sizeof(Patient));
    p->id = 0;
    strcpy(p->token, "TK-0000");
    strcpy(p->name, "Anonymous");
    p->age = 0;
    strcpy(p->gender, "Unspecified");
    strcpy(p->contact, "N/A");
    strcpy(p->chief_complaint, "None");
    p->severity = TRIAGE_NON_URGENT;
    p->status = STATUS_WAITING;
    p->arrival_order = 0;
    get_current_timestamp(p->arrival_time, sizeof(p->arrival_time));
    strcpy(p->triage_notes, "Routine intake");
    strcpy(p->assigned_bed_id, "NONE");
    strcpy(p->doctor_notes, "Pending examination");
}

bool patient_validate(const Patient *p, char *error_msg, size_t err_len) {
    if (!p) {
        if (error_msg) snprintf(error_msg, err_len, "Null patient record pointer");
        return false;
    }
    if (strlen(p->name) == 0) {
        if (error_msg) snprintf(error_msg, err_len, "Patient name cannot be empty");
        return false;
    }
    if (p->age < 0 || p->age > 130) {
        if (error_msg) snprintf(error_msg, err_len, "Patient age must be between 0 and 130");
        return false;
    }
    if (strlen(p->chief_complaint) == 0) {
        if (error_msg) snprintf(error_msg, err_len, "Chief complaint is mandatory for emergency triage");
        return false;
    }
    if (p->severity < TRIAGE_CRITICAL || p->severity > TRIAGE_NON_URGENT) {
        if (error_msg) snprintf(error_msg, err_len, "Invalid severity level (must be 1 to 5)");
        return false;
    }
    return true;
}

const char* triage_level_to_string(TriageLevel level) {
    switch (level) {
        case TRIAGE_CRITICAL:    return "Level 1 (CRITICAL / RESUSCITATION)";
        case TRIAGE_EMERGENT:    return "Level 2 (EMERGENT)";
        case TRIAGE_URGENT:      return "Level 3 (URGENT)";
        case TRIAGE_LESS_URGENT: return "Level 4 (LESS URGENT)";
        case TRIAGE_NON_URGENT:  return "Level 5 (NON-URGENT)";
        default:                 return "Unknown Severity";
    }
}

const char* triage_level_color(TriageLevel level) {
    switch (level) {
        case TRIAGE_CRITICAL:    return COLOR_RED;
        case TRIAGE_EMERGENT:    return COLOR_MAGENTA;
        case TRIAGE_URGENT:      return COLOR_YELLOW;
        case TRIAGE_LESS_URGENT: return COLOR_GREEN;
        case TRIAGE_NON_URGENT:  return COLOR_CYAN;
        default:                 return COLOR_WHITE;
    }
}

const char* patient_status_to_string(PatientStatus status) {
    switch (status) {
        case STATUS_WAITING:         return "WAITING";
        case STATUS_TRIAGED:         return "TRIAGED";
        case STATUS_UNDER_TREATMENT: return "UNDER TREATMENT";
        case STATUS_ADMITTED_BED:    return "ADMITTED IN BED";
        case STATUS_DISCHARGED:      return "DISCHARGED";
        case STATUS_TRANSFERRED:     return "TRANSFERRED";
        default:                     return "UNKNOWN";
    }
}

void patient_generate_token(int id, char *out_token, size_t out_len) {
    if (!out_token || out_len == 0) return;
    snprintf(out_token, out_len, "TK-%04d", id);
}

void patient_format_card(const Patient *p) {
    if (!p) return;
    const char *color = triage_level_color(p->severity);

    printf("%s+-------------------------------------------------------------+\n", color);
    printf("| [EMERGENCY PATIENT CARD]           Token: %-17s |\n", p->token);
    printf("+-------------------------------------------------------------+\n" COLOR_RESET);
    printf("  Record ID       : #%d\n", p->id);
    printf("  Full Name       : " COLOR_BOLD "%s" COLOR_RESET " (%d yrs, %s)\n", p->name, p->age, p->gender);
    printf("  Contact         : %s\n", p->contact);
    printf("  Intake Time     : %s\n", p->arrival_time);
    printf("  Arrival Seq #   : %lu\n", p->arrival_order);
    printf("  Chief Complaint : " COLOR_BOLD "%s" COLOR_RESET "\n", p->chief_complaint);
    printf("  Triage Priority : %s%s" COLOR_RESET "\n", color, triage_level_to_string(p->severity));
    printf("  Triage Notes    : %s\n", p->triage_notes);
    printf("  Status          : " COLOR_BOLD "%s" COLOR_RESET "\n", patient_status_to_string(p->status));
    printf("  Assigned Bed    : %s\n", p->assigned_bed_id);
    printf("  Doctor's Notes  : %s\n", p->doctor_notes);
    printf("%s+-------------------------------------------------------------+\n" COLOR_RESET, color);
}
