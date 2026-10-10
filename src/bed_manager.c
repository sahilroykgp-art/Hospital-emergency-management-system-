#include "bed_manager.h"

void bed_manager_init(BedManager *bm) {
    if (!bm) return;
    bm->total_beds = 0;

    /* 4 ICU Beds */
    for (int i = 1; i <= 4; i++) {
        Bed *b = &bm->beds[bm->total_beds++];
        snprintf(b->bed_id, sizeof(b->bed_id), "ICU-%02d", i);
        b->type = BED_ICU;
        b->is_occupied = false;
        b->patient_token[0] = '\0';
        b->patient_name[0] = '\0';
        b->patient_severity = 0;
        b->admission_time[0] = '\0';
    }

    /* 4 Trauma Bay Beds */
    for (int i = 1; i <= 4; i++) {
        Bed *b = &bm->beds[bm->total_beds++];
        snprintf(b->bed_id, sizeof(b->bed_id), "TRM-%02d", i);
        b->type = BED_TRAUMA;
        b->is_occupied = false;
        b->patient_token[0] = '\0';
        b->patient_name[0] = '\0';
        b->patient_severity = 0;
        b->admission_time[0] = '\0';
    }

    /* 8 Acute ER Beds */
    for (int i = 1; i <= 8; i++) {
        Bed *b = &bm->beds[bm->total_beds++];
        snprintf(b->bed_id, sizeof(b->bed_id), "ER-%02d", i);
        b->type = BED_ACUTE_ER;
        b->is_occupied = false;
        b->patient_token[0] = '\0';
        b->patient_name[0] = '\0';
        b->patient_severity = 0;
        b->admission_time[0] = '\0';
    }

    /* 6 Observation Beds */
    for (int i = 1; i <= 6; i++) {
        Bed *b = &bm->beds[bm->total_beds++];
        snprintf(b->bed_id, sizeof(b->bed_id), "OBS-%02d", i);
        b->type = BED_OBSERVATION;
        b->is_occupied = false;
        b->patient_token[0] = '\0';
        b->patient_name[0] = '\0';
        b->patient_severity = 0;
        b->admission_time[0] = '\0';
    }
}

const char* bed_type_to_string(BedType type) {
    switch (type) {
        case BED_ICU:         return "ICU / Resuscitation";
        case BED_TRAUMA:      return "Trauma Bay";
        case BED_ACUTE_ER:    return "Acute ER Bed";
        case BED_OBSERVATION: return "Observation Bed";
        default:              return "General Bed";
    }
}

bool bed_allocate(BedManager *bm, BedType preferred_type, const Patient *p, char *out_bed_id, size_t out_len) {
    if (!bm || !p) return false;

    /* First pass: match exact preferred bed type */
    for (int i = 0; i < bm->total_beds; i++) {
        Bed *b = &bm->beds[i];
        if (b->type == preferred_type && !b->is_occupied) {
            b->is_occupied = true;
            strncpy(b->patient_token, p->token, sizeof(b->patient_token) - 1);
            strncpy(b->patient_name, p->name, sizeof(b->patient_name) - 1);
            b->patient_severity = (int)p->severity;
            get_current_timestamp(b->admission_time, sizeof(b->admission_time));
            if (out_bed_id && out_len > 0) {
                strncpy(out_bed_id, b->bed_id, out_len - 1);
                out_bed_id[out_len - 1] = '\0';
            }
            return true;
        }
    }

    /* Second pass: if acute ER or observation, can allocate any available general ER bed */
    for (int i = 0; i < bm->total_beds; i++) {
        Bed *b = &bm->beds[i];
        if (!b->is_occupied && b->type != BED_ICU && b->type != BED_TRAUMA) {
            b->is_occupied = true;
            strncpy(b->patient_token, p->token, sizeof(b->patient_token) - 1);
            strncpy(b->patient_name, p->name, sizeof(b->patient_name) - 1);
            b->patient_severity = (int)p->severity;
            get_current_timestamp(b->admission_time, sizeof(b->admission_time));
            if (out_bed_id && out_len > 0) {
                strncpy(out_bed_id, b->bed_id, out_len - 1);
                out_bed_id[out_len - 1] = '\0';
            }
            return true;
        }
    }

    return false;
}

