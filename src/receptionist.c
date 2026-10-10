#include "receptionist.h"
#include "triage.h"

bool receptionist_register_patient(ReceptionistContext *ctx) {
    if (!ctx || !ctx->pq) return false;

    Patient p;
    patient_init_default(&p);

    printf("\n" COLOR_BOLD "-------------------------------------------------------------\n" COLOR_RESET);
    printf(COLOR_CYAN COLOR_BOLD "       EMERGENCY DEPARTMENT - PATIENT REGISTRATION INTAKE    \n" COLOR_RESET);
    printf("-------------------------------------------------------------\n");

    /* 1. Patient Details */
    printf("  Enter Patient Full Name    : ");
    get_input_string(p.name, sizeof(p.name));
    if (strlen(p.name) == 0) {
        printf(COLOR_RED "  [ERROR] Patient name cannot be blank. Registration cancelled.\n" COLOR_RESET);
        return false;
    }

    p.age = get_input_int("  Enter Patient Age (0 - 130): ", 0, 130);

    printf("  Enter Gender (M/F/Other)   : ");
    get_input_string(p.gender, sizeof(p.gender));
    if (strlen(p.gender) == 0) strcpy(p.gender, "Unspecified");

    printf("  Enter Contact Phone Number : ");
    get_input_string(p.contact, sizeof(p.contact));
    if (strlen(p.contact) == 0) strcpy(p.contact, "N/A");

    /* 2. Chief Complaint */
    printf("  Enter Chief Complaint      : ");
    get_input_string(p.chief_complaint, sizeof(p.chief_complaint));
    if (strlen(p.chief_complaint) == 0) {
        printf(COLOR_RED "  [ERROR] Chief complaint is required. Registration cancelled.\n" COLOR_RESET);
        return false;
    }

    /* 3. Automated Triage Suggestion */
    TriageLevel suggested = triage_suggest_level(p.chief_complaint);
    const TriageProtocol *proto = triage_get_protocol(suggested);
    printf("\n  " COLOR_YELLOW "[Clinical Triage Suggestion]" COLOR_RESET " %s%s" COLOR_RESET "\n",
           triage_level_color(suggested), proto ? proto->title : "Level");
    if (proto) {
        printf("  Guideline: %s\n", proto->clinical_description);
    }

    /* 4. Select / Confirm Severity */
    printf("\n  Select Triage Severity Level:\n");
    printf("    1) " COLOR_RED "Level 1 - CRITICAL / RESUSCITATION (Immediate)" COLOR_RESET "\n");
    printf("    2) " COLOR_MAGENTA "Level 2 - EMERGENT (< 10 mins)" COLOR_RESET "\n");
    printf("    3) " COLOR_YELLOW "Level 3 - URGENT (< 30 mins)" COLOR_RESET "\n");
    printf("    4) " COLOR_GREEN "Level 4 - LESS URGENT (< 60 mins)" COLOR_RESET "\n");
    printf("    5) " COLOR_CYAN "Level 5 - NON-URGENT (< 120 mins)" COLOR_RESET "\n");

    char prompt_buf[64];
    snprintf(prompt_buf, sizeof(prompt_buf), "  Confirm Severity (Default: %d) [1-5]: ", (int)suggested);
    int choice = get_input_int(prompt_buf, 1, 5);
    p.severity = (TriageLevel)choice;

    printf("  Enter Triage Notes (optional) : ");
    get_input_string(p.triage_notes, sizeof(p.triage_notes));
    if (strlen(p.triage_notes) == 0) {
        snprintf(p.triage_notes, sizeof(p.triage_notes), "Triage Level %d verified", p.severity);
    }

    /* 5. Token & Metadata Generation */
    p.id = (*ctx->next_patient_id)++;
    (*ctx->global_arrival_counter)++;
    p.arrival_order = *ctx->global_arrival_counter;
    patient_generate_token(p.id, p.token, sizeof(p.token));
    get_current_timestamp(p.arrival_time, sizeof(p.arrival_time));
    p.status = STATUS_WAITING;

    /* 6. Enqueue into Min-Heap Priority Queue */
    if (!pq_enqueue(ctx->pq, &p)) {
        printf(COLOR_RED "  [FATAL] Failed to enqueue patient into priority queue!\n" COLOR_RESET);
        return false;
    }

    /* Update Statistics */
    (*ctx->total_registered_count)++;
    if (ctx->severity_counts && p.severity >= 1 && p.severity <= 5) {
        ctx->severity_counts[p.severity]++;
    }

    /* 7. Save into Persistent Audit File er_patient_log.txt */
    file_log_patient_intake(&p);

    /* 8. Success Output */
    printf("\n" COLOR_GREEN COLOR_BOLD "  [SUCCESS] Patient Registered & Prioritized in Queue!\n" COLOR_RESET);
    patient_format_card(&p);

    return true;
}

