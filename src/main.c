#include "common.h"
#include "patient.h"
#include "triage.h"
#include "priority_queue.h"
#include "bed_manager.h"
#include "file_io.h"
#include "receptionist.h"
#include "doctor.h"
#include "admin.h"

/* Preload realistic emergency simulation patients for live demonstration */
static void load_demo_scenario(PriorityQueue *pq,
                               int *next_patient_id,
                               unsigned long *arrival_counter,
                               int *total_registered,
                               int *severity_counts) {
    if (!pq || !next_patient_id || !arrival_counter) return;

    struct DemoCase {
        const char *name;
        int age;
        const char *gender;
        const char *contact;
        const char *complaint;
        TriageLevel severity;
        const char *notes;
    } cases[] = {
        {
            .name = "Vikram Malhotra",
            .age = 58,
            .gender = "Male",
            .contact = "+91-9811223344",
            .complaint = "Acute crushing chest pain radiating to left arm (suspected STEMI)",
            .severity = TRIAGE_EMERGENT,
            .notes = "ECG urgently needed; oxygen administered"
        },
        {
            .name = "Aarav Sharma",
            .age = 22,
            .gender = "Male",
            .contact = "+91-9876543210",
            .complaint = "Deep forearm laceration with active bleeding from glass cut",
            .severity = TRIAGE_LESS_URGENT,
            .notes = "Pressure dressing applied; vitals stable"
        },
        {
            .name = "Rajeshwari Devi",
            .age = 67,
            .gender = "Female",
            .contact = "+91-9455112233",
            .complaint = "Sudden collapse, unresponsive, suspected cardiac arrest / VFib",
            .severity = TRIAGE_CRITICAL,
            .notes = "Immediate CPR in progress, defibrillator mobilized"
        },
        {
            .name = "Priya Nambiar",
            .age = 34,
            .gender = "Female",
            .contact = "+91-9988776655",
            .complaint = "Severe lower right quadrant abdominal pain with fever (Acute appendicitis)",
            .severity = TRIAGE_URGENT,
            .notes = "IV line started, ultrasound requested"
        },
        {
            .name = "Suresh Sen",
            .age = 45,
            .gender = "Male",
            .contact = "+91-9123456780",
            .complaint = "High-speed motorcycle collision, multiple blunt polytrauma",
            .severity = TRIAGE_CRITICAL,
            .notes = "Airway secured, cervical collar on; immediate trauma bay alert"
        },
        {
            .name = "Ananya Roy",
            .age = 19,
            .gender = "Female",
            .contact = "+91-9011223344",
            .complaint = "Mild migraine and requested prescription refill for asthma inhaler",
            .severity = TRIAGE_NON_URGENT,
            .notes = "Vitals normal, ambulatory"
        }
    };

    int num_cases = (int)(sizeof(cases) / sizeof(cases[0]));
    printf("\n" COLOR_CYAN "  [SIMULATION] Injecting %d emergency patients into Priority Queue...\n" COLOR_RESET, num_cases);

    for (int i = 0; i < num_cases; i++) {
        Patient p;
        patient_init_default(&p);
        p.id = (*next_patient_id)++;
        (*arrival_counter)++;
        p.arrival_order = *arrival_counter;
        patient_generate_token(p.id, p.token, sizeof(p.token));
        strncpy(p.name, cases[i].name, sizeof(p.name) - 1);
        p.age = cases[i].age;
        strncpy(p.gender, cases[i].gender, sizeof(p.gender) - 1);
        strncpy(p.contact, cases[i].contact, sizeof(p.contact) - 1);
        strncpy(p.chief_complaint, cases[i].complaint, sizeof(p.chief_complaint) - 1);
        p.severity = cases[i].severity;
        strncpy(p.triage_notes, cases[i].notes, sizeof(p.triage_notes) - 1);
        p.status = STATUS_WAITING;

        pq_enqueue(pq, &p);
        file_log_patient_intake(&p);

        if (total_registered) (*total_registered)++;
        if (severity_counts && p.severity >= 1 && p.severity <= 5) {
            severity_counts[p.severity]++;
        }

        printf("  + Added: %-8s | %-18s | %s%s" COLOR_RESET "\n",
               p.token, p.name, triage_level_color(p.severity), triage_level_to_string(p.severity));
    }

    printf(COLOR_GREEN "  [SUCCESS] Demo scenario loaded! Notice how Level 1 patients will jump to front of queue!\n" COLOR_RESET);
}

/* Public Overview Board */
static void show_public_overview_board(const PriorityQueue *pq, const BedManager *bm, const Patient *curr) {
    clear_screen();
    printf(COLOR_BOLD "========================================================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                 HOSPITAL EMERGENCY DEPARTMENT - LIVE PUBLIC STATUS BOARD                                \n" COLOR_RESET);
    printf("========================================================================================================\n");

    if (curr && strlen(curr->token) > 0) {
        printf("  [CURRENTLY UNDER TREATMENT IN EXAMINATION BAY]:\n");
        printf("  Token: " COLOR_BOLD "%s" COLOR_RESET " | Name: %s | Triage: %s%s" COLOR_RESET " | Status: %s\n",
               curr->token, curr->name, triage_level_color(curr->severity),
               triage_level_to_string(curr->severity), patient_status_to_string(curr->status));
    } else {
        printf("  [EXAMINATION BAY]: " COLOR_GREEN "Ready for Next Patient" COLOR_RESET "\n");
    }
    printf("--------------------------------------------------------------------------------------------------------\n");

    pq_display_table(pq);
    bed_display_matrix(bm);
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
                show_public_overview_board(&pq, &bm, &current_serving_patient);
                pause_console();
                break;
            }
            case 5: {
                load_demo_scenario(&pq, &next_patient_id, &global_arrival_counter, &total_registered, severity_counts);
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

    pq_free(&pq);
    return 0;
}
