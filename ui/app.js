// Hospital Emergency Management System - UI Simulation Controller

const queue = [
    { token: "TK-1003", name: "Rajeshwari Devi", age: 67, severity: 1, complaint: "Cardiac arrest / Unresponsive", seq: 3 },
    { token: "TK-1005", name: "Suresh Sen", age: 45, severity: 1, complaint: "Severe polytrauma motorcycle collision", seq: 5 },
    { token: "TK-1001", name: "Vikram Malhotra", age: 58, severity: 2, complaint: "Acute chest pain (STEMI)", seq: 1 },
    { token: "TK-1004", name: "Priya Nambiar", age: 34, severity: 3, complaint: "Acute appendicitis with fever", seq: 4 },
    { token: "TK-1002", name: "Aarav Sharma", age: 22, severity: 4, complaint: "Deep forearm laceration", seq: 2 },
    { token: "TK-1006", name: "Ananya Roy", age: 19, severity: 5, complaint: "Migraine & prescription refill", seq: 6 }
];

let activeExamPatient = null;
let arrivalCounter = 6;
let nextTokenId = 1007;

const beds = [
    { id: "ICU-01", type: "ICU", occupied: false, patient: null },
    { id: "ICU-02", type: "ICU", occupied: false, patient: null },
    { id: "ICU-03", type: "ICU", occupied: false, patient: null },
    { id: "ICU-04", type: "ICU", occupied: false, patient: null },
    { id: "TRM-01", type: "Trauma", occupied: false, patient: null },
    { id: "TRM-02", type: "Trauma", occupied: false, patient: null },
    { id: "TRM-03", type: "Trauma", occupied: false, patient: null },
    { id: "TRM-04", type: "Trauma", occupied: false, patient: null },
    { id: "ER-01", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-02", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-03", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-04", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-05", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-06", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-07", type: "Acute ER", occupied: false, patient: null },
    { id: "ER-08", type: "Acute ER", occupied: false, patient: null },
    { id: "OBS-01", type: "Observation", occupied: false, patient: null },
    { id: "OBS-02", type: "Observation", occupied: false, patient: null },
    { id: "OBS-03", type: "Observation", occupied: false, patient: null },
    { id: "OBS-04", type: "Observation", occupied: false, patient: null },
    { id: "OBS-05", type: "Observation", occupied: false, patient: null },
    { id: "OBS-06", type: "Observation", occupied: false, patient: null }
];

function getSeverityBadge(severity) {
    const labels = {
        1: "Level 1: Critical",
        2: "Level 2: Emergent",
        3: "Level 3: Urgent",
        4: "Level 4: Less Urgent",
        5: "Level 5: Non-Urgent"
    };
    return `<span class="badge badge-level-${severity}">${labels[severity]}</span>`;
}

// Min-heap priority comparator: severity first, arrival seq second
function sortQueue() {
    queue.sort((a, b) => {
        if (a.severity !== b.severity) return a.severity - b.severity;
        return a.seq - b.seq;
    });
}

function renderQueue() {
    sortQueue();
    const tbody = document.getElementById("queue-body");
    tbody.innerHTML = "";

    if (queue.length === 0) {
        tbody.innerHTML = `<tr><td colspan="7" style="text-align: center; color: var(--text-muted); padding: 24px;">No patients currently waiting in emergency queue.</td></tr>`;
    } else {
        queue.forEach((p, index) => {
            const tr = document.createElement("tr");
            tr.innerHTML = `
                <td><strong>#${index + 1}</strong></td>
                <td><code>${p.token}</code></td>
                <td><strong>${p.name}</strong></td>
                <td>${p.age} yrs</td>
                <td>${getSeverityBadge(p.severity)}</td>
                <td>${p.complaint}</td>
                <td>Seq ${p.seq}</td>
            `;
            tbody.appendChild(tr);
        });
    }

    renderStats();
}

