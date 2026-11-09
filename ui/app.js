// ============================================================================
// Hospital Emergency Management System - Core Web Application (Single Script)
// Replaces multi-controller complexity with a clean, unified architecture.
//
// Key Concepts:
//   1. Min-Heap Priority Queue (DSA):
//      - Sorted strictly by Triage Severity (Level 1: Critical -> Level 5: Non-Urgent)
//      - Equal severity tie-breaking: Strict FIFO arrival order sequence
//   2. Emergency Ward Bed Allocation (22 specialized beds)
//   3. Clinical Workflow: Receptionist Intake -> Doctor Bay -> Bed / Discharge
//   4. Audit Logging & Report Generation (er_patient_log.txt, daily_er_report.txt)
// ============================================================================

const ERApp = (function () {
    // ------------------------------------------------------------------------
    // 1. Central Application State
    // ------------------------------------------------------------------------
    const state = {
        queue: [],
        activePatient: null,
        arrivalSequence: 0,
        nextTokenNum: 1001,
        totalRegisteredCount: 0,
        totalServedCount: 0,
        severityCounts: { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 },
        logEntries: [],
        beds: [
            { id: "ICU-01", type: "ICU / Resus", occupied: false, patientToken: "" },
            { id: "ICU-02", type: "ICU / Resus", occupied: false, patientToken: "" },
            { id: "ICU-03", type: "ICU / Resus", occupied: false, patientToken: "" },
            { id: "ICU-04", type: "ICU / Resus", occupied: false, patientToken: "" },
            { id: "TRM-01", type: "Trauma Bay", occupied: false, patientToken: "" },
            { id: "TRM-02", type: "Trauma Bay", occupied: false, patientToken: "" },
            { id: "TRM-03", type: "Trauma Bay", occupied: false, patientToken: "" },
            { id: "TRM-04", type: "Trauma Bay", occupied: false, patientToken: "" },
            { id: "ER-01", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-02", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-03", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-04", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-05", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-06", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-07", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "ER-08", type: "Acute ER", occupied: false, patientToken: "" },
            { id: "OBS-01", type: "Observation", occupied: false, patientToken: "" },
            { id: "OBS-02", type: "Observation", occupied: false, patientToken: "" },
            { id: "OBS-03", type: "Observation", occupied: false, patientToken: "" },
            { id: "OBS-04", type: "Observation", occupied: false, patientToken: "" },
            { id: "OBS-05", type: "Observation", occupied: false, patientToken: "" },
            { id: "OBS-06", type: "Observation", occupied: false, patientToken: "" }
        ]
    };

    let demoBatchIndex = 0;

    // 30 Unique Realistic Cases for Demo Simulations
    const DEMO_CASES = [
        { name: "Vikram Malhotra", age: 58, gender: "Male", contact: "+91-9811223344", complaint: "Acute crushing chest pain (suspected STEMI)", severity: 2 },
        { name: "Aarav Sharma", age: 22, gender: "Male", contact: "+91-9876543210", complaint: "Deep forearm laceration with active bleeding", severity: 4 },
        { name: "Rajeshwari Devi", age: 67, gender: "Female", contact: "+91-9455112233", complaint: "Sudden collapse, unresponsive, suspected cardiac arrest", severity: 1 },
        { name: "Priya Nambiar", age: 34, gender: "Female", contact: "+91-9988776655", complaint: "Severe lower abdominal pain (Acute appendicitis)", severity: 3 },
        { name: "Suresh Sen", age: 45, gender: "Male", contact: "+91-9123456780", complaint: "High-speed motorcycle collision polytrauma", severity: 1 },
        { name: "Ananya Roy", age: 19, gender: "Female", contact: "+91-9011223344", complaint: "Migraine & routine inhaler prescription refill", severity: 5 },
        { name: "Devendra Kapoor", age: 61, gender: "Male", contact: "+91-9822334455", complaint: "Sudden slurred speech & hemiparesis (acute ischemic stroke)", severity: 2 },
        { name: "Rohan Verma", age: 28, gender: "Male", contact: "+91-9711224466", complaint: "Twisted ankle during football match, moderate swelling", severity: 4 },
        { name: "Shanti Swaroop", age: 72, gender: "Female", contact: "+91-9322115588", complaint: "Massive hematemesis & hemorrhagic shock (bleeding varices)", severity: 1 },
        { name: "Meera Iyer", age: 41, gender: "Female", contact: "+91-9833445566", complaint: "High remittent fever, rigors, & severe flank pain (pyelonephritis)", severity: 3 },
        { name: "Gurpreet Singh", age: 36, gender: "Male", contact: "+91-9744556677", complaint: "Severe industrial chemical burn across chest & inhalation injury", severity: 1 },
        { name: "Kavita Joshi", age: 25, gender: "Female", contact: "+91-9155667788", complaint: "Mild seasonal allergic rhinitis, sneezing & conjunctivitis", severity: 5 },
        { name: "Farhan Akhtar", age: 50, gender: "Male", contact: "+91-9866778899", complaint: "Unstable angina with diaphoresis (triple-vessel disease)", severity: 2 },
        { name: "Nikhil Deshmukh", age: 31, gender: "Male", contact: "+91-9777889900", complaint: "Second-degree thermal burn on right hand from boiling water", severity: 4 },
        { name: "Saraswati Chatterjee", age: 80, gender: "Female", contact: "+91-9388990011", complaint: "Profound bradycardia (HR 24) with complete 3rd-degree AV block", severity: 1 },
        { name: "Sunita Rao", age: 49, gender: "Female", contact: "+91-9999001122", complaint: "Severe colicky epigastric pain radiating to back (pancreatitis)", severity: 3 },
        { name: "Balwinder Gill", age: 42, gender: "Male", contact: "+91-9800112233", complaint: "Traumatic pneumothorax with paradoxical chest movement & cyanosis", severity: 1 },
        { name: "Sneha Kulkarni", age: 23, gender: "Female", contact: "+91-9011334455", complaint: "Mild superficial finger contusion from cabinet door slam", severity: 5 },
        { name: "Harish Chandra", age: 64, gender: "Male", contact: "+91-9822446688", complaint: "Severe acute dyspnea & orthopnea (congestive heart failure)", severity: 2 },
        { name: "Aditya Mehta", age: 27, gender: "Male", contact: "+91-9733557799", complaint: "Suspected closed wrist hairline fracture following bicycle fall", severity: 4 },
        { name: "Kamla Bai", age: 75, gender: "Female", contact: "+91-9344668800", complaint: "Septic shock secondary to urosepsis, BP 70/40 mmHg, lethargic", severity: 1 },
        { name: "Deepa Menon", age: 38, gender: "Female", contact: "+91-9955779911", complaint: "Acute renal colic with macroscopic hematuria & severe groin pain", severity: 3 },
        { name: "Tariq Mansoor", age: 33, gender: "Male", contact: "+91-9866880022", complaint: "Anaphylactic shock from bee sting with stridor & facial angioedema", severity: 1 },
        { name: "Pooja Pandey", age: 21, gender: "Female", contact: "+91-9077991133", complaint: "Mild chronic tension headache, routine evaluation requested", severity: 5 },
        { name: "Jagdish Prasad", age: 56, gender: "Male", contact: "+91-9811335577", complaint: "Severe diabetic ketoacidosis, glucose 540 mg/dL, Kussmaul breathing", severity: 2 },
        { name: "Arjun Pillai", age: 29, gender: "Male", contact: "+91-9722446600", complaint: "Corneal foreign body sensation & photophobia after grinding", severity: 4 },
        { name: "Nirmala Sundaram", age: 69, gender: "Female", contact: "+91-9333557722", complaint: "Massive pulmonary embolism with acute hemodynamic collapse", severity: 1 },
        { name: "Ritu Bhattacharya", age: 32, gender: "Female", contact: "+91-9944668833", complaint: "Acute cholecystitis with Murphy's sign positive & high fever", severity: 3 },
        { name: "Manpreet Dhillon", age: 40, gender: "Male", contact: "+91-9855779944", complaint: "Acute cardiac tamponade from penetrating stab wound, Beck's triad", severity: 1 },
        { name: "Tanvi Saxena", age: 20, gender: "Female", contact: "+91-9066880055", complaint: "Minor superficial cat scratch on forearm, rabies/tetanus check", severity: 5 }
    ];

    // ------------------------------------------------------------------------
    // 2. Helper Functions (Formatting & Logging)
    // ------------------------------------------------------------------------
    function getTimestamp() {
        const d = new Date();
        return d.toISOString().replace("T", " ").substring(0, 19);
    }

    function getSeverityBadge(sev) {
        const labels = {
            1: "Level 1 (CRITICAL)",
            2: "Level 2 (EMERGENT)",
            3: "Level 3 (URGENT)",
            4: "Level 4 (LESS URGENT)",
            5: "Level 5 (NON-URGENT)"
        };
        return `<span class="badge badge-${sev}">${labels[sev] || "Level " + sev}</span>`;
    }

    function appendLog(eventType, text) {
        const time = getTimestamp();
        const entry = `[${time}] [${eventType}] ${text}`;
        state.logEntries.push(entry);
        renderLogBox();
    }

    // ------------------------------------------------------------------------
    // 3. Min-Heap Priority Queue (DSA)
    // ------------------------------------------------------------------------
    function sortQueue() {
        state.queue.sort((a, b) => {
            // Rule 1: Lower numeric severity = Higher priority (1 before 2)
            if (a.severity !== b.severity) {
                return a.severity - b.severity;
            }
            // Rule 2: Strict FIFO arrival sequence tie-breaking
            return a.arrivalSequence - b.arrivalSequence;
        });
    }

    function enqueue(patient) {
        if (!patient) return false;
        state.queue.push(patient);
        sortQueue();
        renderAll();
        return true;
    }

    function dequeue() {
        if (state.queue.length === 0) return null;
        sortQueue();
        const p = state.queue.shift();
        renderAll();
        return p;
    }

    // ------------------------------------------------------------------------
    // 4. Bed Management
    // ------------------------------------------------------------------------
    function assignBed(bedId, patientToken) {
        const bed = state.beds.find(b => b.id === bedId);
        if (!bed) return false;
        bed.occupied = true;
        bed.patientToken = patientToken;
        renderAll();
        return true;
    }

    function releaseBed(bedId) {
        const bed = state.beds.find(b => b.id === bedId);
        if (!bed) return false;
        bed.occupied = false;
        bed.patientToken = "";
        renderAll();
        return true;
    }

    function toggleBedState(bedId) {
        const bed = state.beds.find(b => b.id === bedId);
        if (!bed) return;
        bed.occupied = !bed.occupied;
        if (!bed.occupied) {
            bed.patientToken = "";
            appendLog("BED RELEASE", `Bed ${bedId} was manually marked vacant.`);
        } else {
            bed.patientToken = "MANUAL";
            appendLog("BED OCCUPIED", `Bed ${bedId} was manually marked occupied.`);
        }
        renderAll();
    }

    function autoAllocateForPatient(patient) {
        if (!patient) return null;
        let preferredPrefix = "ER-";
        if (patient.severity === 1) preferredPrefix = "ICU-";
        else if (patient.severity === 2) preferredPrefix = "TRM-";
        else if (patient.severity >= 4) preferredPrefix = "OBS-";

        // Find available bed in preferred unit
        let targetBed = state.beds.find(b => !b.occupied && b.id.startsWith(preferredPrefix));
        // Fallback to any vacant bed
        if (!targetBed) {
            targetBed = state.beds.find(b => !b.occupied);
        }

        if (targetBed) {
            targetBed.occupied = true;
            targetBed.patientToken = patient.token;
            patient.assignedBed = targetBed.id;
            return targetBed.id;
        }
        return null;
    }

    // ------------------------------------------------------------------------
    // 5. Clinical Actions (Doctor Examination Bay)
    // ------------------------------------------------------------------------
    function advanceNextPatient() {
        if (state.queue.length === 0) {
            alert("The Emergency Priority Queue is currently empty. No patients waiting.");
            return false;
        }

        if (state.activePatient) {
            const ok = confirm(`Patient ${state.activePatient.token} (${state.activePatient.name}) is currently in the Examination Bay.\n\nDo you want to finalize/overwrite and advance the next patient?`);
            if (!ok) return false;
        }

        const patient = dequeue();
        if (!patient) return false;

        patient.status = "UNDER TREATMENT";
        state.activePatient = patient;
        state.totalServedCount++;

        appendLog("DOCTOR DEQUEUE", `Advanced ${patient.token} (${patient.name}, Level ${patient.severity}) into Clinical Examination Bay.`);
        renderAll();
        return true;
    }

    function dischargeActivePatient() {
        if (!state.activePatient) return;
        const p = state.activePatient;
        if (!confirm(`Confirm discharge for ${p.token} (${p.name})?`)) return;

        if (p.assignedBed && p.assignedBed !== "None") {
            releaseBed(p.assignedBed);
        }

        appendLog("PATIENT DISCHARGE", `Discharged ${p.token} (${p.name}). Notes: ${p.doctorNotes || "None"}.`);
        state.activePatient = null;
        renderAll();
    }

    function transferActivePatient() {
        if (!state.activePatient) return;
        const p = state.activePatient;
        const dest = prompt(`Enter destination department for ${p.token} (${p.name}):`, "Cardiology ICU");
        if (!dest) return;

        if (p.assignedBed && p.assignedBed !== "None") {
            releaseBed(p.assignedBed);
        }

        appendLog("PATIENT TRANSFER", `Transferred ${p.token} (${p.name}) to ${dest}. Notes: ${p.doctorNotes || "None"}.`);
        state.activePatient = null;
        renderAll();
    }

    function updateActivePatientNotes(notes) {
        if (!state.activePatient) return;
        state.activePatient.doctorNotes = notes;
        appendLog("DOCTOR NOTES", `Updated notes for ${state.activePatient.token}: "${notes}".`);
        renderExamBay();
    }

    function admitActivePatientToBed(bedId) {
        if (!state.activePatient) return;
        if (!bedId) {
            alert("Please select a valid vacant bed.");
            return;
        }

        const bed = state.beds.find(b => b.id === bedId);
        if (!bed || bed.occupied) {
            alert("Selected bed is not available.");
            return;
        }

        // Release previously held bed if switching
        if (state.activePatient.assignedBed && state.activePatient.assignedBed !== "None") {
            releaseBed(state.activePatient.assignedBed);
        }

        assignBed(bedId, state.activePatient.token);
        state.activePatient.assignedBed = bedId;
        state.activePatient.status = "ADMITTED";

        appendLog("BED ADMISSION", `Admitted ${state.activePatient.token} (${state.activePatient.name}) to Bed ${bedId}.`);
        renderAll();
    }

    // ------------------------------------------------------------------------
    // 6. Simulation Batch Loader
    // ------------------------------------------------------------------------
    function injectDemoBatch() {
        const batchSize = 6;
        for (let i = 0; i < batchSize; i++) {
            const c = DEMO_CASES[(demoBatchIndex + i) % DEMO_CASES.length];
            state.arrivalSequence++;
            state.totalRegisteredCount++;
            if (state.severityCounts[c.severity] !== undefined) {
                state.severityCounts[c.severity]++;
            }

            const token = `TK-${state.nextTokenNum++}`;
            const p = {
                token: token,
                name: c.name,
                age: c.age,
                gender: c.gender,
                contact: c.contact,
                complaint: c.complaint,
                severity: c.severity,
                arrivalSequence: state.arrivalSequence,
                arrivalTime: new Date().toLocaleTimeString(),
                status: "WAITING",
                assignedBed: "",
                doctorNotes: ""
            };
            state.queue.push(p);
            appendLog("SIMULATION INTAKE", `Demo Enqueued ${token} (${c.name}, Sev ${c.severity}) - ${c.complaint}`);
        }

        demoBatchIndex = (demoBatchIndex + batchSize) % DEMO_CASES.length;
        sortQueue();
        renderAll();
    }

    // ------------------------------------------------------------------------
    // 7. Audit Log & Report Export (er_patient_log.txt, daily_er_report.txt)
    // ------------------------------------------------------------------------
    function downloadFile(filename, content) {
        if (typeof document === "undefined" || !document.createElement) return;
        const blob = new Blob([content], { type: "text/plain;charset=utf-8" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }

    function exportPatientLog() {
        let content = "======================================================================\n";
        content += "        HOSPITAL EMERGENCY DEPARTMENT - PATIENT AUDIT LOG (er_patient_log.txt)\n";
        content += "======================================================================\n\n";
        content += state.logEntries.join("\n\n");
        content += "\n\nEnd of Log.\n";
        downloadFile("er_patient_log.txt", content);
    }

    function exportDailyReport() {
        const now = getTimestamp();
        const occupied = state.beds.filter(b => b.occupied).length;
        const totalBeds = state.beds.length;
        const pct = ((occupied / totalBeds) * 100).toFixed(1);

        let report = "######################################################################\n";
        report += "#             HOSPITAL EMERGENCY DEPARTMENT - DAILY AUDIT REPORT      #\n";
        report += "######################################################################\n\n";
        report += `Report Generated At : ${now}\n`;
        report += `Facility Name       : Central Hospital Emergency & Trauma Care\n`;
        report += `Operational Status  : Active (24x7 Emergency Services)\n\n`;
        report += "----------------------------------------------------------------------\n";
        report += "1. PATIENT VOLUME SUMMARY\n";
        report += "----------------------------------------------------------------------\n";
        report += `Total Patients Registered Today : ${state.totalRegisteredCount}\n`;
        report += `Total Patients Served / Treated : ${state.totalServedCount}\n`;
        report += `Patients Currently Waiting/Active: ${state.queue.length}\n\n`;
        report += "----------------------------------------------------------------------\n";
        report += "2. TRIAGE SEVERITY DISTRIBUTION (ESI SCALE)\n";
        report += "----------------------------------------------------------------------\n";
        report += `Level 1 - Resuscitation (Immediate Life Threat) :   ${state.severityCounts[1]}\n`;
        report += `Level 2 - Emergent (High Risk / Minutes Matter) :   ${state.severityCounts[2]}\n`;
        report += `Level 3 - Urgent (Complex / Stable Vitals)       :   ${state.severityCounts[3]}\n`;
        report += `Level 4 - Less Urgent (Single Resource)          :   ${state.severityCounts[4]}\n`;
        report += `Level 5 - Non-Urgent (Routine Evaluation)        :   ${state.severityCounts[5]}\n\n`;
        report += "----------------------------------------------------------------------\n";
        report += "3. EMERGENCY BED OCCUPANCY STATUS\n";
        report += "----------------------------------------------------------------------\n";
        report += `Total Capacity       : ${totalBeds} Beds\n`;
        report += `Occupied Beds        : ${occupied} Beds\n`;
        report += `Available Beds       : ${totalBeds - occupied} Beds\n`;
        report += `Occupancy Percentage : ${pct}%\n\n`;
        report += "Occupied Beds List:\n";

        const occupiedBeds = state.beds.filter(b => b.occupied);
        if (occupiedBeds.length === 0) {
            report += "  All beds are currently vacant.\n";
        } else {
            occupiedBeds.forEach(b => {
                report += `  Bed [${b.id}] - Token: ${b.patientToken || "MANUAL"} | Type: ${b.type}\n`;
            });
        }

        report += "\n----------------------------------------------------------------------\n";
        report += "End of Daily Report - End of File\n";
        report += "######################################################################\n";

        downloadFile("daily_er_report.txt", report);
    }

    function resetAllRecords() {
        if (!confirm("Are you sure you want to reset all emergency department records?\nThis will clear the priority queue, release all beds, and purge logs.")) {
            return;
        }

        state.queue = [];
        state.activePatient = null;
        state.arrivalSequence = 0;
        state.nextTokenNum = 1001;
        state.totalRegisteredCount = 0;
        state.totalServedCount = 0;
        state.severityCounts = { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 };
        state.logEntries = [];
        demoBatchIndex = 0;

        state.beds.forEach(b => {
            b.occupied = false;
            b.patientToken = "";
        });

        appendLog("SYSTEM RESET", "Emergency department records reset for new operational cycle.");
        renderAll();
    }

    // ------------------------------------------------------------------------
    // 8. DOM Rendering Functions
    // ------------------------------------------------------------------------
    function renderStats() {
        if (typeof document === "undefined") return;
        const statWaiting = document.getElementById("stat-waiting");
        const statTreating = document.getElementById("stat-treating");
        const statTotal = document.getElementById("stat-total");
        const statBeds = document.getElementById("stat-beds");

        if (statWaiting) statWaiting.textContent = state.queue.length;
        if (statTreating) statTreating.textContent = state.activePatient ? 1 : 0;
        if (statTotal) statTotal.textContent = state.totalRegisteredCount;

        const occupiedCount = state.beds.filter(b => b.occupied).length;
        if (statBeds) statBeds.textContent = `${occupiedCount} / ${state.beds.length}`;
    }

    function renderQueueTable() {
        if (typeof document === "undefined") return;
        sortQueue();
        const tbody = document.getElementById("queue-tbody");
        if (!tbody) return;

        tbody.innerHTML = "";
        if (state.queue.length === 0) {
            tbody.innerHTML = `<tr><td colspan="8" class="text-center text-muted">No patients waiting in queue. Register a patient above or load demo simulation.</td></tr>`;
        } else {
            state.queue.forEach((p, index) => {
                const tr = document.createElement("tr");
                tr.innerHTML = `
                    <td><strong>#${index + 1}</strong></td>
                    <td><code>${p.token}</code></td>
                    <td><strong>${p.name}</strong></td>
                    <td>${p.age} / ${p.gender}</td>
                    <td>${getSeverityBadge(p.severity)}</td>
                    <td>${p.complaint}</td>
                    <td>Seq #${p.arrivalSequence} (${p.arrivalTime})</td>
                    <td><span class="badge badge-3">${p.status}</span></td>
                `;
                tbody.appendChild(tr);
            });
        }
    }

    function renderBedsMatrix() {
        if (typeof document === "undefined") return;
        const container = document.getElementById("beds-matrix");
        if (!container) return;

        container.innerHTML = "";
        state.beds.forEach(b => {
            const div = document.createElement("div");
            div.className = `bed-card ${b.occupied ? "occupied" : "vacant"}`;
            div.innerHTML = `
                <div class="bed-id">${b.id}</div>
                <div class="bed-type">${b.type}</div>
                <div class="bed-status">${b.occupied ? "OCCUPIED" : "VACANT"}</div>
                ${b.patientToken ? `<div style="font-size: 11px; margin-top: 4px; color: #b71c1c;"><strong>${b.patientToken}</strong></div>` : ""}
            `;
            div.title = b.occupied ? `Occupied by ${b.patientToken}. Click to release.` : "Vacant. Click to mark occupied.";
            div.addEventListener("click", () => toggleBedState(b.id));
            container.appendChild(div);
        });
    }

    function renderExamBay() {
        if (typeof document === "undefined") return;
        const container = document.getElementById("exam-bay-container");
        if (!container) return;

        if (!state.activePatient) {
            container.innerHTML = `
                <div class="alert alert-info">
                    Examination Bay is currently empty. Click <strong>"Advance Highest-Priority Patient"</strong> above to dequeue next critical patient.
                </div>
            `;
            return;
        }

        const vacantBeds = state.beds.filter(b => !b.occupied);
        let bedOptions = vacantBeds.map(b => `<option value="${b.id}">${b.id} (${b.type})</option>`).join("");
        if (!bedOptions) bedOptions = `<option value="">No beds available</option>`;

        container.innerHTML = `
            <div class="exam-card">
                <div class="exam-grid">
                    <div class="exam-item">
                        <label>Active Token</label>
                        <strong>${state.activePatient.token}</strong>
                    </div>
                    <div class="exam-item">
                        <label>Patient Details</label>
                        <strong>${state.activePatient.name} (${state.activePatient.age} yrs, ${state.activePatient.gender})</strong>
                    </div>
                    <div class="exam-item">
                        <label>Triage Priority</label>
                        <div>${getSeverityBadge(state.activePatient.severity)}</div>
                    </div>
                    <div class="exam-item">
                        <label>Chief Complaint</label>
                        <strong>${state.activePatient.complaint}</strong>
                    </div>
                    <div class="exam-item">
                        <label>Arrival Sequence</label>
                        <strong>#${state.activePatient.arrivalSequence} (${state.activePatient.arrivalTime})</strong>
                    </div>
                    <div class="exam-item">
                        <label>Assigned Bed</label>
                        <strong>${state.activePatient.assignedBed || "None (In Bay)"}</strong>
                    </div>
                </div>

                <div style="margin: 12px 0;">
                    <label for="doc-notes" style="font-size: 12px; font-weight: bold; color: #5d4037;">Doctor's Diagnosis & Notes:</label>
                    <input type="text" id="doc-notes" value="${state.activePatient.doctorNotes || ""}" placeholder="Enter medical observations..." style="width: 100%; padding: 8px; font-size: 13px; border: 1px solid #bcaaa4; border-radius: 4px; box-sizing: border-box; margin-top: 4px;">
                </div>

                <div class="exam-actions">
                    <button type="button" id="btn-save-notes" class="btn btn-sm btn-blue">Save Notes</button>
                    <div style="display: flex; gap: 8px; align-items: center;">
                        <select id="select-bed" style="padding: 6px; font-size: 12px; border-radius: 4px; border: 1px solid #ccc;">
                            ${bedOptions}
                        </select>
                        <button type="button" id="btn-admit-bed" class="btn btn-sm btn-green">Admit to Bed</button>
                    </div>
                    <button type="button" id="btn-discharge" class="btn btn-sm btn-orange">Discharge Home</button>
                    <button type="button" id="btn-transfer" class="btn btn-sm btn-outline">Transfer Specialty</button>
                </div>
            </div>
        `;

        // Wire exam bay buttons
        const btnSaveNotes = document.getElementById("btn-save-notes");
        if (btnSaveNotes) {
            btnSaveNotes.addEventListener("click", () => {
                const notesInput = document.getElementById("doc-notes");
                if (notesInput) updateActivePatientNotes(notesInput.value);
            });
        }

        const btnAdmitBed = document.getElementById("btn-admit-bed");
        if (btnAdmitBed) {
            btnAdmitBed.addEventListener("click", () => {
                const sel = document.getElementById("select-bed");
                if (sel) admitActivePatientToBed(sel.value);
            });
        }

        const btnDischarge = document.getElementById("btn-discharge");
        if (btnDischarge) {
            btnDischarge.addEventListener("click", dischargeActivePatient);
        }

        const btnTransfer = document.getElementById("btn-transfer");
        if (btnTransfer) {
            btnTransfer.addEventListener("click", transferActivePatient);
        }
    }

    function renderLogBox() {
        if (typeof document === "undefined") return;
        const logBox = document.getElementById("log-preview");
        if (logBox) {
            logBox.value = state.logEntries.join("\n");
            logBox.scrollTop = logBox.scrollHeight;
        }
    }

    function renderAll() {
        renderStats();
        renderQueueTable();
        renderBedsMatrix();
        renderExamBay();
        renderLogBox();
    }

    // ------------------------------------------------------------------------
    // 9. Application Bootstrap & Event Wiring
    // ------------------------------------------------------------------------
    function init() {
        if (typeof document === "undefined") return;

        // Digital Clock
        function updateClock() {
            const el = document.getElementById("clock-display");
            if (el) {
                const d = new Date();
                el.textContent = "Time: " + d.toLocaleTimeString();
            }
        }
        setInterval(updateClock, 1000);
        updateClock();

        // 1. Patient Intake Form
        const form = document.getElementById("patient-form");
        if (form) {
            form.addEventListener("submit", (e) => {
                e.preventDefault();
                const name = document.getElementById("p-name").value.trim();
                const age = parseInt(document.getElementById("p-age").value, 10);
                const gender = document.getElementById("p-gender").value;
                const contact = document.getElementById("p-contact").value.trim();
                const complaint = document.getElementById("p-complaint").value.trim();
                const severity = parseInt(document.getElementById("p-severity").value, 10);

                if (!name || isNaN(age) || !complaint) {
                    alert("Please fill in all required fields!");
                    return;
                }

                // Check duplicate active name
                const isDuplicate = state.queue.some(p => p.name.toLowerCase() === name.toLowerCase()) ||
                                    (state.activePatient && state.activePatient.name.toLowerCase() === name.toLowerCase());
                if (isDuplicate) {
                    const proceed = confirm(`Warning: A patient named "${name}" is already currently active in the ER.\n\nDo you still wish to proceed?`);
                    if (!proceed) return;
                }

                state.arrivalSequence++;
                state.totalRegisteredCount++;
                if (state.severityCounts[severity] !== undefined) {
                    state.severityCounts[severity]++;
                }

                const token = `TK-${state.nextTokenNum++}`;
                const patient = {
                    token: token,
                    name: name,
                    age: age,
                    gender: gender,
                    contact: contact || "N/A",
                    complaint: complaint,
                    severity: severity,
                    arrivalSequence: state.arrivalSequence,
                    arrivalTime: new Date().toLocaleTimeString(),
                    status: "WAITING",
                    assignedBed: "",
                    doctorNotes: ""
                };

                enqueue(patient);
                appendLog("PATIENT INTAKE", `Registered ${token} (${name}, ${age}y, Sev ${severity}) - "${complaint}". Enqueued into Min-Heap.`);
                form.reset();
                alert(`Success! Patient ${token} (${name}) registered and prioritized!`);
            });
        }

        // 2. Advance Queue Button
        const btnAdvance = document.getElementById("btn-advance-queue");
        if (btnAdvance) {
            btnAdvance.addEventListener("click", advanceNextPatient);
        }

        // 3. Demo Simulation Button
        const btnDemo = document.getElementById("btn-demo-data");
        if (btnDemo) {
            btnDemo.addEventListener("click", injectDemoBatch);
        }

        // 4. Download Log
        const btnDownloadLog = document.getElementById("btn-download-log");
        if (btnDownloadLog) {
            btnDownloadLog.addEventListener("click", exportPatientLog);
        }

        // 5. Download Report
        const btnDownloadReport = document.getElementById("btn-download-report");
        if (btnDownloadReport) {
            btnDownloadReport.addEventListener("click", exportDailyReport);
        }

        // 6. Reset System
        const btnClear = document.getElementById("btn-clear-all");
        if (btnClear) {
            btnClear.addEventListener("click", resetAllRecords);
        }

        // Initial Render & Boot Log
        appendLog("SYSTEM BOOT", "Hospital Emergency Department operational. Priority Queue & Bed Matrix ready.");
        renderAll();
    }

    // Auto-init on DOMContentLoaded if running in browser
    if (typeof document !== "undefined" && document.addEventListener) {
        document.addEventListener("DOMContentLoaded", init);
    }

    // Public API
    return {
        state,
        init,
        enqueue,
        dequeue,
        assignBed,
        releaseBed,
        autoAllocateForPatient,
        advanceNextPatient,
        dischargeActivePatient,
        transferActivePatient,
        updateActivePatientNotes,
        admitActivePatientToBed,
        injectDemoBatch,
        exportPatientLog,
        exportDailyReport,
        resetAllRecords,
        renderAll,
        renderStats,
        getCases: () => DEMO_CASES
    };
})();

// Attach to window for global access
if (typeof window !== "undefined") {
    window.ERApp = ERApp;
}