void receptionist_search_patient(ReceptionistContext *ctx) {
    if (!ctx || !ctx->pq) return;

    printf("\n  Enter Patient Token (e.g. TK-1001) or Name: ");
    char query[64];
    get_input_string(query, sizeof(query));

    if (strlen(query) == 0) {
        printf(COLOR_RED "  [ERROR] Query cannot be empty.\n" COLOR_RESET);
        return;
    }

    /* Check in Priority Queue */
    Patient found;
    bool in_queue = false;

    if (strncmp(query, "TK-", 3) == 0 || strncmp(query, "tk-", 3) == 0) {
        /* Search by token */
        in_queue = pq_find_by_token(ctx->pq, query, &found);
    }

    if (!in_queue) {
        /* Search by name across queue */
        for (int i = 0; i < ctx->pq->size; i++) {
            if (strstr(ctx->pq->heap[i].name, query) != NULL ||
                strcasecmp(ctx->pq->heap[i].token, query) == 0) {
                found = ctx->pq->heap[i];
                in_queue = true;
                break;
            }
        }
    }

    if (in_queue) {
        printf("\n" COLOR_GREEN "  [RECORD LOCATED IN EMERGENCY QUEUE]\n" COLOR_RESET);
        patient_format_card(&found);
        return;
    }

    /* Check currently serving patient */
    if (ctx->current_serving_patient &&
        (strcasecmp(ctx->current_serving_patient->token, query) == 0 ||
         strstr(ctx->current_serving_patient->name, query) != NULL)) {
        printf("\n" COLOR_YELLOW "  [RECORD LOCATED: CURRENTLY UNDER ACTIVE TREATMENT]\n" COLOR_RESET);
        patient_format_card(ctx->current_serving_patient);
        return;
    }

    /* Check Beds */
    if (ctx->bm) {
        const Bed *b = bed_find_by_token(ctx->bm, query);
        if (b) {
            printf("\n" COLOR_BLUE "  [RECORD LOCATED: CURRENTLY ADMITTED IN EMERGENCY BED %s]\n" COLOR_RESET, b->bed_id);
            printf("  Patient Name : %s\n", b->patient_name);
            printf("  Bed Type     : %s\n", bed_type_to_string(b->type));
            printf("  Admitted At  : %s\n", b->admission_time);
            return;
        }
    }

    printf(COLOR_YELLOW "  [INFO] No active patient found matching '%s' in current emergency care.\n" COLOR_RESET, query);
    printf("  (Check er_patient_log.txt for past discharged records)\n");
}

void receptionist_view_serving_status(const ReceptionistContext *ctx) {
    printf("\n" COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                 EMERGENCY ROOM - CURRENT SERVING STATUS              \n" COLOR_RESET);
    printf("======================================================================\n");

    if (ctx && ctx->current_serving_patient && strlen(ctx->current_serving_patient->token) > 0) {
        const Patient *p = ctx->current_serving_patient;
        printf("  Current Patient Being Treated :\n");
        printf("  Token         : " COLOR_BOLD "%s" COLOR_RESET "\n", p->token);
        printf("  Name          : %s (Age: %d, %s)\n", p->name, p->age, p->gender);
        printf("  Triage Level  : %s%s" COLOR_RESET "\n", triage_level_color(p->severity), triage_level_to_string(p->severity));
        printf("  Complaint     : %s\n", p->chief_complaint);
        printf("  Status        : " COLOR_YELLOW "%s" COLOR_RESET "\n", patient_status_to_string(p->status));
        printf("  Assigned Bed  : %s\n", p->assigned_bed_id);
    } else {
        printf(COLOR_GREEN "  [STATUS] Examination Bay is currently available for next patient.\n" COLOR_RESET);
    }

    if (ctx && ctx->pq) {
        printf("\n  Waiting Queue Depth : %d patients\n", pq_size(ctx->pq));
        Patient next_up;
        if (pq_peek(ctx->pq, &next_up)) {
            printf("  Next Highest-Priority Patient : %s (%s, %s)\n",
                   next_up.token, next_up.name, triage_level_to_string(next_up.severity));
        }
    }
    printf("======================================================================\n");
}

void receptionist_portal_run(ReceptionistContext *ctx) {
    while (true) {
        clear_screen();
        printf(COLOR_CYAN COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf(COLOR_CYAN COLOR_BOLD "          RECEPTIONIST PORTAL - EMERGENCY INTAKE & TRIAGE             \n" COLOR_RESET);
        printf(COLOR_CYAN COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf("  1. Register New Emergency Patient (Intake & Triage Enqueue)\n");
        printf("  2. View Live Priority Waiting Queue\n");
        printf("  3. Check Current Serving Status (Doctor Examination Room)\n");
        printf("  4. Search Patient Record (By Token / Name)\n");
        printf("  5. View Clinical Triage Severity Guide (ESI)\n");
        printf("  6. Return to Main Menu\n");
        printf("----------------------------------------------------------------------\n");

        int choice = get_input_int("  Select Option [1-6]: ", 1, 6);

        switch (choice) {
            case 1:
                receptionist_register_patient(ctx);
                pause_console();
                break;
            case 2:
                pq_display_table(ctx->pq);
                pause_console();
                break;
            case 3:
                receptionist_view_serving_status(ctx);
                pause_console();
                break;
            case 4:
                receptionist_search_patient(ctx);
                pause_console();
                break;
            case 5:
                triage_print_guide();
                pause_console();
                break;
            case 6:
                return;
        }
    }
}
