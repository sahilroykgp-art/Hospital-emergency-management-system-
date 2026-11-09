#include "common.h"
#include "patient.h"
#include "priority_queue.h"
#include "bed_manager.h"
#include "file_io.h"
#include "triage.h"
#include "receptionist.h"
#include "doctor.h"
#include "admin.h"

/* Structure for realistic demo patients */
typedef struct {
    const char *name;
    int age;
    const char *gender;
    const char *contact;
    const char *complaint;
    TriageLevel severity;
    const char *notes;
} DemoPatient;

/* 6 Realistic Clinical Demo Cases for Instant Simulation */
static const DemoPatient DEMO_PATIENTS[] = {
    {
        .name = "Vikram Malhotra",
        .age = 58,
        .gender = "Male",
        .contact = "+91-9811223344",
        .complaint = "Acute crushing chest pain (suspected STEMI)",
        .severity = TRIAGE_EMERGENT,
        .notes = "ECG urgently needed; oxygen administered"
    },
    {
        .name = "Aarav Sharma",
        .age = 22,
        .gender = "Male",
        .contact = "+91-9876543210",
        .complaint = "Deep forearm laceration with active bleeding",
        .severity = TRIAGE_LESS_URGENT,
        .notes = "Pressure dressing applied; vitals stable"
    },
    {
        .name = "Rajeshwari Devi",
        .age = 67,
        .gender = "Female",
        .contact = "+91-9455112233",
        .complaint = "Sudden collapse, unresponsive, suspected cardiac arrest",
        .severity = TRIAGE_CRITICAL,
        .notes = "Immediate CPR, defibrillator mobilized"
    },
    {
        .name = "Priya Nambiar",
        .age = 34,
        .gender = "Female",
        .contact = "+91-9988776655",
        .complaint = "Severe lower abdominal pain (Acute appendicitis)",
        .severity = TRIAGE_URGENT,
        .notes = "IV line started, ultrasound requested"
    },
    {
        .name = "Suresh Sen",
        .age = 45,
        .gender = "Male",
        .contact = "+91-9123456780",
        .complaint = "High-speed motorcycle collision polytrauma",
        .severity = TRIAGE_CRITICAL,
        .notes = "Airway secured, cervical collar on; immediate trauma bay alert"
    },
    {
        .name = "Ananya Roy",
        .age = 19,
        .gender = "Female",
        .contact = "+91-9011223344",
        .complaint = "Mild migraine and inhaler prescription refill",
        .severity = TRIAGE_NON_URGENT,
        .notes = "Vitals normal, ambulatory"
    }
};

/* Loads demo patients into priority queue to easily demonstrate Min-Heap sorting */
static void load_demo_patients(PriorityQueue *pq,
                              int *next_patient_id,
                              unsigned long *global_arrival_counter,
                              int *total_registered,
                              int *severity_counts) {
    if (!pq || !next_patient_id || !global_arrival_counter) return;

    printf("\n" COLOR_CYAN COLOR_BOLD "======================================================================\n" COLOR_RESET);
    printf(COLOR_CYAN COLOR_BOLD "       LOADING 6 REALISTIC EMERGENCY CASES INTO MIN-HEAP QUEUE        \n" COLOR_RESET);
    printf(COLOR_CYAN COLOR_BOLD "======================================================================\n" COLOR_RESET);

    size_t count = sizeof(DEMO_PATIENTS) / sizeof(DEMO_PATIENTS[0]);
    for (size_t i = 0; i < count; i++) {
        Patient p;
        patient_init_default(&p);

        p.id = (*next_patient_id)++;
        (*global_arrival_counter)++;
        p.arrival_order = *global_arrival_counter;
        patient_generate_token(p.id, p.token, sizeof(p.token));
        get_current_timestamp(p.arrival_time, sizeof(p.arrival_time));

        strncpy(p.name, DEMO_PATIENTS[i].name, sizeof(p.name) - 1);
        p.age = DEMO_PATIENTS[i].age;
        strncpy(p.gender, DEMO_PATIENTS[i].gender, sizeof(p.gender) - 1);
        strncpy(p.contact, DEMO_PATIENTS[i].contact, sizeof(p.contact) - 1);
        strncpy(p.chief_complaint, DEMO_PATIENTS[i].complaint, sizeof(p.chief_complaint) - 1);
        p.severity = DEMO_PATIENTS[i].severity;
        strncpy(p.triage_notes, DEMO_PATIENTS[i].notes, sizeof(p.triage_notes) - 1);
        p.status = STATUS_WAITING;

        if (pq_enqueue(pq, &p)) {
            (*total_registered)++;
            if (severity_counts && p.severity >= 1 && p.severity <= 5) {
                severity_counts[p.severity]++;
            }
            file_log_patient_intake(&p);

            const char *col = triage_level_color(p.severity);
            printf("  [+] Enqueued %-9s %-20.20s (%s%-12s" COLOR_RESET ") -> %s\n",
                   p.token, p.name, col, triage_level_to_string(p.severity), p.chief_complaint);
        }
    }

    printf("----------------------------------------------------------------------\n");
    printf(COLOR_GREEN "  Successfully loaded 6 emergency patients into priority queue!\n" COLOR_RESET);
    printf("  (Notice: Patients are automatically sorted by Severity 1 -> 5 in Min-Heap)\n");
}