bool bed_release(BedManager *bm, const char *bed_id) {
    if (!bm || !bed_id) return false;
    for (int i = 0; i < bm->total_beds; i++) {
        Bed *b = &bm->beds[i];
        if (strcmp(b->bed_id, bed_id) == 0) {
            if (!b->is_occupied) return false;
            b->is_occupied = false;
            b->patient_token[0] = '\0';
            b->patient_name[0] = '\0';
            b->patient_severity = 0;
            b->admission_time[0] = '\0';
            return true;
        }
    }
    return false;
}

bool bed_release_by_token(BedManager *bm, const char *token) {
    if (!bm || !token) return false;
    for (int i = 0; i < bm->total_beds; i++) {
        Bed *b = &bm->beds[i];
        if (b->is_occupied && strcmp(b->patient_token, token) == 0) {
            b->is_occupied = false;
            b->patient_token[0] = '\0';
            b->patient_name[0] = '\0';
            b->patient_severity = 0;
            b->admission_time[0] = '\0';
            return true;
        }
    }
    return false;
}

const Bed* bed_find(const BedManager *bm, const char *bed_id) {
    if (!bm || !bed_id) return NULL;
    for (int i = 0; i < bm->total_beds; i++) {
        if (strcmp(bm->beds[i].bed_id, bed_id) == 0) {
            return &bm->beds[i];
        }
    }
    return NULL;
}

const Bed* bed_find_by_token(const BedManager *bm, const char *token) {
    if (!bm || !token) return NULL;
    for (int i = 0; i < bm->total_beds; i++) {
        if (bm->beds[i].is_occupied && strcmp(bm->beds[i].patient_token, token) == 0) {
            return &bm->beds[i];
        }
    }
    return NULL;
}

int bed_get_total_count(const BedManager *bm) {
    return bm ? bm->total_beds : 0;
}

int bed_get_occupied_count(const BedManager *bm) {
    if (!bm) return 0;
    int count = 0;
    for (int i = 0; i < bm->total_beds; i++) {
        if (bm->beds[i].is_occupied) count++;
    }
    return count;
}

int bed_get_available_count(const BedManager *bm) {
    if (!bm) return 0;
    return bm->total_beds - bed_get_occupied_count(bm);
}

int bed_get_available_by_type(const BedManager *bm, BedType type) {
    if (!bm) return 0;
    int count = 0;
    for (int i = 0; i < bm->total_beds; i++) {
        if (bm->beds[i].type == type && !bm->beds[i].is_occupied) {
            count++;
        }
    }
    return count;
}

void bed_display_matrix(const BedManager *bm) {
    if (!bm) return;

    int occupied = bed_get_occupied_count(bm);
    int total = bm->total_beds;
    double pct = total > 0 ? ((double)occupied / total) * 100.0 : 0.0;

    printf("\n" COLOR_BOLD "========================================================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                    EMERGENCY DEPARTMENT BED OCCUPANCY MATRIX (Occupied: %d/%d - %.1f%%)                \n" COLOR_RESET,
           occupied, total, pct);
    printf("========================================================================================================\n");
    printf(COLOR_BOLD "%-8s | %-20s | %-10s | %-9s | %-22s | %-19s\n" COLOR_RESET,
           "Bed ID", "Category", "Status", "Token", "Patient Name", "Admission Time");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < bm->total_beds; i++) {
        const Bed *b = &bm->beds[i];
        if (b->is_occupied) {
            printf("%-8s | %-20s | " COLOR_RED "%-10s" COLOR_RESET " | %-9s | %-22.22s | %-19s\n",
                   b->bed_id,
                   bed_type_to_string(b->type),
                   "OCCUPIED",
                   b->patient_token,
                   b->patient_name,
                   b->admission_time);
        } else {
            printf("%-8s | %-20s | " COLOR_GREEN "%-10s" COLOR_RESET " | %-9s | %-22s | %-19s\n",
                   b->bed_id,
                   bed_type_to_string(b->type),
                   "VACANT",
                   "-",
                   "-",
                   "-");
        }
    }
    printf("========================================================================================================\n");
}
