#include "file_io.h"

bool file_log_patient_intake(const Patient *p) {
    if (!p) return false;

    FILE *fp = fopen(PATIENT_LOG_FILE, "a");
    if (!fp) {
        printf(COLOR_RED "  [ERROR] Unable to open log file %s for writing!\n" COLOR_RESET, PATIENT_LOG_FILE);
        return false;
    }

    fprintf(fp, "======================================================================\n");
    fprintf(fp, "[EVENT: PATIENT REGISTRATION & TRIAGE INTAKE]\n");
    fprintf(fp, "Timestamp       : %s\n", p->arrival_time);
    fprintf(fp, "Token           : %s (Record #%d)\n", p->token, p->id);
    fprintf(fp, "Arrival Seq     : %lu\n", p->arrival_order);
    fprintf(fp, "Name            : %s\n", p->name);
    fprintf(fp, "Age / Gender    : %d / %s\n", p->age, p->gender);
    fprintf(fp, "Contact         : %s\n", p->contact);
    fprintf(fp, "Chief Complaint : %s\n", p->chief_complaint);
    fprintf(fp, "Triage Level    : Level %d (%s)\n", p->severity, triage_level_to_string(p->severity));
    fprintf(fp, "Triage Notes    : %s\n", p->triage_notes);
    fprintf(fp, "Initial Status  : %s\n", patient_status_to_string(p->status));
    fprintf(fp, "======================================================================\n\n");

    fclose(fp);
    return true;
}

bool file_log_patient_served(const Patient *p, const char *action, const char *doctor_notes, const char *bed_id) {
    if (!p) return false;

    FILE *fp = fopen(PATIENT_LOG_FILE, "a");
    if (!fp) {
        printf(COLOR_RED "  [ERROR] Unable to open log file %s for writing!\n" COLOR_RESET, PATIENT_LOG_FILE);
        return false;
    }

    char now[32];
    get_current_timestamp(now, sizeof(now));

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "[EVENT: CLINICAL ACTION / DEQUEUE]\n");
    fprintf(fp, "Timestamp       : %s\n", now);
    fprintf(fp, "Action Type     : %s\n", action ? action : "Treatment Advanced");
    fprintf(fp, "Token           : %s\n", p->token);
    fprintf(fp, "Patient Name    : %s (Age: %d)\n", p->name, p->age);
    fprintf(fp, "Triage Severity : Level %d\n", p->severity);
    fprintf(fp, "Assigned Bed    : %s\n", bed_id ? bed_id : "None / Ambulatory");
    fprintf(fp, "Doctor's Notes  : %s\n", doctor_notes ? doctor_notes : "Standard care administered");
    fprintf(fp, "Updated Status  : %s\n", patient_status_to_string(p->status));
    fprintf(fp, "----------------------------------------------------------------------\n\n");

    fclose(fp);
    return true;
}

