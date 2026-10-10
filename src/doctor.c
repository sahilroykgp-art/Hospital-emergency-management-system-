#include "doctor.h"
#include "triage.h"

bool doctor_advance_next_patient(DoctorContext *ctx) {
    if (!ctx || !ctx->pq) return false;

    /* Requirement 17: Check whether any patient is waiting before dequeue */
    if (pq_is_empty(ctx->pq)) {
        printf("\n" COLOR_YELLOW "  [INFO] Priority queue is currently EMPTY. No patients waiting to be advanced.\n" COLOR_RESET);
        return false;
    }

    /* Warn if there is already an active patient being examined */
    if (ctx->current_serving_patient && strlen(ctx->current_serving_patient->token) > 0) {
        printf(COLOR_YELLOW "  [NOTICE] Patient %s (%s) is already under active examination.\n" COLOR_RESET,
               ctx->current_serving_patient->token, ctx->current_serving_patient->name);
        printf("  Please finalize or admit the current patient before advancing the next.\n");
        printf("  Do you wish to force advance next patient anyway? (1 = Yes, 2 = No): ");
        int conf = get_input_int(NULL, 1, 2);
        if (conf != 1) return false;
    }

    /* Dequeue highest priority patient */
    Patient dequeued;
    if (!pq_dequeue(ctx->pq, &dequeued)) {
        printf(COLOR_RED "  [ERROR] Failed to dequeue patient from priority queue.\n" COLOR_RESET);
        return false;
    }

    dequeued.status = STATUS_UNDER_TREATMENT;
    if (ctx->current_serving_patient) {
        *ctx->current_serving_patient = dequeued;
    }
    (*ctx->total_served_count)++;

    /* Log dequeue event to er_patient_log.txt */
    file_log_patient_served(&dequeued, "Called for Examination / Dequeued", "Physician examination commenced", "Pending");

    printf("\n" COLOR_GREEN COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf(COLOR_GREEN COLOR_BOLD "  [CALLING NEXT HIGHEST-PRIORITY PATIENT TO EXAMINATION BAY]          \n" COLOR_RESET);
    printf(COLOR_GREEN COLOR_BOLD "======================================================================\n" COLOR_RESET);
    patient_format_card(&dequeued);
    printf(COLOR_GREEN "  Patient status updated to: UNDER TREATMENT\n" COLOR_RESET);

    return true;
}

void doctor_manage_current_patient(DoctorContext *ctx) {
    if (!ctx || !ctx->current_serving_patient || strlen(ctx->current_serving_patient->token) == 0) {
        printf("\n" COLOR_YELLOW "  [INFO] No patient currently in examination bay. Advance a patient first.\n" COLOR_RESET);
        return;
    }

    Patient *p = ctx->current_serving_patient;

    while (true) {
        clear_screen();
        printf(COLOR_MAGENTA COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf(COLOR_MAGENTA COLOR_BOLD "           CLINICAL MANAGEMENT: PATIENT %s (%s)                      \n" COLOR_RESET,
               p->token, p->name);
        printf(COLOR_MAGENTA COLOR_BOLD "======================================================================\n" COLOR_RESET);
        patient_format_card(p);
        printf("\n  Available Clinical Actions:\n");
        printf("  1. Record / Update Doctor Diagnosis & Clinical Notes\n");
        printf("  2. Assign / Admit Patient to Emergency Bed\n");
        printf("  3. Discharge Patient (Care completed)\n");
        printf("  4. Transfer to Specialty Unit (ICU / OT / Specialty Care)\n");
        printf("  5. Return to Doctor Menu\n");
        printf("----------------------------------------------------------------------\n");

        int act = get_input_int("  Select Action [1-5]: ", 1, 5);

        switch (act) {
            case 1: {
                printf("\n  Enter Clinical Diagnosis & Treatment Notes: ");
                get_input_string(p->doctor_notes, sizeof(p->doctor_notes));
                file_log_patient_served(p, "Doctor Notes Updated", p->doctor_notes, p->assigned_bed_id);
                printf(COLOR_GREEN "  [SUCCESS] Medical notes updated successfully.\n" COLOR_RESET);
                pause_console();
                break;
            }
            case 2: {
                if (strcmp(p->assigned_bed_id, "NONE") != 0 && strlen(p->assigned_bed_id) > 0) {
                    printf(COLOR_YELLOW "  [NOTICE] Patient already assigned to Bed %s.\n" COLOR_RESET, p->assigned_bed_id);
                    pause_console();
                    break;
                }

                printf("\n  Select Bed Type for Admission:\n");
                printf("  1. ICU / Resuscitation Bay (For Level 1 / Critical)\n");
                printf("  2. Trauma Bay (For Major Trauma / Severe Injury)\n");
                printf("  3. Acute ER Bed (For Level 2/3 Acute Care)\n");
                printf("  4. Observation Bed (For Short-stay Monitoring)\n");
                int bchoice = get_input_int("  Select Bed Type [1-4]: ", 1, 4);

                BedType preferred = BED_ACUTE_ER;
                if (bchoice == 1) preferred = BED_ICU;
                else if (bchoice == 2) preferred = BED_TRAUMA;
                else if (bchoice == 3) preferred = BED_ACUTE_ER;
                else if (bchoice == 4) preferred = BED_OBSERVATION;

                char allocated_id[16];
                if (bed_allocate(ctx->bm, preferred, p, allocated_id, sizeof(allocated_id))) {
                    strncpy(p->assigned_bed_id, allocated_id, sizeof(p->assigned_bed_id) - 1);
                    p->status = STATUS_ADMITTED_BED;
                    file_log_patient_served(p, "Admitted to Bed", p->doctor_notes, allocated_id);
                    printf("\n" COLOR_GREEN "  [SUCCESS] Patient successfully admitted to Bed %s!\n" COLOR_RESET, allocated_id);
                } else {
                    printf(COLOR_RED "  [ERROR] No beds of requested type available in Emergency Department!\n" COLOR_RESET);
                }
                pause_console();
                break;
            }
            case 3: {
                printf("\n  Confirm Discharge of Patient %s? (1 = Confirm, 2 = Cancel): ", p->token);
                if (get_input_int(NULL, 1, 2) == 1) {
                    p->status = STATUS_DISCHARGED;
                    if (strcmp(p->assigned_bed_id, "NONE") != 0 && strlen(p->assigned_bed_id) > 0) {
                        bed_release(ctx->bm, p->assigned_bed_id);
                    }
                    file_log_patient_served(p, "Patient Discharged", p->doctor_notes, p->assigned_bed_id);
                    printf("\n" COLOR_GREEN "  [SUCCESS] Patient %s discharged home. Medical record finalized.\n" COLOR_RESET, p->token);
                    /* Clear active serving slot */
                    memset(p, 0, sizeof(Patient));
                    pause_console();
                    return;
                }
                break;
            }
            case 4: {
                printf("\n  Enter Destination Department (e.g. Surgical OT, Cardiac ICU): ");
                char dest[64];
                get_input_string(dest, sizeof(dest));
                p->status = STATUS_TRANSFERRED;
                if (strcmp(p->assigned_bed_id, "NONE") != 0 && strlen(p->assigned_bed_id) > 0) {
                    bed_release(ctx->bm, p->assigned_bed_id);
                }
                char note_buf[384];
                snprintf(note_buf, sizeof(note_buf), "Transferred to: %s. %s", dest, p->doctor_notes);
                file_log_patient_served(p, "Patient Transferred", note_buf, p->assigned_bed_id);
                printf("\n" COLOR_BLUE "  [SUCCESS] Patient %s escalated and transferred to %s.\n" COLOR_RESET, p->token, dest);
                memset(p, 0, sizeof(Patient));
                pause_console();
                return;
            }
            case 5:
                return;
        }
    }
}

void doctor_retriage_waiting_patient(DoctorContext *ctx) {
    if (!ctx || !ctx->pq || pq_is_empty(ctx->pq)) {
        printf("\n" COLOR_YELLOW "  [INFO] No patients currently waiting in queue to re-triage.\n" COLOR_RESET);
        return;
    }

    pq_display_table(ctx->pq);

    printf("\n  Enter Token of Patient whose condition changed (e.g. TK-1001): ");
    char token[16];
    get_input_string(token, sizeof(token));

    Patient target;
    if (!pq_find_by_token(ctx->pq, token, &target)) {
        printf(COLOR_RED "  [ERROR] Patient token %s not found in waiting queue.\n" COLOR_RESET, token);
        return;
    }

    printf("\n  Current Severity: %s\n", triage_level_to_string(target.severity));
    printf("  Select New Severity Level (1-Critical to 5-Non Urgent): ");
    int new_sev = get_input_int(NULL, 1, 5);

    printf("  Enter Reason for Re-triage / Deterioration: ");
    char reason[128];
    get_input_string(reason, sizeof(reason));

    if (pq_update_severity(ctx->pq, token, (TriageLevel)new_sev, reason)) {
        printf("\n" COLOR_GREEN "  [SUCCESS] Patient %s re-triaged to Level %d! Priority queue re-balanced.\n" COLOR_RESET,
               token, new_sev);
    } else {
        printf(COLOR_RED "  [ERROR] Failed to update severity.\n" COLOR_RESET);
    }
}

void doctor_discharge_bed_patient(DoctorContext *ctx) {
    if (!ctx || !ctx->bm) return;

    bed_display_matrix(ctx->bm);

    if (bed_get_occupied_count(ctx->bm) == 0) {
        printf("\n" COLOR_YELLOW "  [INFO] No beds currently occupied.\n" COLOR_RESET);
        return;
    }

    printf("\n  Enter Bed ID to discharge occupant (e.g. ER-01, ICU-02): ");
    char bed_id[16];
    get_input_string(bed_id, sizeof(bed_id));

    const Bed *b = bed_find(ctx->bm, bed_id);
    if (!b || !b->is_occupied) {
        printf(COLOR_RED "  [ERROR] Bed %s is not currently occupied or does not exist.\n" COLOR_RESET, bed_id);
        return;
    }

    char tok[16], name[64];
    strncpy(tok, b->patient_token, sizeof(tok) - 1);
    strncpy(name, b->patient_name, sizeof(name) - 1);

    if (bed_release(ctx->bm, bed_id)) {
        printf("\n" COLOR_GREEN "  [SUCCESS] Bed %s released. Patient %s (%s) discharged.\n" COLOR_RESET,
               bed_id, tok, name);
    } else {
        printf(COLOR_RED "  [ERROR] Failed to release bed.\n" COLOR_RESET);
    }
}

void doctor_portal_run(DoctorContext *ctx) {
    /* Simple Role PIN authentication for Medical Staff */
    printf("\n" COLOR_BOLD "=======================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "        MEDICAL STAFF / CLINICAL PORTAL ACCESS         \n" COLOR_RESET);
    printf("=======================================================\n");
    printf("  Enter Doctor Access PIN (Default: 1234): ");
    char pin[16];
    get_input_string(pin, sizeof(pin));

    if (strcmp(pin, "1234") != 0 && strcmp(pin, "admin") != 0) {
        printf(COLOR_RED "  [ACCESS DENIED] Invalid Doctor PIN. Returning to main menu.\n" COLOR_RESET);
        pause_console();
        return;
    }

    while (true) {
        clear_screen();
        printf(COLOR_MAGENTA COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf(COLOR_MAGENTA COLOR_BOLD "          DOCTOR & CLINICAL CARE PORTAL (ROLE: MEDICAL STAFF)         \n" COLOR_RESET);
        printf(COLOR_MAGENTA COLOR_BOLD "======================================================================\n" COLOR_RESET);
        if (ctx && ctx->current_serving_patient && strlen(ctx->current_serving_patient->token) > 0) {
            printf("  Currently Treating : " COLOR_BOLD "%s (%s) - %s" COLOR_RESET "\n",
                   ctx->current_serving_patient->token,
                   ctx->current_serving_patient->name,
                   triage_level_to_string(ctx->current_serving_patient->severity));
        } else {
            printf("  Currently Treating : (No patient in examination bay)\n");
        }
        printf("----------------------------------------------------------------------\n");
        printf("  1. Advance Highest-Priority Patient (Priority Queue Dequeue)\n");
        printf("  2. Manage Current Examination Bay Patient (Diagnosis, Bed, Discharge)\n");
        printf("  3. View Live Emergency Waiting Queue\n");
        printf("  4. Re-evaluate / Re-triage Waiting Patient (Clinical Escalation)\n");
        printf("  5. View Emergency Beds Matrix & Occupancy\n");
        printf("  6. Discharge / Release Bed by Bed ID\n");
        printf("  7. Return to Main Menu\n");
        printf("----------------------------------------------------------------------\n");

        int choice = get_input_int("  Select Option [1-7]: ", 1, 7);

        switch (choice) {
            case 1:
                doctor_advance_next_patient(ctx);
                pause_console();
                break;
            case 2:
                doctor_manage_current_patient(ctx);
                break;
            case 3:
                pq_display_table(ctx->pq);
                pause_console();
                break;
            case 4:
                doctor_retriage_waiting_patient(ctx);
                pause_console();
                break;
            case 5:
                bed_display_matrix(ctx->bm);
                pause_console();
                break;
            case 6:
                doctor_discharge_bed_patient(ctx);
                pause_console();
                break;
            case 7:
                return;
        }
    }
}
