# Critical Design Review with Advisor (Dr. Tacca)

**Date/Time:** August 28, 2025  
**Attendees:** All team members (Eric, Adrianne, Zack, Jairo, Dominic) and Advisor (Dr. Tacca)  

## Agenda / Purpose  
- Present finalized CDR slide deck.  
- Receive advisor feedback on technical design, subsystem progress, and presentation quality.  
- Clarify advisor expectations for prototyping and final deliverables.  

## Discussion Summary  
- **General Feedback:** Begin PCB design early (recommended OSHPark). Move beyond breadboards to prototyping boards and enclosures. Demo in November; final report due this semester.  
- **Abstract & Requirements:** Zack presented abstract. DT asked about PCB experience.  
- **Subsystem Updates:**  
  - Power Delivery (Jairo): Use protoboards.  
  - Battery Management (Zack): No major comments.  
  - MCU (Adrianne): Confirmed 2 SPI lines.  
  - RF (Eric): No comments.  
  - Audio (Dominic): DT requested clear signal path (mic → I2S → ESP32 → RF), digital-to-analog conversion, RX compatibility, sample rate handling.  
  - HMI (Eric): Reviewed rotary encoder, volume knob, display. DT asked about stereo vs. mono audio and potentiometer type (recommended log taper, brands Bourne/ALPS). Also raised question of VHF/UHF switching.  
- **Educational Goals:** GitHub Wiki + YouTube channel approved.  
- **Statements:** Ethics (Dominic), Lifelong Learning (Eric), Contemporary Issues (Jairo), Multidisciplinary (Adrianne).  
- **Key Advisor Notes:**  
  - Signal format must align with ham standards.  
  - Clarify mono vs. stereo requirements.  
  - Confirm VHF/UHF switching.  
  - 10A switch justification.  
  - Use UTD fab shop for 3D prints.  
  - Implement error handling for handshake fails.  
  - Engage amateur radio clubs and professors for outreach.  

## Action Items / Next Steps  
- Start PCB design; explore OSHPark.  
- Replace breadboards with prototyping boards and enclosure (consider 3D print).  
- Clarify audio path and conversion.  
- Confirm mono vs. stereo; select log-taper potentiometer.  
- Confirm VHF/UHF switching.  
- Verify transmitted signal format matches ham standards.  
- Create GitHub Wiki; reserve YouTube channel.  
- Submit STL files for fab shop print.  
- Reach out to amateur radio clubs and faculty for feedback.  
