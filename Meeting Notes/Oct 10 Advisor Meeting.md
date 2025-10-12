# 📝 Meeting Notes Summary

## Meeting 2: Advisor Weekly Meeting (October 10, 2025)
**Participants:** Dr. Tacca (advisor), Eric Mutton, Adrianne Quick, Zack Potersnak, Dominic Nguyen, Jairo Thiongo  
**Purpose:** Formal update and advisor feedback on subsystem PCB progress, impedance control, and ordering plans.

---

### 1. Overall Status
- All PCBs routed and entering **final review and ordering** phase.  
- Next stage: implement **firmware architecture** and begin subsystem demo code.  
- Budget remains strong; next expenses will be PCB fabrication and potential stencil costs.

---

### 2. Power System (Zack)
- Power PCB complete with test points for voltage/current measurement.  
- Discussion on the **main battery switch** placement — advised to use external wiring for flexibility.  
- Confirmed connector current ratings (2.5 A limit acceptable).  
- Firmware development for **PD profiles (20 V / 25 W max)** planned.  
- Dr. Tacca emphasized ensuring the PD controller enforces only supported power profiles.

---

### 3. MCU Subsystem (Adrianne)
- Board complete, zero DRC/validation errors; only needs correction of **sharp trace corners** (for fabrication quality).  
- Discussion on **photolithography limitations** and why sharp corners reduce signal reliability.  
- MCU board (≈ 3 × 3 in) uses Arduino-sized mounting holes for prototype compatibility.  
- Next: begin **FreeRTOS and firmware architecture design** while waiting for fabrication.  
- Future revision will reduce board size after confirming functionality.

---

### 4. RF Subsystem (Eric)
- Split into **three modular boards:**  
  - **Transceiver** – based on TI reference design  
  - **Amplifier** – two mirrored circuits (VHF & UHF sides)  
  - **Switch Network** – connects both to one antenna  
- Removed all RF test points (interfere with signal integrity).  
- Advisor confirmed correctness of impedance control approach (target ≈ 50 Ω).  
- Advised to differentiate enable lines (EN1 → EN3/EN4) for clarity.  
- Discussed **four-layer boards** for better impedance control and routing options.  
- RF mentor (Dr. Henderson) provided additional feedback; further consultation with RF graduate students recommended.

---

### 5. Audio Subsystem (Dominic)
- PCB routed, only minor edge clearance errors remain.  
- Next step: firmware for **microphone and speaker I/O** via ESP32.  
- Advisor reminded that there’s one unified firmware handling all subsystems, not per-board code.  
- Dominic will add labeling for mic input location to prevent confusion.

---

### 6. HMI Subsystem (Eric & Zack)
- PCB routing finished; design ready for order.  
- Discussion of **keypad and display size constraints** — current version uses smaller tactile switches for initial test fit.  
- Advisor noted that component miniaturization can cause usability issues; acceptable for prototype.

---

### 7. Impedance Control Discussion
- Reviewed 50 Ω line impedance calculations using **KiCad’s built-in calculator**.  
- Dr. Tacca explained that **± 5 Ω variation** is acceptable for the sub-GHz range.  
- RF losses discussed: not heat-based, but due to reflection from mismatched impedance.  
- Recommended using **OSH Park’s 4-layer service** for prototypes and verifying impedance with grad-level RF labs.  
- Action item: Eric to follow up with Dr. Henderson and contact RF graduate students for practical testing.

---

### 8. Fabrication & Next Steps
- Team will **order stable subsystem PCBs separately** rather than waiting for all to be finalized.  
- Target: submit first board orders by **Monday** following the meeting.  
- RF board fabrication may occur later after impedance tuning.  
- Sunday lab session scheduled to finalize checks before submission.

---

### 9. Advisor Recommendations
- Keep routing corners rounded.  
- Label enables distinctly for clarity.  
- Begin firmware integration immediately after first boards arrive.  
- Use Sunday lab time for collective review before sending Gerber files.  
- Maintain modular test plan and emphasize documentation.

---

### 10. Action Items
- [ ] Adrianne → Fix routing corners; start firmware structure.  
- [ ] Eric → Refine impedance control; contact grad students.  
- [ ] Zack → Verify PD controller current handling.  
- [ ] Dominic → Add microphone label; prepare firmware hooks.  
- [ ] Team → Finalize ordering logistics and BOM updates.  
- [ ] Prepare follow-up report for next advisor meeting.  

---

### 11. Key Takeaways
- Dr. Tacca confirmed strong technical progress and readiness for PCB fabrication.  
- RF and impedance control require specialized follow-up.  
- Firmware phase begins immediately.  
- Orders will be split by readiness, maintaining forward momentum.  
