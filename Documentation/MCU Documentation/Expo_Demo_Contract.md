# OpenAir Lab – Expo Demo Contract

**Project:** OpenAir Lab Modular Handheld Radio  
**Demo Date:** December 12, 2025 (within 12 days)  
**Version:** v1.0 – Demo Contract (Scope Freeze)

---

## 1. Demo Objective

Show a **single, reliable vertical slice** of the handheld radio system:

> Power on → live status screen → user changes frequency → PTT engages TX (with timeout) → returns safely to RX, with battery and RSSI visible the whole time.

This contract defines the **minimum behavior** required for the expo demo. Anything not listed here is **nice-to-have** and must not block this behavior.

---

## 2. Features in Scope (Must Work)

1. **RX frequency visible**
   - The main screen shows the current **receive frequency** (in Hz or MHz).
   - Frequency must update on screen whenever the user changes it.

2. **RSSI visible**
   - The screen displays a **received signal strength** indicator (numeric and/or bar).
   - RSSI is updated periodically (e.g., several times per second) while in RX.

3. **PTT triggers TX**
   - A designated **PTT control** (button or equivalent input) switches the radio from RX to TX.
   - When TX is active:
     - A clear visual indicator (e.g., “TX” label or color change) appears on the screen.
     - RF path is configured for transmit (CC1200 + RF switch + amp as appropriate for the demo).

4. **TX timeout safety**
   - A **TX timeout** is enforced (e.g., fixed duration chosen by the team).
   - If PTT is held longer than the timeout:
     - The system automatically returns to RX.
     - The TX indicator clears.
   - After timeout, the system remains stable and ready for another PTT.

5. **Encoder (or keypad) changes frequency**
   - A **single primary input method** (rotary encoder or keypad) is used to adjust the RX frequency.
   - Turning/pressing this input:
     - Changes the frequency in a predictable way (defined step size).
     - Immediately updates the displayed frequency.
   - Behavior must be debounced/filtered enough to feel usable during the demo.

6. **Battery % visible**
   - The main screen shows **battery status**:
     - At minimum: a numeric percentage or approximate level indicator.
   - Battery information is **polled periodically** (e.g., every 2–5 seconds).
   - Displayed value updates when new data is read.

7. **Screen updates live**
   - The display is driven by a **status update loop or task** (e.g., 5–10 Hz refresh).
   - On-screen values for:
     - Frequency
     - RSSI
     - TX/RX state
     - Battery status  
     are updated **without** flickering full-screen redraws or freezing.
   - The system remains responsive (display and inputs continue to work) during:
     - Normal RX
     - TX (while PTT active)
     - TX timeout transitions

---

## 3. Subsystems in Scope

For this demo, the following subsystems are considered **in scope** and must be integrated well enough to support the behavior above:

- **MCU modulino**
  - Core firmware, FreeRTOS task structure, and state management.
- **RF modulino**
  - CC1200 transceiver (RX/TX control, RSSI).
  - RF switch and amplifier signals required for safe RX/TX path selection.
- **HMI modulino**
  - Display (status screen).
  - One primary user input: rotary encoder *or* keypad (PTT here).
- **BMS / Power modulino**
  - Battery level readout sufficient to compute or approximate battery percentage.

Other modulinos or features may be present but must **not** be required for a successful demo.

---

## 4. Success Criteria (Pass/Fail)

The demo is considered **successful** if all of the following hold in a live run:

1. On power-up, the device reliably:
   - Boots to a main status screen.
   - Shows a valid initial frequency and battery indication.

2. While in RX:
   - RSSI changes on the screen when the RF environment changes.
   - The user can change the displayed frequency using the encoder/keypad.

3. PTT behavior:
   - Press/hold PTT → system enters TX, clearly indicated on screen.
   - Releasing PTT (before timeout) → system returns to RX cleanly.
   - Holding PTT beyond the configured timeout → system automatically returns to RX.

4. Throughout the demo run:
   - Battery indication updates periodically.
   - The UI remains responsive (no freezes or crashes).
   - The system can perform the above sequence **multiple times** without reboot.

If any of these conditions fail consistently, the demo does **not** meet this contract.

---

## 5. Out of Scope (Must Not Block Demo)

The following items are explicitly **out of scope** for this demo and may only be addressed if everything above is complete and stable:

- Full audio chain (microphone → RF modulation → speaker audio quality).
- Multi-band or multi-mode operation beyond what is required to prove the concept.
- Advanced menu systems, settings pages, or configuration screens.
- USB Power Delivery profile switching and detailed power analytics.
- Any PC/tool integration or over-the-air configuration.

---

**Sign-off:**  
This Demo Contract is intended to focus all work toward a single, repeatable, expo-ready behavior. Changes to this scope should be rare and agreed upon by the team.

