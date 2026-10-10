// ============================================================================
// Hospital Emergency Management System - Client Controller
// Simulates Min-Heap Priority Queue (DSA), Bed Management, & File Logs
// ============================================================================

// State
let queue = [];
let activePatient = null;
let arrivalSequence = 0;
let nextTokenNum = 1001;
let totalRegisteredCount = 0;
let severityCounts = { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 };
let logEntries = [];

// 22 Emergency Beds definition (matching C implementation)
let beds = [
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
];

// Helper: Timestamp
function getTimestamp() {
    const d = new Date();
    return d.toISOString().replace("T", " ").substring(0, 19);
}

// Helper: Triage Label & Badge
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

// DSA: Min-Heap / Priority Sorting
// Rule: Lower severity number = Higher Medical Priority (1 before 2).
// Equal severity = FIFO arrival sequence tie-breaking (earlier seq first).
function sortPriorityQueue() {
    queue.sort((a, b) => {
        if (a.severity !== b.severity) {
            return a.severity - b.severity;
        }
        return a.arrivalSequence - b.arrivalSequence;
    });
}

// Log an event
function appendLog(eventType, text) {
    const time = getTimestamp();
    const entry = `[${time}] [${eventType}] ${text}`;
    logEntries.push(entry);
    const logBox = document.getElementById("log-preview");
    if (logBox) {
        logBox.value = logEntries.join("\n");
        logBox.scrollTop = logBox.scrollHeight;
    }
}

