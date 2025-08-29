# Meeting Notes: Team Meeting (members only)
### Date: July 23, 2025

### Attendees: Eric Mutton, Adrianne Quick, Zack Potersnak, Dominic Nguyen, Jairo Thiongo
## 1. Meeting Goals
- Review and finalize the presentation slides for the advisor meeting with Dr. Taka.
- Debrief and reflect on the Preliminary Design Review (PDR).
- Review and address questions raised by Dr. Taka during the PDR.
- Assess status updates across subsystems and finalize procurement protocols.
- Confirm clarity of slide content and GitHub issue linkage.
## 2. Action Items
- Submit slides to Dr. Taka by **12:00 PM, July 24, 2025**.
- Ensure all slides are linked to corresponding GitHub tasks using task numbers.
- Confirm official procurement process with Dr. Taka (mentor approval protocol).
- Share Figma block diagram access with all team members.
- Utilize OneDrive templates for expense tracking and request submission.
- Select and order 2–3 different battery types for initial testing.
- Temporarily gray out HMI section from slides due to lack of new content.
## 3. Slide Preparation & Structure
The weekly presentation slides will continue using the modified PDR template, which has proven effective so far. Slides will include:
- Task updates and completions since last meeting
- Updated system overview (builds toward full schematic)
- Power subsystem progress: battery, charger, USB-PD, and fuel gauge
- Microcontroller selection and architecture discussion
- RF subsystem component investigation
- Firmware architecture planning (RTOS vs. Arduino discussion)
- Questions for Dr. Taka slide for unresolved or new design decisions
## 4. Subsystem Updates
### Power Subsystem
- Battery Selection:
  - Focused on 2.4–3.5Ah lithium-ion options.
  - Pouch cell form factor preferred for compact design.
  - Testing will include different form factors (18650 vs. flat pouch).
- Fuel Gauge:
  - Selected external IC for accuracy and flexibility.
  - I²C interface; allows percentage readouts and supports user interface feedback.
- USB-PD & Charging:
  - TI buck-boost IC considered to handle a range of input voltages.
  - Ensures safe charging from lower-output power banks.
  - Compatibility with USB 2.0/3.0 under review (Dr. Taka’s concern).
### Microcontroller
- ESP32-WROVER selected for its familiarity, Arduino IDE compatibility, and low power consumption.
- Rejected Raspberry Pi Zero W for being overkill in power, complexity, and educational value.
- User interface design will dictate future MCU complexity if goals evolve (e.g., touchscreen).
### RF Subsystem
- Investigation into amplifier types (PA vs. LNA) and vendor options (Qorvo, Mini-Circuits).
- Clarified “gain block” terminology and importance of signal chain design.
- Slides will focus on progress and not rehash explanations for Dr. Taka.
### Firmware Architecture
- Pin mapping in early phase; awaiting clarity from other subsystem interfaces.
- Initial planning for RTOS vs. Arduino module approach is ongoing.
- Researching GPIO expanders to address pin limitations.
- Will document architecture in GitHub and link relevant tasks to presentation slides.
## 5. Collaboration Tools
- **GitHub:** Task tracking, issues, documentation; all slides to reference task numbers.
- **Figma:** System block diagram shared via team account; access centralized.
- **OneDrive:** Used for expense tracking and part request forms (budget cap: $2000).
## 6. Questions for Dr. Taka
- What is the official protocol for ordering components and gaining mentor approval?
- Will older USB-C cable standards be compatible with our power delivery implementation?
- Can we proceed with early-stage battery orders before the design is finalized?
## 7. Workflow & Process Notes
- Weekly slide decks serve as design status updates.
- Tuesday post-lecture work sessions proposed to encourage collaborative slide creation.
- Slides should avoid “no update” filler; include only significant progress or clearly marked TBDs.
- All slides should include GitHub task numbers for tracking and future traceability.
