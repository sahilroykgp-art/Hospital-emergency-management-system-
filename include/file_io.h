#ifndef FILE_IO_H
#define FILE_IO_H

#include "patient.h"
#include "bed_manager.h"

#define PATIENT_LOG_FILE   "er_patient_log.txt"
#define DAILY_REPORT_FILE  "daily_er_report.txt"

/* Patient Activity Logging into er_patient_log.txt */
bool file_log_patient_intake(const Patient *p);
bool file_log_patient_served(const Patient *p, const char *action, const char *doctor_notes, const char *bed_id);

/* Daily Emergency Report Generation into daily_er_report.txt */
bool file_generate_daily_report(const BedManager *bm,
                                int total_registered,
                                int total_served,
                                int count_level1,
                                int count_level2,
                                int count_level3,
                                int count_level4,
                                int count_level5);

/* File Viewer Functions */
void file_view_patient_log(void);
void file_view_daily_report(void);

/* Reset / Clear File Logs */
bool file_reset_logs(void);

#endif /* FILE_IO_H */