function renderBeds() {
    const container = document.getElementById("beds-container");
    container.innerHTML = "";

    beds.forEach(b => {
        const div = document.createElement("div");
        div.className = `bed-card ${b.occupied ? "occupied" : "vacant"}`;
        div.innerHTML = `
            <div class="bed-id">${b.id}</div>
            <div class="bed-type">${b.type}</div>
            <span class="bed-status-tag ${b.occupied ? "occupied" : "vacant"}">${b.occupied ? "OCCUPIED" : "VACANT"}</span>
        `;
        div.addEventListener("click", () => toggleBed(b.id));
        container.appendChild(div);
    });

    renderStats();
}

function toggleBed(bedId) {
    const bed = beds.find(b => b.id === bedId);
    if (!bed) return;
    bed.occupied = !bed.occupied;
    renderBeds();
}

function renderActivePatient() {
    const container = document.getElementById("current-exam-bay");
    if (!activeExamPatient) {
        container.innerHTML = `<div class="exam-empty"><p>No patient currently in examination bay. Click "Advance Highest Priority" to dequeue.</p></div>`;
        return;
    }

    container.innerHTML = `
        <div class="patient-grid">
            <div class="patient-item"><span>Token</span><strong>${activeExamPatient.token}</strong></div>
            <div class="patient-item"><span>Patient Name</span><strong>${activeExamPatient.name} (${activeExamPatient.age} yrs)</strong></div>
            <div class="patient-item"><span>Triage Acuity</span><strong>Level ${activeExamPatient.severity}</strong></div>
            <div class="patient-item"><span>Chief Complaint</span><strong>${activeExamPatient.complaint}</strong></div>
        </div>
    `;
}

function renderStats() {
    document.getElementById("total-waiting").textContent = queue.length;
    document.getElementById("under-treatment").textContent = activeExamPatient ? 1 : 0;
    const occupiedCount = beds.filter(b => b.occupied).length;
    document.getElementById("beds-occupied").textContent = `${occupiedCount} / ${beds.length}`;
    const criticalCount = queue.filter(p => p.severity === 1).length + (activeExamPatient && activeExamPatient.severity === 1 ? 1 : 0);
    document.getElementById("critical-cases").textContent = criticalCount;
}

// Action: Dequeue highest priority patient
document.getElementById("btn-advance").addEventListener("click", () => {
    if (queue.length === 0) {
        alert("Priority queue is empty! No patients waiting.");
        return;
    }

    sortQueue();
    activeExamPatient = queue.shift();

    // Auto-allocate appropriate bed if vacant
    const bedType = activeExamPatient.severity === 1 ? "ICU" : (activeExamPatient.severity === 2 ? "Trauma" : "Acute ER");
    const vacantBed = beds.find(b => b.type === bedType && !b.occupied) || beds.find(b => !b.occupied);
    if (vacantBed) vacantBed.occupied = true;

    renderActivePatient();
    renderQueue();
    renderBeds();
});

// Action: Add new patient
document.getElementById("btn-add-demo").addEventListener("click", () => {
    const names = ["Rohan Patel", "Sneha Roy", "Amit Verma", "Sunita Nair", "Karan Kapoor"];
    const complaints = [
        { c: "Unconscious / Severe Hypoxia", s: 1 },
        { c: "Sudden facial droop and speech slur", s: 2 },
        { c: "Kidney stone acute pain", s: 3 },
        { c: "Ankle sprain with swelling", s: 4 },
        { c: "Viral flu and cough", s: 5 }
    ];

    const pick = complaints[Math.floor(Math.random() * complaints.length)];
    const p = {
        token: `TK-${nextTokenId++}`,
        name: names[Math.floor(Math.random() * names.length)],
        age: Math.floor(Math.random() * 50) + 18,
        severity: pick.s,
        complaint: pick.c,
        seq: ++arrivalCounter
    };

    queue.push(p);
    renderQueue();
});

// Initial boot
renderQueue();
renderBeds();
renderActivePatient();

// Live clock update
function updateClock() {
    const clockEl = document.getElementById("clock-display");
    if (clockEl) {
        const now = new Date();
        clockEl.textContent = `Live Status: ${now.toLocaleTimeString()}`;
    }
}
setInterval(updateClock, 1000);
updateClock();