/* Displays Live Public Overview Board */
static void show_live_monitor(const PriorityQueue *pq, const BedManager *bm, const Patient *curr) {
    clear_screen();

    char now[32];
    get_current_timestamp(now, sizeof(now));

    int waiting_count = pq ? pq_size(pq) : 0;
    int occupied_beds = bm ? bed_get_occupied_count(bm) : 0;
    int total_beds = bm ? bed_get_total_count(bm) : 22;

    printf(COLOR_CYAN COLOR_BOLD "========================================================================================================\n" COLOR_RESET);
    printf(COLOR_CYAN COLOR_BOLD "             CENTRAL HOSPITAL EMERGENCY DEPARTMENT - LIVE STATUS BOARD                                 \n" COLOR_RESET);
    printf(COLOR_CYAN COLOR_BOLD "========================================================================================================\n" COLOR_RESET);
    printf("  System Time: " COLOR_BOLD "%s" COLOR_RESET "  |  Service Status: " COLOR_GREEN "ONLINE (24x7 Emergency Services)" COLOR_RESET "\n", now);
    printf("  Queue Volume: " COLOR_YELLOW "%d Patients Waiting" COLOR_RESET "  |  Bed Occupancy: " COLOR_GREEN "%d / %d Beds (%.1f%% Occupied)" COLOR_RESET "\n",
           waiting_count, occupied_beds, total_beds,
           total_beds > 0 ? ((float)occupied_beds / (float)total_beds) * 100.0f : 0.0f);
    printf("--------------------------------------------------------------------------------------------------------\n");

    /* Active Examination Bay */
    if (curr && strlen(curr->token) > 0) {
        const char *triage_col = triage_level_color(curr->severity);
        printf("  " COLOR_BOLD "[ACTIVE RESUSCITATION & CLINICAL EXAMINATION BAY]:" COLOR_RESET "\n");
        printf("  +--------------------------------------------------------------------------------------------------+\n");
        printf("  | Token        : " COLOR_BOLD "%-16s" COLOR_RESET " | Name     : %-22s (%d yrs, %-6s) |\n",
               curr->token, curr->name, curr->age, curr->gender);
        printf("  | Acuity Level : %s%-20s" COLOR_RESET " | Status   : %-37s |\n",
               triage_col, triage_level_to_string(curr->severity), patient_status_to_string(curr->status));
        printf("  | Complaint    : %-81.81s |\n", curr->chief_complaint);
        printf("  | Assigned Bed : %-16s | Clinical : %-37.37s |\n",
               strlen(curr->assigned_bed_id) > 0 ? curr->assigned_bed_id : "None (In Bay)",
               strlen(curr->doctor_notes) > 0 ? curr->doctor_notes : "Under active evaluation");
        printf("  +--------------------------------------------------------------------------------------------------+\n");
    } else {
        printf("  " COLOR_BOLD "[EXAMINATION BAY]:" COLOR_RESET " " COLOR_GREEN "Currently Vacant & Prepared for Next Priority Patient" COLOR_RESET "\n");
    }
    printf("--------------------------------------------------------------------------------------------------------\n");

    /* Priority Queue Table */
    if (pq) {
        pq_display_table(pq);
    }

    /* Beds Matrix */
    if (bm) {
        bed_display_matrix(bm);
    }
}

