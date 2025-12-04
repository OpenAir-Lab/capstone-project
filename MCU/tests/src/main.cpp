// I2C + SPI + System/Memory/Core/GPIO diagnostics + Self-contained Interrupt Test (every 3 minutes)
// Framework: Arduino (ESP32-WROVER-E)
// - Runs the whole diagnostic suite at boot, then every 180 seconds.
// - Adds a "self-interrupt" test using one GPIO (default 25) to generate and detect edges with no external hardware.

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <esp_system.h>
#include <rom/ets_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

// ===== I2C parameters =====
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const uint32_t I2C_FREQ = 100000; // 100 kHz

// ===== SPI parameters (VSPI loopback test) =====
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK  18
#define SPI_CS   5
SPIClass spi(SPI);

// ===== Self-interrupt test pin =====
// Choose a safe, bidirectional GPIO that is not used by other peripherals on your devkit.
// GPIO 25 is a common, safe default on many ESP32 dev boards.
static const int SIM_IRQ_PIN = 25;

// ===== Periodic scheduler (3 minutes) =====
static const unsigned long TEST_PERIOD_MS = 180000UL;
static unsigned long lastTestMs = 0;

// ===== Optional GPIO Output Test (leave empty by default) =====
static const int GPIO_TEST_OUTPUTS[] = {
  // Add pins here if you want to toggle actual outputs during tests:
  // 2, 4, 16, 17, 26, 27, 32, 33
};
static const size_t GPIO_TEST_OUTPUTS_COUNT =
  sizeof(GPIO_TEST_OUTPUTS) / sizeof(GPIO_TEST_OUTPUTS[0]);

// ===== Safe input-only pins to read (non-invasive snapshot) =====
static const int GPIO_INPUT_ONLY[] = {34, 35, 36, 39};
static const size_t GPIO_INPUT_ONLY_COUNT =
  sizeof(GPIO_INPUT_ONLY) / sizeof(GPIO_INPUT_ONLY[0]);

// ===== Interrupt event structure and queue (used by the self-interrupt test) =====
typedef struct {
  uint8_t pin;        // pin number (SIM_IRQ_PIN)
  uint8_t level;      // 0 or 1 at ISR capture time
  uint32_t t_us;      // timestamp (micros)
} irq_event_t;

static QueueHandle_t irqQueue = nullptr;
static volatile uint32_t simIrqCount = 0;  // counts since last diagnostics period

// ===== Forward declarations =====
void runAllDiagnosticsOnce();
void scanI2C();
void spiLoopbackTest();
void printSystemInfo();
void printMemoryInfo();
void checkCores();
void gpioReadInputs();
void gpioOutputToggleTest(int cycles, int delayMs);

// Self-interrupt helpers
void setupSelfInterrupt();
void processSelfIrqEvents();
void simulateInterruptBursts(int bursts, int edgesPerBurst, int edgeDelayMs);
void resetSelfIrqCounter();

// ===== I2C scan =====
void scanI2C() {
  Serial.println();
  Serial.println("Scanning I2C bus...");
  uint8_t found = 0;

  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("Found device at 0x%02X (%d)\n", address, address);
      found++;
    } else if (error == 4) {
      Serial.printf("Unknown error at address 0x%02X\n", address);
    }
  }
  if (found == 0)
    Serial.println("No I2C devices found.");
  else
    Serial.printf("Scan complete. %d device%s found.\n", found, (found == 1 ? "" : "s"));
}

// ===== SPI loopback test =====
void spiLoopbackTest() {
  Serial.println();
  Serial.println("=== SPI Loopback Test ===");
  Serial.println("Ensure MOSI and MISO are connected (GPIO23 ↔ GPIO19) with a short jumper.");

  spi.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);

  uint8_t txData[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
  uint8_t rxData[8] = {0};

  digitalWrite(SPI_CS, LOW);
  spi.transferBytes(txData, rxData, sizeof(txData));
  digitalWrite(SPI_CS, HIGH);

  bool pass = true;
  for (int i = 0; i < 8; i++) {
    Serial.printf("0x%02X -> 0x%02X\n", txData[i], rxData[i]);
    if (txData[i] != rxData[i]) pass = false;
  }
  Serial.println(pass ? "SPI loopback PASS" : "SPI loopback FAIL");
  spi.end();
}

