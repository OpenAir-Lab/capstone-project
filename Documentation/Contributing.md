# Contributing to OpenAir Lab – Modular Handheld Radio Project

Thank you for your interest in contributing to the OpenAir Lab Capstone project!  
This repository is intended to be a fully open-source, educational platform for learning RF systems, embedded programming, PCB design, and modular hardware integration.  
We welcome contributions from students, hobbyists, engineers, and educators.

This document outlines the guidelines for contributing code, hardware designs, documentation, and issue reports.

---

#  Table of Contents
1. [Ways to Contribute](#ways-to-contribute)  
2. [Contribution Workflow](#contribution-workflow)  
3. [Branching & Pull Request Guidelines](#branching--pull-request-guidelines)  
4. [Coding Standards](#coding-standards)  
5. [Hardware Standards](#hardware-standards)  
6. [Documentation Standards](#documentation-standards)  
7. [Issue Reporting Guidelines](#issue-reporting-guidelines)  
8. [Community Expectations](#community-expectations)  

---

# 1. Ways to Contribute

You can contribute in many ways depending on your background:

### Firmware Contributions
- Implement new subsystem drivers (CC1200, I²C expander, ST7789 display, etc.)
- Improve FreeRTOS task structure or add new features.
- Add example code for modulinos.
- Fix bugs or optimize existing code.

### Hardware Contributions
- Improve module schematics or PCB layout.
- Add testpoints, revise signal integrity, or improve routing.
- Expand the modular ecosystem with new boards.

### Documentation Contributions
- Improve bring-up guides, testing checklists, or diagrams.
- Clarify README sections.
- Add educational explanations for students learning embedded systems.

### Testing & QA
- Run integration tests.
- Report or fix issues found during build, flashing, or board bring-up.

---

# 2. Contribution Workflow

Follow these steps to propose a contribution:

1. **Fork** the repository to your GitHub account.  
2. Create a **feature branch** in your fork:  
```
git checkout -b feature/<short-description>
```
3. Make changes while adhering to coding, hardware, or documentation standards.  
4. **Test thoroughly** (compiles, boots, UI responds, no regressions).  
5. Commit using clear messages (see below).  
6. Push the branch and submit a **Pull Request (PR)** to the `main` branch.  
7. Maintainers will review your PR and may request changes.  
8. Once approved, the PR will be merged.

---

# 3. Branching & Pull Request Guidelines

### Branch Naming
Use descriptive names:
- `feature/cc1200-rx-routine`
- `fix/i2c-expander-init`
- `docs/power-modulino-update`
- `hardware/revB-routing-fixes`

### Commit Message Style
Follow this pattern:
```
<type>: <short summary>
Optional long description explaining what changed and why.
```

Types:
- `feat` – new feature  
- `fix` – bug fix  
- `docs` – documentation changes  
- `refactor` – code restructuring  
- `hardware` – PCB or schematic changes  
- `test` – adding or modifying tests

Example:
```
feat: implement basic SPI loopback test for MCU modulino
```

### Pull Request Requirements
- Provide **clear description** of what was changed.
- Indicate **related issue numbers**, if applicable.
- Include screenshots, logs, or diagrams when relevant.
- Ensure the PR passes any automated checks.

---

# 4. Coding Standards

### General Requirements
- Use **C++17** for embedded firmware (PlatformIO).  
- Follow project formatting conventions (braces on same line, 4 spaces).  
- Keep code modular (separate driver logic from application logic).  
- No hard-coded pin numbers — use `pinmap.h`.

### FreeRTOS Guidelines
- Avoid blocking calls in high-frequency tasks.  
- Use queues/semaphores for ISR → task communication.  
- Keep ISRs short.

### Error Handling
- Check return codes and failure states.  
- Use `ESP_LOGx()` macros where appropriate.  

---

# 5. Hardware Standards

### Required KiCad Files
Each modulino hardware directory must contain:
- `.kicad_pro`, `.kicad_sch`, `.kicad_pcb`
- Schematic PDF  
- PCB fabrication outputs (Gerber, drill files)  
- BOM + iBOM  

### Layout Expectations
- Follow Espressif hardware design guidelines.  
- Use solid ground planes when possible.  
- Decoupling capacitors must be placed close to IC power pins.  
- Include silkscreen labels for connectors, testpoints, and critical components.

### Review Expectations
All hardware PRs must include:
- Render images  
- Design rule checks (DRC) passed  
- Electrical rule checks (ERC) passed  

---

# 6. Documentation Standards

Documentation should be:
- Clear and beginner-friendly  
- Free of ambiguous or missing steps  
- Written in Markdown  
- Contain diagrams when helpful  
- Stored within the appropriate `/Documentation/` folder

Examples:
- `/Documentation/system_architecture/`  
- `/Documentation/testing/`  
- `/Documentation/hardware/`  

---

# 7. Issue Reporting Guidelines

When opening an issue, include:

### Required fields
- **Description of problem**
- **Steps to reproduce**
- **Expected behavior**
- **Actual behavior**
- **Environment details**  
  - Board revision  
  - Firmware version  
  - Toolchain (PlatformIO/Arduino/ESP-IDF)

### Optional fields (very helpful)
- Screenshots  
- Serial logs  
- Scope/logic analyzer captures  
- Schematics or layout excerpts for hardware issues  

---

# 8. Community Expectations

By contributing, you agree to:

- Be respectful and constructive.  
- Help maintain a positive, inclusive environment.  
- Follow the project’s **Code of Conduct**.  
- Write clear, maintainable contributions.  

We are building an open-source educational ecosystem—thank you for helping make it better!

---

If you have questions or need clarification, please open an issue or contact a project maintainer.

**Happy building! ⚡📡**
