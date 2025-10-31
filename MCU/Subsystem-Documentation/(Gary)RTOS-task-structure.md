# OpenAir Lab – Week 1 FreeRTOS Task Architecture Sheet & ESP‑IDF Skeleton

> ESP32‑WROVER‑E (N8R8) + CC1200 + TFT HMI + BQ25620 power subsystem
>
> **Purpose:** A ready‑to‑print architecture sheet and a buildable ESP‑IDF project skeleton to establish tasks, priorities, timing, inter‑task comms, ISRs, and bus arbitration for the handheld VHF/UHF transceiver capstone.

---

## 1) Task Set (priorities, timing, stacks)

| Task | Purpose | Priority | Trigger | Period / Latency | Est. Stack |
|---|---|---:|---|---:|---:|
| **RadioRX** | Drain CC1200 RX FIFO, parse frames | **5 (high)** | Event (GDO0 ISR → `radio_evt_q`) | <2–5 ms from ISR | 3–4 KB |
| **RadioTX** | Packetize & transmit via CC1200 | 4 | Event (`radio_tx_q`) | N/A | 3–4 KB |
| **HMI** | Scan keypad, read encoder, update UI model | 3 | Periodic + GPIO ISR | 10–20 ms tick | 3–4 KB |
| **Display** | Render TFT from UI model | 2 | Periodic or event-driven | 33–100 ms frame | 5–8 KB (PSRAM for buffers) |
| **PowerMgr** | Fuel/charge status, battery alarms | 3 | Periodic | 500–1000 ms | 2–3 KB |
| **Logger** | Drain `log_q` → UART/flash/SD | 2 | Event (`log_q`) | <50 ms avg | 3–4 KB |
| **Housekeeping** | Stats, heap/PSRAM, RSSI polling | 1 | Periodic | 1–5 s | 2–3 KB |
| **Net/CLI (opt.)** | UART/Wi‑Fi shell for debug | 1 | Event | N/A | 4 KB |

**Priority rationale:** Don’t drop CC1200 RX → highest. TX next. HMI should feel responsive. Display/Logger lower. PowerMgr is periodic but important for alarms.

---

## 2) Inter‑Task Communication Map

- **Queues**
  - `radio_evt_q`: ISR → RadioRX (type `radio_isr_msg_t`, depth 16)
  - `radio_rx_q`: RadioRX → Display/Logger (type `frame_msg_t`, depth 32)
  - `radio_tx_q`: App/HMI → RadioTX (type `frame_msg_t`, depth 16)
  - `hmi_evt_q`: ISRs/poll → HMI (type `hmi_msg_t`, depth 32)
  - `log_q`: any task → Logger (pointer to char* or small struct, depth 64)
- **EventGroup** `sys_events`:
  - `EV_RADIO_LINK_UP`, `EV_BATT_LOW`, `EV_CFG_CHANGED`, ...
- **Mutexes**
  - `spi_bus_mutex` (recursive): arbitrates CC1200 + TFT
  - `i2c_bus_mutex`: arbitrates BQ25620 + future I²C parts

**Rule:** ISRs only enqueue/toggle bits—no SPI/I²C in ISRs. All bus ops in tasks behind mutexes.

---

## 3) ISR Handoff (minimal ISR design)

- **CC1200 GDO0/GDO2 ISRs:** push `RADIO_EVT_RX` / TX‑done into `radio_evt_q` (no parsing in ISR).
- **Encoder/Key ISRs:** push `ENC_STEP` / `KEY_DOWN/UP` into `hmi_evt_q` (timestamped).
- **Timers:** use `esp_timer` or FreeRTOS software timers to kick periodic work (e.g., HMI scan, PowerMgr).

---

## 4) Memory & Reliability

- Place big UI assets/framebuffers in **PSRAM**; keep DMA buffers in **internal RAM**.
- Enable high‑water marks and stack overflow checking (IDF menuconfig). Tune stacks after a 10‑minute burn‑in.
- Task watchdog for RadioRX/RadioTX/HMI; brownout detector ON. PowerMgr sets `EV_BATT_LOW` to notify Display/Logger.

---

## 5) Textual Interaction Diagram

