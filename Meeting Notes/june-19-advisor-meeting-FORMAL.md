### **Meeting Summary with Dr. Tacca (DT)**

### 1. Project Presentation & General Feedback

- Slides were presented to DT regarding the HAM radio/amplifier circuit project.
- DT remains skeptical about the project’s viability and differentiation.
- A clear justification is needed for why a two-stage RF amplifier is preferable over a single-stage alternative.
- Recommendation: Evaluate existing market solutions (e.g., Qorvo, TI) to determine where your solution can stand out.

### 2. Technical Notes & Component Feedback

- Qorvo was recommended for RF power specs but their evaluation board costs $600 and is suited for higher-end RF applications.
- DT questioned whether the team is building an amplifier or a transceiver.
- Some components and ideas discussed:
    - SDR options: ADLM, RTL-SDR
    - Duplex operation chip: CC1200
    - Microcontrollers: ESP32 (not ideal for some applications), alternatives like MSP432 should be considered.
    - Suggestion to seek guidance via TI forums.

### 3. Strategic Considerations

- Clarify target audience: hobbyists/amateurs vs. commercial/profit-driven.
- Competing directly with companies like TI or Qorvo is not feasible.
- Project might be better suited as an open source educational tool, especially if not cost-competitive.
- Important to define what “cost” means in your context (e.g., low-volume hand-assembled vs. high-volume manufacturing).
- Consider the cost and logistics of third-party assembly vs. hand-built prototypes.

### 4. Educational Value and User Interface

- Emphasize educational benefits: aim for a “walkie talkie”-style device that is customizable and teaches radio principles.
- HMI (Human-Machine Interface): needs to be accessible for users with little to no prior radio knowledge.
- Avoid “black box” design — encourage user transparency and learning.
- Evaluate the trade-off between learning experience and ease of use.

---

### Follow-Up Questions for Clarity

1. Scope Clarification: Is the project focus exclusively an amplifier, a transceiver, or a modular system (where users can explore both)?
2. Target Market: Is the primary goal to serve amateur radio hobbyists, educators, or general consumers interested in SDR?
3. ~~Two-Stage Justification: What performance metrics or learning objectives justify using a two-stage amplifier over a simpler one?~~
4. Open Source Strategy: Are you planning to open-source the hardware and software? If so, will you pursue any certifications or community partnerships?
5. Learning vs. Use: How much technical depth is expected from the user? Will you provide interactive tutorials, a GUI interface, or documentation to assist learning?
