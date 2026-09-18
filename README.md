
# HOSPITAL EMERGENCY MANAGEMENT SYSTEM



## MOTIVATION FOR THIS PROJECT

Patients arriving at a hospital's emergency department (ER) require immediate medical attention, yet this process is often managed manually or without a standardized procedure. Such methods can be slow, leading to dangerous delays in treating critically ill patients, and become difficult to manage during high-pressure situations or periods of overcrowding. A lack of proper prioritization often results in long wait times for patients, while medical staff struggle to manage emergency care and bed allocation. The primary objective of this project is to develop an integrated and efficient management system for the hospital ER that automates tasks such as patient registration, prioritization, and the tracking of patient location or movement within the department. This system facilitates the creation of priority-based patient queues and the management of emergency bed availability, while simultaneously minimizing human error and saving time during critical moments. It incorporates distinct access interfaces for medical staff—who admit and treat high-priority patients—and receptionists, who handle patient registration. Furthermore, by addressing a real-world healthcare challenge, students can gain practical insight into various C programming concepts, such as priority queue data structures, file handling, pointers, and functions.



## Features

- Light/dark mode toggle
- Priorty System
- Cross platform


## Roadmap

- Additional browser support

- Add more integrations


## FAQ

#### 1. What is your project?

Answer:
Our project is a Hospital Emergency Management System, a console-based C application that manages emergency patient registration, triage, priority-based queue processing, patient status and daily records.

#### 2. What is the main objective?

Answer:
The main objective is to organize emergency patients according to the severity of their condition and maintain their records efficiently using a priority queue and file handling.

#### 3. Why did you choose this project?

Answer:
We chose it because emergency departments need an organized way to handle patients when multiple patients arrive at the same time. A priority-based system can help represent which patient should be processed first.

#### 4. What is the main DSA concept used?

Answer:
The main DSA concept is a Priority Queue.

#### 5. Why did you use a Priority Queue instead of a normal queue?

Answer:
A normal queue follows FIFO — First In, First Out. But in an emergency department, the most critical patient may need to be treated before someone who arrived earlier. Therefore, a priority queue is more suitable.

#### 6. What are enqueue and dequeue?

Answer:
Enqueue: Adding a patient to the queue.
Dequeue: Removing/serving the highest-priority patient from the queue.
The synopsis specifically identifies these as core queue operations.

#### 7. How is patient priority decided?

Answer:
The priority is based on the severity/triage level assigned to the patient during registration.

#### 8. What happens when a new patient arrives?

Answer:

Patient Registration

       ↓
Token Generation

       ↓
Severity Assignment

       ↓
Patient Structure Created

       ↓
Enqueue into Priority Queue

       ↓
Save Record


This follows the registration algorithm described in the synopsis.

#### 9. What programming language did you use?

Answer:
We used C programming language.

#### 10. What C concepts are used?

Answer:
The project uses:

- Structures
- Functions
- Pointers
- File handling
- Arrays or linked lists
- Conditional statements
- Loops
- Menu-driven programming

The synopsis explicitly identifies structures, pointers, functions and file handling as implementation concepts.

#### 11. Why are structures used?

Answer:
Structures are used to store multiple pieces of information about a patient as a single record, such as patient details, token and severity.

#### 12. Why is file handling required?

Answer:
If the queue and records were kept only in RAM, the information would be lost when the program closes. File handling allows patient and daily emergency records to be stored and retrieved later.

#### 13. Which files are used?

Answer:

er_patient_log.txt — stores patient records.

daily_er_report.txt — stores daily emergency reports.

#### 14. What is the role of the receptionist?

Answer:
The receptionist can register patients, assign/record severity, generate tokens and check the current serving status.

#### 15. What is the role of medical staff/admin?

Answer:
Medical staff or administrators have broader privileges such as advancing the highest-priority patient, viewing emergency history, resetting daily logs and managing beds.

#### 16. What is role-based access?

Answer:
It means different users have different permissions. A receptionist doesn't have the same operational privileges as medical staff or administrators.

#### 17. What happens if the priority queue is empty?

Answer:
The system checks whether any patient is waiting before performing the dequeue operation. If the queue is empty, no patient can be advanced.

#### 18. What happens when a patient is served?

Answer:
The highest-priority patient is removed from the queue, the current serving status is updated, and the operation is recorded in the relevant log/report.

#### 19. What is the limitation of your project?

Answer:
The current project is designed for a local desktop/console environment and small-to-medium-scale data management. It does not currently provide network or online access.

#### 20. What are the future possibilities?

Answer:
Possible future enhancements include:

- Database instead of text files
- Web/mobile interface
- Online patient registration
- Real-time bed management
- AI-assisted triage
- Blockchain-based record integrity
- Notifications
- Advanced analytics and reporting


#### 21. Can blockchain be used in this project?

Answer:
Yes, as a future enhancement. Blockchain could be used to maintain a tamper-evident audit trail for important emergency records. However, the current synopsis uses text files for storage, so we should not claim blockchain is already implemented.

#### 22. What is the biggest advantage of your system?

Answer:
Its main advantage is that it provides a structured priority-based patient processing mechanism instead of relying on a manually maintained patient sequence.

#### 23. What happens if two patients have the same priority?

Answer:
This depends on the queue implementation. The synopsis specifies severity-based priority but does not define a tie-breaking rule. So don't invent one in your viva unless you have implemented it.

## Authors

- [@Sahil Roy](https://github.com/sahilroykgp-art)


## 🚀 About Me
I'm a Computer Applications student specialised in Artifical Intelligence & Data Science.