```
[GDO0 ISR] --> radio_evt_q --> (RadioRX) --frames--> radio_rx_q --> (Display/Logger)
                      ^                                   ^
(App/HMI) --tx req--> radio_tx_q --> (RadioTX) -----------/
[GPIO ISR: keys/enc] --> hmi_evt_q --> (HMI) --UI cmds--> (Display)
(I2C timer) -------> (PowerMgr) --bits--> sys_events --> (Display/Logger)
(Any task) --logs--> log_q --> (Logger) --> UART/Flash/SD
               [spi_bus_mutex] arbitrates {CC1200, TFT}
               [i2c_bus_mutex] arbitrates {BQ25620, ...}
```

---

## 6) Week‑1 Verification Checklist

- [ ] Create all primitives (queues, event group, mutexes) in `app_main()`
- [ ] Wire ISRs (GDO0, encoder/keys) to enqueue tiny structs
- [ ] Prove SPI sharing: 1 CC1200 status read + 1 TFT "hello"/s behind mutex
- [ ] Log pipeline: `LOGI(TAG, ...)` → `log_q` → Logger → UART
- [ ] End‑to‑end demo: press button → HMI posts PTT → RadioTX enqueues fake frame → Logger prints
- [ ] 10‑min run; record high‑water marks & queue stats; adjust stacks/depths

---

## 7) ESP‑IDF Project Skeleton (buildable)

**Suggested tree**
```
openair_freertos_skeleton/
├─ CMakeLists.txt
├─ sdkconfig.defaults
├─ main/
│  ├─ CMakeLists.txt
│  ├─ app_main.c
│  ├─ buses.h/.c          (SPI/I2C init + mutexes)
│  ├─ isr.h/.c            (GPIO ISRs: GDO0, encoder, keys)
│  ├─ radio.h/.c          (CC1200 stubs: init, read_fifo, write_fifo)
│  ├─ hmi.h/.c            (scan matrix, encoder handler, UI model)
│  ├─ display.h/.c        (TFT stubs, frame pacing)
│  ├─ power.h/.c          (BQ25620 stubs)
│  └─ logging.h/.c        (queue‑based logger)
└─ components/            (optional: real drivers later)
```

**Top‑level `CMakeLists.txt`**
```cmake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(openair_freertos_skeleton)
```

**`main/CMakeLists.txt`**
```cmake
idf_component_register(SRCS
    "app_main.c"
    "buses.c" "isr.c" "radio.c" "hmi.c" "display.c" "power.c" "logging.c"
    INCLUDE_DIRS "."
)
```

**`sdkconfig.defaults` (excerpt)**
```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS=y
CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY=y
CONFIG_ESP_SYSTEM_PANIC_PRINT_REBOOT=y
CONFIG_SPIRAM_USE_CAPS_ALLOC=y
CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY=n
CONFIG_BT_ENABLED=n
CONFIG_WIFI_ENABLED=n
```

**`main/app_main.c` (minimal, compiles; fill TODOs)**
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "buses.h"
#include "isr.h"
#include "radio.h"
#include "hmi.h"
#include "display.h"
#include "power.h"
#include "logging.h"

static const char *TAG = "OpenAir";

#define EV_RADIO_LINK_UP   (1<<0)
#define EV_BATT_LOW        (1<<1)

QueueHandle_t radio_evt_q, radio_rx_q, radio_tx_q, hmi_evt_q, log_q;
EventGroupHandle_t sys_events;
SemaphoreHandle_t spi_bus_mutex, i2c_bus_mutex;

void task_radio_rx(void *p);
void task_radio_tx(void *p);
void task_hmi(void *p);
void task_display(void *p);
void task_power(void *p);
void task_logger(void *p);