int main(void) {
    /* Initialize Virtual Terminal Sequences for colored output */
    init_terminal_colors();

    /* System Core State */
    PriorityQueue pq;
    pq_init(&pq, 64);

    BedManager bm;
    bed_manager_init(&bm);

    int next_patient_id = 1001;
    unsigned long global_arrival_counter = 0;
    int total_registered = 0;
    int total_served = 0;
    int severity_counts[6] = {0};
    Patient current_serving_patient;
    memset(&current_serving_patient, 0, sizeof(Patient));

    while (true) {
        clear_screen();
        printf(COLOR_CYAN COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf(COLOR_CYAN COLOR_BOLD "            HOSPITAL EMERGENCY MANAGEMENT SYSTEM                      \n" COLOR_RESET);
        printf(COLOR_CYAN COLOR_BOLD "           (Data Structures & Algorithms - Priority Queue)            \n" COLOR_RESET);
        printf(COLOR_CYAN COLOR_BOLD "======================================================================\n" COLOR_RESET);
        printf(COLOR_WHITE "  Queue Status: " COLOR_YELLOW "%d Waiting" COLOR_WHITE "  |  Beds Occupied: " COLOR_GREEN "%d/%d" COLOR_WHITE "  |  Served Today: " COLOR_CYAN "%d\n" COLOR_RESET,
               pq_size(&pq), bed_get_occupied_count(&bm), bed_get_total_count(&bm), total_served);
        printf("----------------------------------------------------------------------\n");
        printf("  1. " COLOR_BOLD "Receptionist Portal" COLOR_RESET "      [Register, Intake Triage, Live Queue]\n");
        printf("  2. " COLOR_BOLD "Medical Staff / Doctor" COLOR_RESET "   [Priority Dequeue, Diagnosis, Bed Admin]\n");
        printf("  3. " COLOR_BOLD "Administrator Portal" COLOR_RESET "     [Analytics, Reports, Reset Logs]\n");
        printf("  4. " COLOR_BOLD "Live Public ER Monitor" COLOR_RESET "   [Live Queue Board & Bed Matrix]\n");
        printf("  5. " COLOR_BOLD "Load Demo Emergency Data" COLOR_RESET " [Simulate 6 Realistic Emergency Cases]\n");
        printf("  6. " COLOR_BOLD "Triage Reference Guide" COLOR_RESET "   [View Clinical ESI Protocol]\n");
        printf("  7. " COLOR_BOLD "Exit System" COLOR_RESET "\n");
        printf("======================================================================\n");

        int choice = get_input_int("  Select Access Module [1-7]: ", 1, 7);

        switch (choice) {
            case 1: {
                ReceptionistContext rctx = {
                    .pq = &pq,
                    .bm = &bm,
                    .next_patient_id = &next_patient_id,
                    .global_arrival_counter = &global_arrival_counter,
                    .total_registered_count = &total_registered,
                    .severity_counts = severity_counts,
                    .current_serving_patient = &current_serving_patient
                };
                receptionist_portal_run(&rctx);
                break;
            }
            case 2: {
                DoctorContext dctx = {
                    .pq = &pq,
                    .bm = &bm,
                    .current_serving_patient = &current_serving_patient,
                    .total_served_count = &total_served
                };
                doctor_portal_run(&dctx);
                break;
            }
            case 3: {
                AdminContext actx = {
                    .pq = &pq,
                    .bm = &bm,
                    .total_registered_count = &total_registered,
                    .total_served_count = &total_served,
                    .severity_counts = severity_counts,
                    .current_serving_patient = &current_serving_patient,
                    .next_patient_id = &next_patient_id,
                    .global_arrival_counter = &global_arrival_counter
                };
                admin_portal_run(&actx);
                break;
            }
            case 4: {
                show_live_monitor(&pq, &bm, &current_serving_patient);
                pause_console();
                break;
            }
            case 5: {
                load_demo_patients(&pq, &next_patient_id, &global_arrival_counter, &total_registered, severity_counts);
                pause_console();
                break;
            }
            case 6: {
                triage_print_guide();
                pause_console();
                break;
            }
            case 7: {
                printf("\n" COLOR_GREEN "  Exiting Hospital Emergency Management System. Thank you!\n" COLOR_RESET);
                pq_free(&pq);
                return 0;
            }
        }
    }
}