// ===== System / chip info =====
void printSystemInfo() {
  Serial.println();
  Serial.println("=== System Info ===");
  Serial.printf("SDK:            %s\n", ESP.getSdkVersion());
  Serial.printf("Chip model:     %s\n", ESP.getChipModel());
  Serial.printf("Chip revision:  %d\n", ESP.getChipRevision());
  Serial.printf("CPU cores:      %d\n", ESP.getChipCores());
  Serial.printf("CPU freq (MHz): %d\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash size:     %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Flash speed:    %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("PSRAM found:    %s\n", psramFound() ? "yes" : "no");
}

// ===== Memory usage =====
void printMemoryInfo() {
  Serial.println();
  Serial.println("=== Memory Usage ===");
  Serial.printf("Heap total:     %u bytes\n", ESP.getHeapSize());
  Serial.printf("Heap free:      %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Heap max block: %u bytes\n", ESP.getMaxAllocHeap());
  Serial.printf("Heap min free:  %u bytes\n", ESP.getMinFreeHeap());
  if (psramFound()) {
    Serial.printf("PSRAM total:    %u bytes\n", ESP.getPsramSize());
    Serial.printf("PSRAM free:     %u bytes\n", ESP.getFreePsram());
  } else {
    Serial.println("PSRAM total:    0 (not present)");
    Serial.println("PSRAM free:     0");
  }
  Serial.printf("Sketch size:    %u bytes\n", ESP.getSketchSize());
  Serial.printf("Free sketch:    %u bytes\n", ESP.getFreeSketchSpace());
}

// ===== Core check =====
static void corePrinterTask(void* pv) {
  int core = xPortGetCoreID();
  Serial.printf("[corePrinterTask] Running on core: %d\n", core);
  vTaskDelete(NULL);
}

void checkCores() {
  Serial.println();
  Serial.println("=== Core Check ===");
  Serial.printf("setup()/caller core: %d\n", xPortGetCoreID());
#if CONFIG_FREERTOS_UNICORE
  xTaskCreate(corePrinterTask, "corePrint", 2048, nullptr, 1, nullptr);
#else
  xTaskCreatePinnedToCore(corePrinterTask, "corePrint0", 2048, nullptr, 1, nullptr, 0);
  xTaskCreatePinnedToCore(corePrinterTask, "corePrint1", 2048, nullptr, 1, nullptr, 1);
#endif
  delay(100);
}

// ===== GPIO snapshot (input-only pins) =====
void gpioReadInputs() {
  Serial.println();
  Serial.println("=== GPIO Input-Only Read (non-invasive) ===");
  for (size_t i = 0; i < GPIO_INPUT_ONLY_COUNT; ++i) {
    int pin = GPIO_INPUT_ONLY[i];
    pinMode(pin, INPUT);
    int level = digitalRead(pin);
    Serial.printf("GPIO %02d: %s\n", pin, level ? "HIGH" : "LOW");
  }
}

// ===== Optional GPIO output toggle test =====
void gpioOutputToggleTest(int cycles, int delayMs) {
  if (GPIO_TEST_OUTPUTS_COUNT == 0) {
    Serial.println();
    Serial.println("=== GPIO Output Toggle Test ===");
    Serial.println("No output pins listed; skipping. Add GPIOs to GPIO_TEST_OUTPUTS[] if desired.");
    return;
  }
  Serial.println();
  Serial.println("=== GPIO Output Toggle Test (verify wiring before enabling) ===");
  for (size_t i = 0; i < GPIO_TEST_OUTPUTS_COUNT; ++i) {
    pinMode(GPIO_TEST_OUTPUTS[i], OUTPUT);
    digitalWrite(GPIO_TEST_OUTPUTS[i], LOW);
  }
  for (int c = 0; c < cycles; ++c) {
    for (size_t i = 0; i < GPIO_TEST_OUTPUTS_COUNT; ++i) {
      digitalWrite(GPIO_TEST_OUTPUTS[i], (c & 1) ? HIGH : LOW);
    }
    delay(delayMs);
  }
  for (size_t i = 0; i < GPIO_TEST_OUTPUTS_COUNT; ++i) {
    pinMode(GPIO_TEST_OUTPUTS[i], INPUT);
  }
  Serial.println("GPIO output toggle test complete.");
}

// ===== Self-interrupt ISR (kept tiny; runs in IRAM) =====
static IRAM_ATTR void isr_self() {
  irq_event_t e;
  e.pin   = SIM_IRQ_PIN;
  e.level = gpio_get_level((gpio_num_t)SIM_IRQ_PIN);
  e.t_us  = micros();
  simIrqCount++;
  if (irqQueue) xQueueSendFromISR(irqQueue, &e, nullptr);
}

// ===== Configure the self-interrupt test =====
void setupSelfInterrupt() {
  // A small queue to collect IRQ events for printing in loop()
  irqQueue = xQueueCreate(32, sizeof(irq_event_t));

  // Attach interrupt on CHANGE so both rising and falling edges are captured
  pinMode(SIM_IRQ_PIN, INPUT); // input mode for attachInterrupt (direction will be switched after)
  attachInterrupt(digitalPinToInterrupt(SIM_IRQ_PIN), isr_self, CHANGE);

  // After attaching the ISR, switch to OUTPUT so we can drive the pin and generate edges
  pinMode(SIM_IRQ_PIN, OUTPUT);
  digitalWrite(SIM_IRQ_PIN, LOW);

  Serial.println();
  Serial.printf("Self-interrupt configured on GPIO %d (CHANGE). Pin will be toggled in software to generate edges.\n", SIM_IRQ_PIN);
}

// ===== Drain and print any queued self-interrupt events =====
void processSelfIrqEvents() {
  if (!irqQueue) return;
  irq_event_t e;
  int drained = 0;
  while (xQueueReceive(irqQueue, &e, 0) == pdTRUE) {
    Serial.printf("IRQ event: pin=%u level=%u t=%u us\n", e.pin, e.level, e.t_us);
    drained++;
    if (drained >= 32) break; // avoid monopolizing CPU
  }
}

// ===== Actively generate edges on the SIM_IRQ_PIN =====
void simulateInterruptBursts(int bursts, int edgesPerBurst, int edgeDelayMs) {
  // This function toggles SIM_IRQ_PIN to produce a known number of edges.
  // With CHANGE-triggered ISR, every toggle produces one event.
  for (int b = 0; b < bursts; ++b) {
    for (int i = 0; i < edgesPerBurst; ++i) {
      digitalWrite(SIM_IRQ_PIN, HIGH);
      delay(edgeDelayMs);
      digitalWrite(SIM_IRQ_PIN, LOW);
      delay(edgeDelayMs);
    }
  }
}

// ===== Reset the per-period counter =====
void resetSelfIrqCounter() {
  simIrqCount = 0;
}

// ===== Run all diagnostics once =====
void runAllDiagnosticsOnce() {
  Serial.println();
  Serial.println("===== Periodic Diagnostics Start =====");

  // I2C
  scanI2C();

  // SPI (loopback; requires MOSI<->MISO jumper)
  spiLoopbackTest();

  // System / memory / cores
  printSystemInfo();
  printMemoryInfo();
  checkCores();

  // Snapshot of input-only pins
  gpioReadInputs();

  // Self-interrupt test: generate a small, known pattern
  Serial.println();
  Serial.println("=== Self-Interrupt Test (software-generated edges) ===");
  resetSelfIrqCounter();
  simulateInterruptBursts(/*bursts=*/2, /*edgesPerBurst=*/5, /*edgeDelayMs=*/50);

  // Give ISRs/queue a moment and drain
  delay(50);
  processSelfIrqEvents();

  // Summary for this period
  Serial.println();
  Serial.println("=== Interrupt Summary (self-test, since last period) ===");
  Serial.printf("SIM_IRQ_PIN (GPIO %d): %u events\n", SIM_IRQ_PIN, (uint32_t)simIrqCount);
  resetSelfIrqCounter();

  Serial.println("===== Periodic Diagnostics End =====");
}

// ===== Arduino setup/loop =====
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Wire.begin(SDA_PIN, SCL_PIN, I2C_FREQ);
  delay(100);

  setupSelfInterrupt();     // configure self-contained interrupt test

  runAllDiagnosticsOnce();  // run immediately at boot
  lastTestMs = millis();
}

void loop() {
  // Continuously drain and print self-interrupt events (if any) in real time
  processSelfIrqEvents();

  // Run the whole suite every 3 minutes
  unsigned long now = millis();
  if ((now - lastTestMs) >= TEST_PERIOD_MS) {
    runAllDiagnosticsOnce();
    lastTestMs = now;
  }

  vTaskDelay(pdMS_TO_TICKS(10));
}