void app_main(void) {
    ESP_LOGI(TAG, "Booting OpenAir skeleton");

    // Primitives
    radio_evt_q = xQueueCreate(16, sizeof(radio_isr_msg_t));
    radio_rx_q  = xQueueCreate(32, sizeof(frame_msg_t));
    radio_tx_q  = xQueueCreate(16, sizeof(frame_msg_t));
    hmi_evt_q   = xQueueCreate(32, sizeof(hmi_msg_t));
    log_q       = xQueueCreate(64, sizeof(char*));
    sys_events  = xEventGroupCreate();

    spi_bus_mutex = xSemaphoreCreateRecursiveMutex();
    i2c_bus_mutex = xSemaphoreCreateMutex();

    buses_init();        // TODO: SPI/I2C, device handles for CC1200/TFT/BQ25620
    isr_init();          // TODO: attach GDO0, encoder/keys (GPIO ISRs)
    radio_init();        // TODO
    display_init();      // TODO (show splash)
    power_init();        // TODO
    hmi_init();          // TODO
    logging_init();      // optional

    xTaskCreatePinnedToCore(task_radio_rx, "RadioRX", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(task_radio_tx, "RadioTX", 4096, NULL, 4, NULL, 1);
    xTaskCreate(task_hmi,     "HMI",      4096, NULL, 3, NULL);
    xTaskCreate(task_display, "Display",  6144, NULL, 2, NULL);
    xTaskCreate(task_power,   "Power",    3072, NULL, 3, NULL);
    xTaskCreate(task_logger,  "Logger",   4096, NULL, 2, NULL);
}

// === RadioRX ===
void task_radio_rx(void *p) {
    radio_isr_msg_t evt;
    for (;;) {
        if (xQueueReceive(radio_evt_q, &evt, portMAX_DELAY)) {
            xSemaphoreTakeRecursive(spi_bus_mutex, portMAX_DELAY);
            // TODO cc1200_read_fifo(...)
            xSemaphoreGiveRecursive(spi_bus_mutex);
            frame_msg_t f = { .len = 0 };
            // TODO fill f.data / f.len / f.rssi
            xQueueSend(radio_rx_q, &f, 0);
        }
    }
}

// === RadioTX ===
void task_radio_tx(void *p) {
    frame_msg_t f;
    for (;;) {
        if (xQueueReceive(radio_tx_q, &f, portMAX_DELAY)) {
            xSemaphoreTakeRecursive(spi_bus_mutex, portMAX_DELAY);
            // TODO cc1200_write_fifo(f.data, f.len); strobe TX
            xSemaphoreGiveRecursive(spi_bus_mutex);
        }
    }
}

// === HMI ===
void task_hmi(void *p) {
    TickType_t last = xTaskGetTickCount();
    hmi_msg_t evt;
    for (;;) {
        if (xQueueReceive(hmi_evt_q, &evt, 0)) {
            // TODO update UI model / enqueue commands
            logging_printf("HMI evt: %d\n", evt.type);
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(20)); // 50 Hz scan
        // TODO poll keypad rows/cols; debounce; push KEY_DOWN/UP
    }
}

// === Display ===
void task_display(void *p) {
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        // TODO render from UI model (double buffer if needed)
        vTaskDelayUntil(&last, pdMS_TO_TICKS(50)); // ~20 FPS
    }
}

// === PowerMgr ===
void task_power(void *p) {
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);
        // TODO read BQ25620 regs
        xSemaphoreGive(i2c_bus_mutex);
        // if low: xEventGroupSetBits(sys_events, EV_BATT_LOW);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(1000));
    }
}

// === Logger ===
void task_logger(void *p) {
    char *line = NULL;
    for (;;) {
        if (xQueueReceive(log_q, &line, portMAX_DELAY)) {
            if (line) {
                printf("%s\n", line);
                // TODO optionally write to file; then free(line)
            }
        }
    }
}
```

**`main/buses.h` (excerpt)**
```c
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
extern SemaphoreHandle_t spi_bus_mutex, i2c_bus_mutex;
void buses_init(void);
```

**`main/isr.h` (types used by queues)**
```c
#pragma once
#include <stdint.h>

typedef enum { RADIO_EVT_RX, RADIO_EVT_TX_DONE, RADIO_EVT_FIFO_OVF } radio_evt_t;
typedef struct { radio_evt_t type; uint16_t bytes; int8_t rssi; } radio_isr_msg_t;

