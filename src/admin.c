#include "admin.h"

void admin_display_dashboard(const AdminContext *ctx) {
    if (!ctx) return;

    int total_reg = ctx->total_registered_count ? *ctx->total_registered_count : 0;
    int total_srv = ctx->total_served_count ? *ctx->total_served_count : 0;
    int in_queue = ctx->pq ? pq_size(ctx->pq) : 0;
    int occupied_beds = ctx->bm ? bed_get_occupied_count(ctx->bm) : 0;
    int total_beds = ctx->bm ? bed_get_total_count(ctx->bm) : 0;
    double bed_pct = total_beds > 0 ? ((double)occupied_beds / total_beds) * 100.0 : 0.0;

    printf("\n" COLOR_BOLD "========================================================================================\n" COLOR_RESET);
    printf(COLOR_YELLOW COLOR_BOLD "              EXECUTIVE EMERGENCY DEPARTMENT ANALYTICS DASHBOARD                        \n" COLOR_RESET);
    printf("========================================================================================\n");

    printf("  " COLOR_BOLD "1. THROUGHPUT & CAPACITY METRICS" COLOR_RESET "\n");
    printf("     Total Patients Registered Today : " COLOR_CYAN "%d" COLOR_RESET "\n", total_reg);
    printf("     Total Patients Processed/Served : " COLOR_GREEN "%d" COLOR_RESET "\n", total_srv);
    printf("     Current Queue Waiting Depth     : " COLOR_YELLOW "%d" COLOR_RESET "\n", in_queue);
    printf("     Bed Occupancy Rate              : %s%d / %d (%.1f%%)" COLOR_RESET "\n\n",
           bed_pct > 80.0 ? COLOR_RED : COLOR_GREEN, occupied_beds, total_beds, bed_pct);

    printf("  " COLOR_BOLD "2. TRIAGE ACUITY BREAKDOWN (ESI SCALE)" COLOR_RESET "\n");
    if (ctx->severity_counts) {
        int l1 = ctx->severity_counts[1];
        int l2 = ctx->severity_counts[2];
        int l3 = ctx->severity_counts[3];
        int l4 = ctx->severity_counts[4];
        int l5 = ctx->severity_counts[5];
        printf("     " COLOR_RED    "Level 1 (Resuscitation - Immediate) : %3d" COLOR_RESET "\n", l1);
        printf("     " COLOR_MAGENTA"Level 2 (Emergent - High Risk)      : %3d" COLOR_RESET "\n", l2);
        printf("     " COLOR_YELLOW "Level 3 (Urgent - Multi-resource)   : %3d" COLOR_RESET "\n", l3);
        printf("     " COLOR_GREEN  "Level 4 (Less Urgent - Low Risk)    : %3d" COLOR_RESET "\n", l4);
        printf("     " COLOR_CYAN   "Level 5 (Non-Urgent - Routine)      : %3d" COLOR_RESET "\n", l5);
    }

    if (ctx->bm) {
        printf("\n  " COLOR_BOLD "3. UNIT-LEVEL BED ALLOCATION" COLOR_RESET "\n");
        printf("     ICU / Resuscitation Beds : %d Vacant / 4 Total\n", bed_get_available_by_type(ctx->bm, BED_ICU));
        printf("     Trauma Bay Beds          : %d Vacant / 4 Total\n", bed_get_available_by_type(ctx->bm, BED_TRAUMA));
        printf("     Acute ER Beds            : %d Vacant / 8 Total\n", bed_get_available_by_type(ctx->bm, BED_ACUTE_ER));
        printf("     Observation Beds         : %d Vacant / 6 Total\n", bed_get_available_by_type(ctx->bm, BED_OBSERVATION));
    }
    printf("========================================================================================\n");
}

void admin_generate_report(const AdminContext *ctx) {
    if (!ctx) return;

    int total_reg = ctx->total_registered_count ? *ctx->total_registered_count : 0;
    int total_srv = ctx->total_served_count ? *ctx->total_served_count : 0;
    int l1 = ctx->severity_counts ? ctx->severity_counts[1] : 0;
    int l2 = ctx->severity_counts ? ctx->severity_counts[2] : 0;
    int l3 = ctx->severity_counts ? ctx->severity_counts[3] : 0;
    int l4 = ctx->severity_counts ? ctx->severity_counts[4] : 0;
    int l5 = ctx->severity_counts ? ctx->severity_counts[5] : 0;

    if (file_generate_daily_report(ctx->bm, total_reg, total_srv, l1, l2, l3, l4, l5)) {
        printf("\n" COLOR_GREEN COLOR_BOLD "  [SUCCESS] Official Daily Report generated successfully into '%s'!\n" COLOR_RESET, DAILY_REPORT_FILE);
    } else {
        printf(COLOR_RED "  [ERROR] Failed to generate daily report.\n" COLOR_RESET);
    }
}

