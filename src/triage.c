#include "triage.h"

static const TriageProtocol PROTOCOLS[] = {
    {
        .level = TRIAGE_CRITICAL,
        .title = "RESUSCITATION (IMMEDIATE)",
        .target_wait_time = "0 minutes (Immediate)",
        .clinical_description = "Life or limb-threatening condition requiring aggressive resuscitation.",
        .example_conditions = "Cardiac arrest, severe respiratory failure, massive hemorrhage, anaphylaxis, deep coma."
    },
    {
        .level = TRIAGE_EMERGENT,
        .title = "EMERGENT (VERY URGENT)",
        .target_wait_time = "< 10 minutes",
        .clinical_description = "High potential to deteriorate rapidly into life threat, severe acute distress.",
        .example_conditions = "Suspected stroke, acute chest pain (STEMI), severe asthma attack, altered mental state."
    },
    {
        .level = TRIAGE_URGENT,
        .title = "URGENT",
        .target_wait_time = "< 30 minutes",
        .clinical_description = "Potentially serious; requires 2 or more diagnostic/treatment resources; stable vitals.",
        .example_conditions = "Severe abdominal pain, open wound with bleeding controlled, moderate fractures, high fever."
    },
    {
        .level = TRIAGE_LESS_URGENT,
        .title = "LESS URGENT",
        .target_wait_time = "< 60 minutes",
        .clinical_description = "Stable condition requiring single diagnostic/simple therapeutic resource.",
        .example_conditions = "Minor extremity sprain, simple laceration requiring sutures, mild vomiting without dehydration."
    },
    {
        .level = TRIAGE_NON_URGENT,
        .title = "NON-URGENT",
        .target_wait_time = "< 120 minutes",
        .clinical_description = "Chronic or minor issues; no immediate emergency interventions needed.",
        .example_conditions = "Prescription refill, suture removal, mild rash, cold symptoms, minor abrasions."
    }
};

const TriageProtocol* triage_get_protocol(TriageLevel level) {
    if (level >= TRIAGE_CRITICAL && level <= TRIAGE_NON_URGENT) {
        return &PROTOCOLS[level - 1];
    }
    return NULL;
}

void triage_print_guide(void) {
    printf("\n" COLOR_BOLD "========================================================================================\n" COLOR_RESET);
    printf(COLOR_BOLD "                    EMERGENCY DEPARTMENT TRIAGE PROTOCOL (ESI SCALE)                   \n" COLOR_RESET);
    printf("========================================================================================\n");
    for (int i = 0; i < 5; i++) {
        const TriageProtocol *p = &PROTOCOLS[i];
        const char *col = triage_level_color(p->level);
        printf("%s[%s] - Target Wait: %s\n" COLOR_RESET, col, p->title, p->target_wait_time);
        printf("  Description : %s\n", p->clinical_description);
        printf("  Examples    : %s\n\n", p->example_conditions);
    }
    printf("========================================================================================\n");
}

/* Helper to convert string to lowercase for search */
static void to_lower_str(const char *src, char *dest, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

TriageLevel triage_suggest_level(const char *complaint) {
    if (!complaint || strlen(complaint) == 0) return TRIAGE_NON_URGENT;
    char lower[256];
    to_lower_str(complaint, lower, sizeof(lower));

    /* Level 1: Immediate life threat */
    if (strstr(lower, "cardiac arrest") || strstr(lower, "unconscious") ||
        strstr(lower, "not breathing") || strstr(lower, "severe bleeding") ||
        strstr(lower, "respiratory failure") || strstr(lower, "gunshot") ||
        strstr(lower, "massive trauma") || strstr(lower, "choking")) {
        return TRIAGE_CRITICAL;
    }

    /* Level 2: Emergent */
    if (strstr(lower, "chest pain") || strstr(lower, "stroke") ||
        strstr(lower, "paralysis") || strstr(lower, "severe burn") ||
        strstr(lower, "seizure") || strstr(lower, "poisoning") ||
        strstr(lower, "suicidal") || strstr(lower, "overdose") ||
        strstr(lower, "severe dyspnea") || strstr(lower, "head injury")) {
        return TRIAGE_EMERGENT;
    }

    /* Level 3: Urgent */
    if (strstr(lower, "abdominal pain") || strstr(lower, "fracture") ||
        strstr(lower, "broken bone") || strstr(lower, "high fever") ||
        strstr(lower, "kidney stone") || strstr(lower, "asthma") ||
        strstr(lower, "deep cut") || strstr(lower, "infection")) {
        return TRIAGE_URGENT;
    }

    /* Level 4: Less Urgent */
    if (strstr(lower, "sprain") || strstr(lower, "laceration") ||
        strstr(lower, "minor cut") || strstr(lower, "earache") ||
        strstr(lower, "sore throat") || strstr(lower, "vomiting") ||
        strstr(lower, "swelling") || strstr(lower, "back pain")) {
        return TRIAGE_LESS_URGENT;
    }

    /* Default Level 5: Non-Urgent */
    return TRIAGE_NON_URGENT;
}