typedef enum { KEY_DOWN, KEY_UP, ENC_STEP } hmi_evt_t;
typedef struct { hmi_evt_t type; uint8_t key_id; int8_t enc_delta; uint32_t ts_ms; } hmi_msg_t;
```

**Stub headers to create (`radio.h`, `hmi.h`, `display.h`, `power.h`, `logging.h`)**
- Provide `*_init()` prototypes and minimal structs.

---

## 8) Bring‑Up Notes

- **SPI sharing proof:** do one CC1200 `STATUS` read + one TFT "hello" per second with `spi_bus_mutex` held around each transfer.
- **End‑to‑end test:** map a key to enqueue a fake `frame_msg_t` to `radio_tx_q`; have Logger print when TX request received.
- **Sizing:** after a 10‑minute run, print `uxTaskGetStackHighWaterMark()` for each task; adjust.

---

## 9) TODO Hooks for Week‑2

- Swap fake frames for real CC1200 FIFO reads/writes; add RX overrun recovery.
- Basic menu UI (channel, volume, squelch) via encoder; render to TFT.
- Battery icon + voltage%, `EV_BATT_LOW` banner from PowerMgr.
- Optional: CLI over UART for live stats.



---

# Unified Pin Map (ESP32‑WROVER‑E)

> Two SPI buses (VSPI→CC1200, HSPI→TFT), one I²C bus (BQ25620 + keypad expander), and full‑duplex I²S (ICS‑43434 mic + MAX98357A amp). Avoid boot strapping pins (GPIO0/2/5/12/15) for CS/inputs; GPIO34–39 are input‑only. Many WROVER modules reserve GPIO16/17 internally.

| Bus / Subsystem | Signal (ESP32) | Dir. | ESP32 GPIO | Connected To | Notes |
|---|---|---:|---:|---|---|
| **VSPI → CC1200** | SCLK | → | **18** | CC1200 SCLK | Default high‑speed VSPI clock. |
|  | MOSI | → | **23** | CC1200 SI | Default VSPI MOSI. |
|  | MISO | ← | **19** | CC1200 SO | Default VSPI MISO. |
|  | CS | → | **32** | CC1200 CSn | Non‑strapping, safe for CS. |
|  | GDO0 (IRQ) | ← | **34** | CC1200 GDO0 | Input‑only pin—ideal for IRQ. |
|  | GDO2 (IRQ) | ← | **35** | CC1200 GDO2 | Second IRQ line (TX‑done, etc.). |
| **HSPI → TFT** | SCLK | → | **14** | TFT SCK | Common HSPI SCK. |
|  | MOSI | → | **13** | TFT MOSI | 4‑wire write‑only typical. |
|  | MISO (opt.) | ← | — | — | Most TFTs don’t need MISO; free the pin. |
|  | CS | → | **4** | TFT CS | Non‑strapping on most boards; OK as output. |
|  | DC | → | **26** | TFT D/C | Free GPIO, easy routing. |
|  | RST | → | — | — | Tie to 3V3 with RC or to board reset; no GPIO needed. |
| **I²C → BQ25620 + Keypad Expander** | SDA | ↔ | **21** | BQ25620 SDA + Expander SDA | Shared I²C bus; 4.7–10 kΩ pull‑ups. |
|  | SCL | ↔ | **22** | BQ25620 SCL + Expander SCL | Shared I²C bus. |
|  | Expander INT | ← | **39** | MCP23017/PCF8574 INT | Wakes HMI task on change. |
| **I²S audio (full‑duplex)** | BCLK | → | **27** | Mic SCK + Amp BCLK | One clock for both devices. |
|  | LRCLK / WS | → | **25** | Mic WS + Amp LRCLK | Shared LRCLK/WS. |
|  | DOUT (ESP32→amp) | → | **33** | MAX98357A DIN | Standard I²S; no MCLK needed. |
|  | DIN (mic→ESP32) | ← | **36** | ICS‑43434 SD | Digital I²S mic; no MCLK. |
|  | Mic SEL | — | — | ICS‑43434 SEL | Tie GND (Left) or VDD (Right). |
| **HMI encoder** | A / B | ← | — | On I²C expander | Put both phases on expander; INT→GPIO39. |
| **Keypad** | Rows/Cols | ← | — | On I²C expander | Entire matrix on expander; INT→GPIO39. |
| **Debug UART** | TX0 / RX0 | ↔ | **1 / 3** | USB‑UART | Keep default for logs/CLI. |

**Downloads:**
- One‑pager PDF (table + color diagram): **OpenAir_Pin_Map_OnePager.pdf**
- Color bus/interrupt diagram (PNG): **bus_interrupt_diagram.png**



---

# Unified Pin Map (v2, with STUSB4500 USB‑PD)

> VSPI→CC1200 • HSPI→TFT • I²C→BQ25620 + Keypad Expander + **STUSB4500** • I²S (full‑duplex) → Mic + Amp  
> Avoid boot strapping pins (GPIO0/2/5/12/15). GPIO34–39 are input‑only. Many WROVER modules reserve GPIO16/17 internally.

| Bus / Subsystem | Signal (ESP32) | Dir. | ESP32 GPIO | Connected To | Notes |
|---|---|---:|---:|---|---|
| **VSPI → CC1200** | SCLK | → | **18** | CC1200 SCLK | Default high‑speed VSPI clock. |
|  | MOSI | → | **23** | CC1200 SI | Default VSPI MOSI. |
|  | MISO | ← | **19** | CC1200 SO | Default VSPI MISO. |
|  | CS | → | **32** | CC1200 CSn | Non‑strapping, safe for CS. |
|  | GDO0 (IRQ) | ← | **34** | CC1200 GDO0 | Input‑only pin—ideal for IRQ. |
|  | GDO2 (IRQ) | ← | **35** | CC1200 GDO2 | Second IRQ line (TX‑done, etc.). |
| **HSPI → TFT** | SCLK | → | **14** | TFT SCK | Common HSPI SCK. |
|  | MOSI | → | **13** | TFT MOSI | 4‑wire write‑only typical. |
|  | MISO (opt.) | ← | — | — | Most TFTs don’t need MISO; free the pin. |
|  | CS | → | **4** | TFT CS | Non‑strapping on most boards; OK as output. |
|  | DC | → | **26** | TFT D/C | Free GPIO, easy routing. |
|  | RST | → | — | — | Tie to 3V3 with RC or to board reset; no GPIO needed. |
| **I²C → BQ25620 + Keypad Expander + STUSB4500** | SDA | ↔ | **21** | BQ25620 SDA + Expander SDA + STUSB4500 SDA | Shared I²C bus; 4.7–10 kΩ pull‑ups. |
|  | SCL | ↔ | **22** | BQ25620 SCL + Expander SCL + STUSB4500 SCL | Shared I²C bus. |
|  | Expander INT | ← | **39** | MCP23017/PCF8574 INT | Wakes HMI task on change. |
|  | **STUSB4500 ALERT** | ← | **38** | STUSB4500 ALERT | Open‑drain; pull‑up to 3V3; PD event interrupt. |
| **I²S audio (full‑duplex)** | BCLK | → | **27** | Mic SCK + Amp BCLK | One clock for both devices. |
|  | LRCLK / WS | → | **25** | Mic WS + Amp LRCLK | Shared LRCLK/WS. |
|  | DOUT (ESP32→amp) | → | **33** | MAX98357A DIN | Standard I²S; no MCLK needed. |
|  | DIN (mic→ESP32) | ← | **36** | ICS‑43434 SD | Digital I²S mic; no MCLK. |
|  | Mic SEL | — | — | ICS‑43434 SEL | Tie GND (Left) or VDD (Right). |
| **HMI encoder** | A / B | ← | — | On I²C expander | Put both phases on expander; INT→GPIO39. |
| **Keypad** | Rows/Cols | ← | — | On I²C expander | Entire matrix on expander; INT→GPIO39. |
| **Debug UART** | TX0 / RX0 | ↔ | **1 / 3** | USB‑UART | Keep default for logs/CLI. |

**Non‑MCU wiring (for your schematic)**
- **STUSB4500 → USB‑C receptacle:** CC1 ↔ A5, CC2 ↔ B5. Optional **dead‑battery**: tie CC1DB→CC1 and CC2DB→CC2.  
- **VBUS path:** STUSB4500 **VBUS_EN_SNK** drives a high‑side FET (or power switch) feeding **BQ25620 VIN** from **VBUS**.  
- **VBUS discharge:** Use **VBUS_VS_DISCH/DISCH** to safely discharge when detaching or stepping down PDO.  
- **Addresses:** Suggest **STUSB4500 = 0x28**, **BQ25620 = 0x6B**, **MCP23017/PCF8574 = 0x20–0x27** (set by jumpers). Ensure all unique.

**Downloads (updated):**
- One‑pager PDF (table + color diagram): **OpenAir_Pin_Map_OnePager_v2.pdf**
- Color bus/interrupt diagram (PNG): **bus_interrupt_diagram_v2.png**

