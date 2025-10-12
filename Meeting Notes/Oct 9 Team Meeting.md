# Meeting Notes Summary

## Meeting 1: Internal Slide Review (October 9, 2025)
**Participants:** Adrianne Quick, Eric Mutton, Zack Potersnak, Dominic Nguyen, Jairo Thiongo  
**Purpose:** Internal review of PCB designs and progress before advisor meeting.

---

### 1. General Discussion
- The team clarified what “PCB review” entails — a peer-level review before bringing in external feedback from mentors or professionals (e.g., Dr. Tacca).  
- Agreed to reconvene as a full team to review all designs and then individually schedule consultations with professors for specialized feedback (e.g., power electronics).  
- Consensus that multiple subsystems have diverged since the last joint review, so synchronization is necessary.

---

### 2. Project Direction
- Discussion about balancing **hardware vs. firmware focus** — team agreed to begin shifting attention toward firmware and integration planning.  
- Emphasis on using upcoming meetings with Dr. Tacca to discuss **firmware architecture and system-level design**.

---

### 3. PCB and Ordering Progress
- Members agreed that **individual boards should be ordered as they’re ready**, rather than waiting for a bulk order.  
- Testing can proceed in parallel with firmware development using development kits (e.g., MEMS mic, gyro, BQ chips).  
- A timeline update was noted as overdue but not a priority for the advisor meeting.

---

### 4. Subsystem Updates

#### **Power/BMS**
- Zack reported that the **BMS PCB** is finished with 3D models and test points.  
- Discussion on test point placement and accessibility (on back of board).

#### **MCU (Adrianne)**
- MCU board complete with no errors or warnings; routing improvements planned (sharper corners).  
- Acknowledged large physical size — revision 2 will optimize board area and placement.  
- Discussion on mounting holes and potential 3D-printed standoffs or plates.  
- Future revisions will include optional test points and labeled silkscreen text.

#### **RF Module (Eric)**
- RF system now divided into **three boards**: transceiver, amplifier, and switch network.  
- Learned from RF mentor that “test points for RF” aren’t practical — removed them to prevent interference.  
- Next steps: impedance control (~50 Ω traces), possible 4-layer PCB for routing quality.  
- Recognized that RF fabrication cost increases with board area and layer count (2-layer → $60; 4-layer → $120).

#### **Audio (Dominic)**
- Audio board routed with no warnings.  
- Plans to clearly label microphone port and improve visibility for assembly/testing.

#### **HMI (Eric & Zack)**
- Reduced PCB size for cost efficiency.  
- Planning integration of keypad, display, and optional navigation buttons (rotary encoder fallback).  
- Focused on ergonomic layout and minimizing complexity for first test revision.

---

### 5. Action Items
- [ ] Schedule full internal PCB review and final design cleanup.  
- [ ] Begin firmware test development while waiting for boards.  
- [ ] Update timeline before next advisor meeting.  
- [ ] Review mounting and mechanical fit for all boards.  
- [ ] Prepare test and validation plan for subsystem integration.  
- [ ] Adrianne to correct trace angles and consider adding on-board labels.  
- [ ] Eric to remove RF test points and adjust layer stackup.  
- [ ] Dominic to relabel mic position clearly.  
- [ ] Zack to assist others with board reviews.

---

### 6. Key Takeaways
- Focus is shifting from pure PCB design to **firmware development and system integration**.  
- Importance of individual initiative — team members free to advance testing independently.  
- Iteration cycle: **Review → Fabricate → Test → Revise** is now in effect.  