// Render Waiting Queue
function renderQueueTable() {
    sortPriorityQueue();
    const tbody = document.getElementById("queue-tbody");
    tbody.innerHTML = "";

    if (queue.length === 0) {
        tbody.innerHTML = `<tr><td colspan="8" class="text-center text-muted">No patients waiting in queue. Register a patient above or load demo simulation.</td></tr>`;
    } else {
        queue.forEach((p, index) => {
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

    renderStats();
}

// Render Beds Matrix
function renderBedsMatrix() {
    const container = document.getElementById("beds-matrix");
    container.innerHTML = "";

    beds.forEach(b => {
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

    renderStats();
}

// Manual Bed Toggle
function toggleBedState(bedId) {
    const bed = beds.find(b => b.id === bedId);
    if (!bed) return;
    bed.occupied = !bed.occupied;
    if (!bed.occupied) {
        bed.patientToken = "";
        appendLog("BED RELEASE", `Bed ${bedId} was manually released to vacant.`);
    } else {
        bed.patientToken = "MANUAL";
        appendLog("BED OCCUPIED", `Bed ${bedId} was manually marked occupied.`);
    }
    renderBedsMatrix();
}

// Render Examination Bay
function renderExamBay() {
    const container = document.getElementById("exam-bay-container");
    if (!activePatient) {
        container.innerHTML = `
            <div class="alert alert-info">
                Examination Bay is currently empty. Click <strong>"Advance Highest-Priority Patient"</strong> above to dequeue next critical patient.
            </div>
        `;
        return;
    }

    const vacantBeds = beds.filter(b => !b.occupied);
    let bedOptions = vacantBeds.map(b => `<option value="${b.id}">${b.id} (${b.type})</option>`).join("");
    if (!bedOptions) bedOptions = `<option value="">No beds available</option>`;

    container.innerHTML = `
        <div class="exam-card">
            <div class="exam-grid">
                <div class="exam-item">
                    <label>Active Token</label>
                    <strong>${activePatient.token}</strong>
                </div>
                <div class="exam-item">
                    <label>Patient Details</label>
                    <strong>${activePatient.name} (${activePatient.age} yrs, ${activePatient.gender})</strong>
                </div>
                <div class="exam-item">
                    <label>Triage Priority</label>
                    <div>${getSeverityBadge(activePatient.severity)}</div>
                </div>
                <div class="exam-item">
                    <label>Chief Complaint</label>
                    <strong>${activePatient.complaint}</strong>
                </div>
                <div class="exam-item">
                    <label>Arrival Sequence</label>
                    <strong>#${activePatient.arrivalSequence} (${activePatient.arrivalTime})</strong>
                </div>
                <div class="exam-item">
                    <label>Assigned Bed</label>
                    <strong>${activePatient.assignedBed || "None (In Bay)"}</strong>
                </div>
            </div>

            <div style="margin: 12px 0;">
                <label for="doc-notes" style="font-size: 12px; font-weight: bold; color: #5d4037;">Doctor's Diagnosis & Notes:</label>
                <input type="text" id="doc-notes" value="${activePatient.doctorNotes || ""}" placeholder="Enter medical observations..." style="width: 100%; padding: 8px; font-size: 13px; border: 1px solid #bcaaa4; border-radius: 4px; box-sizing: border-box; margin-top: 4px;">
            </div>

            <div class="exam-actions-row">
                <label style="font-size: 13px; font-weight: bold;">Allocate Emergency Bed:</label>
                <select id="bed-select" style="padding: 6px 10px; font-size: 13px; border-radius: 4px; border: 1px solid #8d6e63;">
                    ${bedOptions}
                </select>
                <button type="button" id="btn-assign-bed" class="btn btn-sm btn-blue">Assign Bed</button>
                <button type="button" id="btn-discharge" class="btn btn-sm btn-green" style="margin-left: auto;">Discharge Patient</button>
                <button type="button" id="btn-transfer" class="btn btn-sm btn-orange">Transfer Specialty</button>
            </div>
        </div>
    `;

    // Bind Bay Actions
    document.getElementById("btn-assign-bed").addEventListener("click", () => {
        const bedId = document.getElementById("bed-select").value;
        if (!bedId) {
            alert("No available bed selected!");
            return;
        }
        const b = beds.find(x => x.id === bedId);
        if (b) {
            b.occupied = true;
            b.patientToken = activePatient.token;
            activePatient.assignedBed = bedId;
            activePatient.status = "ADMITTED IN BED";
            appendLog("BED ADMISSION", `Patient ${activePatient.token} (${activePatient.name}) admitted into Bed ${bedId}.`);
            renderExamBay();
            renderBedsMatrix();
        }
    });

    document.getElementById("btn-discharge").addEventListener("click", () => {
        if (!confirm(`Confirm discharge for ${activePatient.name} (${activePatient.token})?`)) return;
        const notes = document.getElementById("doc-notes").value;
        appendLog("PATIENT DISCHARGE", `Patient ${activePatient.token} discharged home. Doctor Notes: ${notes || "Routine treatment administered"}`);
        if (activePatient.assignedBed) {
            const b = beds.find(x => x.id === activePatient.assignedBed);
            if (b) {
                b.occupied = false;
                b.patientToken = "";
            }
        }
        activePatient = null;
        renderExamBay();
        renderBedsMatrix();
        renderStats();
    });

    document.getElementById("btn-transfer").addEventListener("click", () => {
        const dept = prompt("Enter Transfer Destination (e.g. Surgical OT, Cardiac ICU):", "Specialty Surgical Unit");
        if (!dept) return;
        appendLog("PATIENT TRANSFER", `Patient ${activePatient.token} transferred to ${dept}.`);
        if (activePatient.assignedBed) {
            const b = beds.find(x => x.id === activePatient.assignedBed);
            if (b) {
                b.occupied = false;
                b.patientToken = "";
            }
        }
        activePatient = null;
        renderExamBay();
        renderBedsMatrix();
        renderStats();
    });
}

// Render Stats Cards
function renderStats() {
    document.getElementById("stat-waiting").textContent = queue.length;
    document.getElementById("stat-treating").textContent = activePatient ? 1 : 0;
    document.getElementById("stat-total").textContent = totalRegisteredCount;
    const occupiedCount = beds.filter(b => b.occupied).length;
    document.getElementById("stat-beds").textContent = `${occupiedCount} / ${beds.length}`;
}

// Intake Form Submission
document.getElementById("patient-form").addEventListener("submit", (e) => {
    e.preventDefault();

    const name = document.getElementById("p-name").value.trim();
    const age = parseInt(document.getElementById("p-age").value, 10);
    const gender = document.getElementById("p-gender").value;
    const contact = document.getElementById("p-contact").value.trim() || "N/A";
    const complaint = document.getElementById("p-complaint").value.trim();
    const severity = parseInt(document.getElementById("p-severity").value, 10);

    if (!name || isNaN(age) || !complaint) {
        alert("Please fill all required fields!");
        return;
    }

    arrivalSequence++;
    totalRegisteredCount++;
    if (severityCounts[severity] !== undefined) severityCounts[severity]++;

    const token = `TK-${nextTokenNum++}`;
    const patient = {
        token: token,
        name: name,
        age: age,
        gender: gender,
        contact: contact,
        complaint: complaint,
        severity: severity,
        arrivalSequence: arrivalSequence,
        arrivalTime: new Date().toLocaleTimeString(),
        status: "WAITING",
        assignedBed: "",
        doctorNotes: ""
    };

    queue.push(patient);
    appendLog("PATIENT INTAKE", `Registered ${token} (${name}, ${age}y, Sev ${severity}) - "${complaint}". Enqueued into Min-Heap.`);

    renderQueueTable();
    document.getElementById("patient-form").reset();
    alert(`Success! Patient ${token} (${name}) registered and enqueued at Priority Position!`);
});

// Action: Advance Highest Priority Patient (Dequeue)
document.getElementById("btn-advance-queue").addEventListener("click", () => {
    if (queue.length === 0) {
        alert("Emergency Queue is currently EMPTY! No patients waiting to be advanced.");
        return;
    }

    if (activePatient) {
        if (!confirm(`Warning: Patient ${activePatient.token} (${activePatient.name}) is currently under examination. Discharge or admit them first, or click OK to replace examination slot.`)) {
            return;
        }
    }

    sortPriorityQueue();
    activePatient = queue.shift();
    activePatient.status = "UNDER TREATMENT";

    appendLog("PRIORITY DEQUEUE", `Advanced highest-priority patient ${activePatient.token} (${activePatient.name}, Level ${activePatient.severity}) to Examination Bay.`);

    // Auto-allocate appropriate bed if vacant
    const bedCategory = activePatient.severity === 1 ? "ICU" : (activePatient.severity === 2 ? "Trauma" : "Acute ER");
    const matchedBed = beds.find(b => b.type.includes(bedCategory) && !b.occupied) || beds.find(b => !b.occupied);
    if (matchedBed) {
        matchedBed.occupied = true;
        matchedBed.patientToken = activePatient.token;
        activePatient.assignedBed = matchedBed.id;
        appendLog("AUTO-BED ALLOCATION", `Auto-assigned bed ${matchedBed.id} to critical patient ${activePatient.token}.`);
    }

    renderQueueTable();
    renderExamBay();
    renderBedsMatrix();
});

// Action: Load Demo Simulation (6 Patients)
document.getElementById("btn-demo-data").addEventListener("click", () => {
    const demoCases = [
        { name: "Vikram Malhotra", age: 58, gender: "Male", contact: "+91-9811223344", complaint: "Acute crushing chest pain (STEMI)", severity: 2 },
        { name: "Aarav Sharma", age: 22, gender: "Male", contact: "+91-9876543210", complaint: "Deep forearm laceration with active bleeding", severity: 4 },
        { name: "Rajeshwari Devi", age: 67, gender: "Female", contact: "+91-9455112233", complaint: "Sudden collapse, unresponsive, suspected cardiac arrest", severity: 1 },
        { name: "Priya Nambiar", age: 34, gender: "Female", contact: "+91-9988776655", complaint: "Severe lower abdominal pain (Acute appendicitis)", severity: 3 },
        { name: "Suresh Sen", age: 45, gender: "Male", contact: "+91-9123456780", complaint: "High-speed motorcycle collision polytrauma", severity: 1 },
        { name: "Ananya Roy", age: 19, gender: "Female", contact: "+91-9011223344", complaint: "Migraine & routine inhaler prescription refill", severity: 5 }
    ];

    demoCases.forEach(item => {
        arrivalSequence++;
        totalRegisteredCount++;
        if (severityCounts[item.severity] !== undefined) severityCounts[item.severity]++;

        const token = `TK-${nextTokenNum++}`;
        queue.push({
            token: token,
            name: item.name,
            age: item.age,
            gender: item.gender,
            contact: item.contact,
            complaint: item.complaint,
            severity: item.severity,
            arrivalSequence: arrivalSequence,
            arrivalTime: new Date().toLocaleTimeString(),
            status: "WAITING",
            assignedBed: "",
            doctorNotes: ""
        });
        appendLog("DEMO INTAKE", `Injected ${token} (${item.name}, Level ${item.severity}) into queue.`);
    });

    renderQueueTable();
    alert("6 Emergency Demo Patients injected! Notice how Level 1 (Cardiac arrest & Polytrauma) jumped to #1 and #2 in the queue!");
});

// Download File Helpers
function downloadFile(filename, content) {
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

document.getElementById("btn-download-log").addEventListener("click", () => {
    let content = "======================================================================\n";
    content += "        HOSPITAL EMERGENCY DEPARTMENT - PATIENT AUDIT LOG (er_patient_log.txt)\n";
    content += "======================================================================\n\n";
    content += logEntries.join("\n\n");
    content += "\n\nEnd of Log.\n";
    downloadFile("er_patient_log.txt", content);
});

document.getElementById("btn-download-report").addEventListener("click", () => {
    const now = getTimestamp();
    const occupied = beds.filter(b => b.occupied).length;
    let report = "######################################################################\n";
    report += "#          HOSPITAL EMERGENCY DEPARTMENT - DAILY AUDIT REPORT         #\n";
    report += "######################################################################\n\n";
    report += `Report Timestamp : ${now}\n`;
    report += `Total Patients Registered : ${totalRegisteredCount}\n`;
    report += `Currently Waiting in Queue : ${queue.length}\n`;
    report += `Under Treatment in Bay     : ${activePatient ? 1 : 0}\n\n`;
    report += "TRIAGE SEVERITY DISTRIBUTION:\n";
    report += `  Level 1 (Critical)    : ${severityCounts[1]}\n`;
    report += `  Level 2 (Emergent)    : ${severityCounts[2]}\n`;
    report += `  Level 3 (Urgent)      : ${severityCounts[3]}\n`;
    report += `  Level 4 (Less Urgent) : ${severityCounts[4]}\n`;
    report += `  Level 5 (Non-Urgent)  : ${severityCounts[5]}\n\n`;
    report += `EMERGENCY BED OCCUPANCY: ${occupied} / ${beds.length} Beds (${((occupied / beds.length) * 100).toFixed(1)}%)\n\n`;
    report += "End of Daily Report.\n";
    downloadFile("daily_er_report.txt", report);
});

// Reset All Records
document.getElementById("btn-clear-all").addEventListener("click", () => {
    if (!confirm("Are you sure you want to reset all records and clear queue?")) return;
    queue = [];
    activePatient = null;
    arrivalSequence = 0;
    nextTokenNum = 1001;
    totalRegisteredCount = 0;
    severityCounts = { 1: 0, 2: 0, 3: 0, 4: 0, 5: 0 };
    beds.forEach(b => { b.occupied = false; b.patientToken = ""; });
    logEntries = [];
    appendLog("SYSTEM RESET", "All emergency department records and queues were purged.");
    renderQueueTable();
    renderExamBay();
    renderBedsMatrix();
    renderStats();
});

// Clock Updater
function updateClock() {
    const el = document.getElementById("clock-display");
    if (el) {
        el.textContent = `Time: ${new Date().toLocaleTimeString()}`;
    }
}
setInterval(updateClock, 1000);
updateClock();

// Initial Boot
appendLog("SYSTEM STARTUP", "Hospital Emergency Management System web portal initiated.");
renderQueueTable();
renderBedsMatrix();
renderExamBay();
renderStats();
