// ============================================================================
// Hospital Emergency Management System - Frontend Automated Test Suite
// Verifies Min-Heap DSA, Bed Allocation, and Clinical Workflows in ui/app.js
// ============================================================================

const fs = require('fs');
const path = require('path');
const vm = require('vm');

let testsRun = 0;
let testsPassed = 0;

function assert(condition, message) {
    testsRun++;
    if (condition) {
        testsPassed++;
        console.log(`  \x1b[32m[PASS]\x1b[0m ${message}`);
    } else {
        console.error(`  \x1b[31m[FAIL]\x1b[0m ${message}`);
    }
}

// Set up mock browser environment
const domElements = {};
function createMockElement(id) {
    return {
        id,
        value: '',
        textContent: '',
        innerHTML: '',
        children: [],
        appendChild(child) { this.children.push(child); },
        addEventListener(event, fn) { this[`on_${event}`] = fn; },
        reset() { this.value = ''; },
        scrollTop: 0,
        scrollHeight: 100
    };
}

const mockDocument = {
    getElementById: (id) => {
        if (!domElements[id]) {
            domElements[id] = createMockElement(id);
        }
        return domElements[id];
    },
    createElement: (tag) => {
        const el = createMockElement(tag);
        el.className = '';
        el.title = '';
        return el;
    },
    body: {
        appendChild: () => {},
        removeChild: () => {}
    },
    addEventListener: () => {}
};

const sandbox = {
    window: {},
    document: mockDocument,
    console: console,
    Date: Date,
    Blob: class {},
    URL: { createObjectURL: () => "mock-url", revokeObjectURL: () => {} },
    alert: () => {},
    confirm: () => true,
    prompt: () => "Mock Department",
    setInterval: () => {},
    setTimeout: () => {}
};
sandbox.window = sandbox;

vm.createContext(sandbox);

// Load the unified ui/app.js
const appPath = path.join(__dirname, '..', 'ui', 'app.js');
const appCode = fs.readFileSync(appPath, 'utf8');
vm.runInContext(appCode, sandbox);

console.log("\x1b[36m\x1b[1m======================================================================\x1b[0m");
console.log("\x1b[36m\x1b[1m       WEB UI - AUTOMATED SIMULATION & DSA TEST SUITE                 \x1b[0m");
console.log("\x1b[36m\x1b[1m======================================================================\x1b[0m");

const app = sandbox.ERApp;
const state = app.state;

// 1. Central State Initialization
console.log("\n\x1b[1m--- 1. Testing Central State Store ---\x1b[0m");
assert(typeof app === 'object', "ERApp initialized as global object");
assert(Array.isArray(state.queue), "ERApp.state.queue is initialized as array");
assert(state.beds.length === 22, "ERApp.state.beds initialized with exactly 22 beds");

// 2. Priority Queue (Min-Heap DSA)
console.log("\n\x1b[1m--- 2. Testing Priority Queue (Min-Heap DSA) ---\x1b[0m");
app.enqueue({ token: "TK-001", name: "Low Acuity", severity: 5, arrivalSequence: 1, status: "WAITING" });
app.enqueue({ token: "TK-002", name: "High Acuity", severity: 1, arrivalSequence: 2, status: "WAITING" });
app.enqueue({ token: "TK-003", name: "Mid Acuity", severity: 3, arrivalSequence: 3, status: "WAITING" });

assert(state.queue.length === 3, "Queue size is 3 after enqueues");
let firstOut = app.dequeue();
assert(firstOut.token === "TK-002" && firstOut.severity === 1, "Level 1 patient dequeued first (Min-Heap priority ordering)");
let secondOut = app.dequeue();
assert(secondOut.token === "TK-003" && secondOut.severity === 3, "Level 3 patient dequeued second");
let thirdOut = app.dequeue();
assert(thirdOut.token === "TK-001" && thirdOut.severity === 5, "Level 5 patient dequeued third");
assert(state.queue.length === 0, "Queue is empty after dequeues");

// Strict FIFO tie-breaking for equal severity
app.enqueue({ token: "TK-A", name: "Patient A", severity: 2, arrivalSequence: 10, status: "WAITING" });
app.enqueue({ token: "TK-B", name: "Patient B", severity: 2, arrivalSequence: 11, status: "WAITING" });
let fifo1 = app.dequeue();
let fifo2 = app.dequeue();
assert(fifo1.token === "TK-A" && fifo2.token === "TK-B", "Strict FIFO arrival order maintained for equal triage level");

// 3. Emergency Bed Management
console.log("\n\x1b[1m--- 3. Testing Emergency Bed Management ---\x1b[0m");
let icu01 = state.beds.find(b => b.id === "ICU-01");
assert(icu01.occupied === false, "ICU-01 initially vacant");
app.assignBed("ICU-01", "TK-TEST");
assert(icu01.occupied === true && icu01.patientToken === "TK-TEST", "Bed assignment successful");
app.releaseBed("ICU-01");
assert(icu01.occupied === false && icu01.patientToken === "", "Bed release resets occupancy and token");

// Auto-allocation test
const critPatient = { token: "TK-CRIT", severity: 1, assignedBed: "" };
const allocatedId = app.autoAllocateForPatient(critPatient);
assert(allocatedId.startsWith("ICU-"), "Level 1 patient auto-assigned an ICU bed");
assert(critPatient.assignedBed === allocatedId, "Patient record updated with assigned bed ID");
app.releaseBed(allocatedId);

// 4. Realistic Cases Simulation
console.log("\n\x1b[1m--- 4. Testing Realistic Simulation Cases (30 Unique Patients) ---\x1b[0m");
const pool = app.getCases();
assert(pool.length === 30, "Simulation pool contains exactly 30 unique patient cases");

// Verify all 30 names are unique
const nameSet = new Set();
let duplicatesFound = 0;
pool.forEach(c => {
    if (nameSet.has(c.name.toLowerCase())) {
        duplicatesFound++;
    }
    nameSet.add(c.name.toLowerCase());
});
assert(duplicatesFound === 0, "All 30 patient cases have 100% unique names in JavaScript pool");

// Inject 6 patients via simulation
app.injectDemoBatch();
assert(state.queue.length === 6, "Batch injection enqueued 6 patients");
assert(state.queue[0].severity === 1, "Level 1 patient jumped to the front of queue after injection");

// 5. Clinical & Doctor Workflow
console.log("\n\x1b[1m--- 5. Testing Clinical & Doctor Workflow ---\x1b[0m");
app.advanceNextPatient();
assert(state.activePatient !== null, "Active patient established in Doctor Examination Bay");
assert(state.activePatient.status === "UNDER TREATMENT", "Active patient status updated to UNDER TREATMENT");
assert(state.queue.length === 5, "Queue decremented from 6 to 5");

// 6. Administrator & KPI Stats
console.log("\n\x1b[1m--- 6. Testing Administrator Analytics & KPI Stats ---\x1b[0m");
app.renderStats();
assert(domElements["stat-waiting"].textContent == 5, "Patients waiting stat correctly displays 5");
assert(domElements["stat-treating"].textContent == 1, "Patients treating stat correctly displays 1");

console.log("\n\x1b[1m======================================================================\x1b[0m");
console.log(`\x1b[1m WEB TEST RESULTS: ${testsPassed} / ${testsRun} Tests Passed (${((testsPassed/testsRun)*100).toFixed(1)}%)\x1b[0m`);
console.log("\x1b[1m======================================================================\x1b[0m");

if (testsPassed !== testsRun) {
    process.exit(1);
}