bool file_generate_daily_report(const BedManager *bm,
                                int total_registered,
                                int total_served,
                                int count_level1,
                                int count_level2,
                                int count_level3,
                                int count_level4,
                                int count_level5) {
    FILE *fp = fopen(DAILY_REPORT_FILE, "w");
    if (!fp) {
        printf(COLOR_RED "  [ERROR] Unable to open report file %s for writing!\n" COLOR_RESET, DAILY_REPORT_FILE);
        return false;
    }

    char now[32];
    get_current_timestamp(now, sizeof(now));

    fprintf(fp, "######################################################################\n");
    fprintf(fp, "#             HOSPITAL EMERGENCY DEPARTMENT - DAILY AUDIT REPORT      #\n");
    fprintf(fp, "######################################################################\n\n");
    fprintf(fp, "Report Generated At : %s\n", now);
    fprintf(fp, "Facility Name       : Central Hospital Emergency & Trauma Care\n");
    fprintf(fp, "Operational Status  : Active (24x7 Emergency Services)\n\n");

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "1. PATIENT VOLUME SUMMARY\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Total Patients Registered Today : %d\n", total_registered);
    fprintf(fp, "Total Patients Served / Treated : %d\n", total_served);
    int waiting = total_registered - total_served;
    fprintf(fp, "Patients Currently Waiting/Active: %d\n\n", waiting >= 0 ? waiting : 0);

    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "2. TRIAGE SEVERITY DISTRIBUTION (ESI SCALE)\n");
    fprintf(fp, "----------------------------------------------------------------------\n");
    fprintf(fp, "Level 1 - Resuscitation (Immediate Life Threat) : %3d\n", count_level1);
    fprintf(fp, "Level 2 - Emergent (High Risk / Minutes Matter) : %3d\n", count_level2);
    fprintf(fp, "Level 3 - Urgent (Complex / Stable Vitals)       : %3d\n", count_level3);
    fprintf(fp, "Level 4 - Less Urgent (Single Resource)          : %3d\n", count_level4);
    fprintf(fp, "Level 5 - Non-Urgent (Routine Evaluation)        : %3d\n\n", count_level5);

    if (bm) {
        int occupied = bed_get_occupied_count(bm);
        int total_beds = bed_get_total_count(bm);
        double occ_rate = total_beds > 0 ? ((double)occupied / total_beds) * 100.0 : 0.0;

        fprintf(fp, "----------------------------------------------------------------------\n");
        fprintf(fp, "3. EMERGENCY BED OCCUPANCY STATUS\n");
        fprintf(fp, "----------------------------------------------------------------------\n");
        fprintf(fp, "Total Capacity       : %d Beds\n", total_beds);
        fprintf(fp, "Occupied Beds        : %d Beds\n", occupied);
        fprintf(fp, "Available Beds       : %d Beds\n", total_beds - occupied);
        fprintf(fp, "Occupancy Percentage : %.1f%%\n\n", occ_rate);

        fprintf(fp, "Breakdown by Care Unit:\n");
        fprintf(fp, "  ICU / Resuscitation : %d Available / 4 Total\n", bed_get_available_by_type(bm, BED_ICU));
        fprintf(fp, "  Trauma Bays         : %d Available / 4 Total\n", bed_get_available_by_type(bm, BED_TRAUMA));
        fprintf(fp, "  Acute ER Beds       : %d Available / 8 Total\n", bed_get_available_by_type(bm, BED_ACUTE_ER));
        fprintf(fp, "  Observation Beds    : %d Available / 6 Total\n\n", bed_get_available_by_type(bm, BED_OBSERVATION));

        fprintf(fp, "Currently Occupied Beds List:\n");
        bool any = false;
        for (int i = 0; i < bm->total_beds; i++) {
            if (bm->beds[i].is_occupied) {
                any = true;
                fprintf(fp, "  Bed [%-7s] - Token: %-8s | Patient: %-20s | Admitted: %s\n",
                        bm->beds[i].bed_id,
                        bm->beds[i].patient_token,
                        bm->beds[i].patient_name,
                        bm->beds[i].admission_time);
            }
        }
        if (!any) {
            fprintf(fp, "  (All emergency beds currently vacant)\n");
        }
    }

    fprintf(fp, "\n----------------------------------------------------------------------\n");
    fprintf(fp, "End of Daily Report - End of File\n");
    fprintf(fp, "######################################################################\n");

    fclose(fp);
    return true;
}

void file_view_patient_log(void) {
    FILE *fp = fopen(PATIENT_LOG_FILE, "r");
    if (!fp) {
        printf("\n" COLOR_YELLOW "  [INFO] Patient log file (%s) does not exist yet. No records logged.\n" COLOR_RESET, PATIENT_LOG_FILE);
        return;
    }

    printf("\n" COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                 VIEWING LOG FILE: %s\n" COLOR_RESET, PATIENT_LOG_FILE);
    printf("======================================================================\n");

    char line[512];
    int line_count = 0;
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
        line_count++;
    }
    fclose(fp);

    if (line_count == 0) {
        printf("  (File is currently empty)\n");
    }
    printf("======================================================================\n");
}

void file_view_daily_report(void) {
    FILE *fp = fopen(DAILY_REPORT_FILE, "r");
    if (!fp) {
        printf("\n" COLOR_YELLOW "  [INFO] Daily report file (%s) does not exist yet. Generate one via Admin portal.\n" COLOR_RESET, DAILY_REPORT_FILE);
        return;
    }

    printf("\n" COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                 VIEWING DAILY REPORT: %s\n" COLOR_RESET, DAILY_REPORT_FILE);
    printf("======================================================================\n");

    char line[512];
    int line_count = 0;
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
        line_count++;
    }
    fclose(fp);

    if (line_count == 0) {
        printf("  (File is currently empty)\n");
    }
    printf("======================================================================\n");
}

bool file_reset_logs(void) {
    FILE *fp1 = fopen(PATIENT_LOG_FILE, "w");
    if (fp1) fclose(fp1);

    FILE *fp2 = fopen(DAILY_REPORT_FILE, "w");
    if (fp2) fclose(fp2);

    return true;
}