void admin_reset_system(AdminContext *ctx) {
    printf("\n" COLOR_RED COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf(COLOR_RED COLOR_BOLD "  [DANGER ZONE] SYSTEM RESET & DAILY AUDIT PURGE                       \n" COLOR_RESET);
    printf(COLOR_RED COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf("  This operation will:\n");
    printf("  - Purge er_patient_log.txt and daily_er_report.txt\n");
    printf("  - Clear all waiting patients in priority queue\n");
    printf("  - Release all occupied beds\n");
    printf("  - Reset daily counters back to zero\n\n");
    printf("  Type 'CONFIRM' to proceed with reset: ");

    char conf[32];
    get_input_string(conf, sizeof(conf));

    if (strcmp(conf, "CONFIRM") != 0) {
        printf(COLOR_YELLOW "  [CANCELLED] Reset operation aborted by user.\n" COLOR_RESET);
        return;
    }

    /* Reset files */
    file_reset_logs();

    /* Clear queue */
    if (ctx->pq) pq_clear(ctx->pq);

    /* Reinitialize beds */
    if (ctx->bm) bed_manager_init(ctx->bm);

    /* Reset counters */
    if (ctx->total_registered_count) *ctx->total_registered_count = 0;
    if (ctx->total_served_count) *ctx->total_served_count = 0;
    if (ctx->severity_counts) {
        for (int i = 0; i <= 5; i++) ctx->severity_counts[i] = 0;
    }
    if (ctx->current_serving_patient) {
        memset(ctx->current_serving_patient, 0, sizeof(Patient));
    }
    if (ctx->next_patient_id) *ctx->next_patient_id = 1001;
    if (ctx->global_arrival_counter) *ctx->global_arrival_counter = 0;

    printf("\n" COLOR_GREEN "  [SUCCESS] Hospital Emergency Management System reset for new day.\n" COLOR_RESET);
}

void admin_portal_run(AdminContext *ctx) {
    printf("\n" COLOR_BOLD "=======================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "            HOSPITAL ADMIN PORTAL SECURITY             \n" COLOR_RESET);
    printf("=======================================================\n");
    printf("  Enter Administrator PIN (Default: 9999): ");
    char pin[16];
    get_input_string(pin, sizeof(pin));

    if (strcmp(pin, "9999") != 0 && strcmp(pin, "admin") != 0) {
        printf(COLOR_RED "  [ACCESS DENIED] Invalid Administrator PIN. Returning to main menu.\n" COLOR_RESET);
        pause_console();
        return;
    }

    while (true) {
        clear_screen();
        printf(COLOR_YELLOW COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf(COLOR_YELLOW COLOR_BOLD "         ADMINISTRATOR & SYSTEM OPERATIONS (ROLE: ADMIN)              \n" COLOR_RESET);
        printf(COLOR_YELLOW COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf("  1. Emergency Department Analytics Dashboard\n");
        printf("  2. View Emergency Bed Matrix & Unit Allocation\n");
        printf("  3. Generate / Export Daily Audit Report (daily_er_report.txt)\n");
        printf("  4. View Patient Audit Log (er_patient_log.txt)\n");
        printf("  5. View Daily Report on Screen (daily_er_report.txt)\n");
        printf("  6. View Live Priority Waiting Queue\n");
        printf("  7. Reset Daily Logs & Purge Emergency Records (End-of-Day)\n");
        printf("  8. Return to Main Menu\n");
        printf("----------------------------------------------------------------------\n");

        int choice = get_input_int("  Select Option [1-8]: ", 1, 8);

        switch (choice) {
            case 1:
                admin_display_dashboard(ctx);
                pause_console();
                break;
            case 2:
                bed_display_matrix(ctx->bm);
                pause_console();
                break;
            case 3:
                admin_generate_report(ctx);
                pause_console();
                break;
            case 4:
                file_view_patient_log();
                pause_console();
                break;
            case 5:
                file_view_daily_report();
                pause_console();
                break;
            case 6:
                pq_display_table(ctx->pq);
                pause_console();
                break;
            case 7:
                admin_reset_system(ctx);
                pause_console();
                break;
            case 8:
                return;
        }
    }
}
