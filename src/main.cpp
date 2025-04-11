#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RELAY_PIN  12 // GPIO pin connected to relay

// Setup ADC properties
#define ADC_PIN 34  // GPIO pin connected to the variable resistor 
int adcValue = 0;

// Setup PWM
const int pwmFreq = 1 / 30;  // 30-second cycle period
const int pwmResolution = 8; // 8-bit resolution (0-255)
const int pwmChannel = 0;

// Define the OLED reset pin (optional)
#define OLED_RESET    -1  // No reset pin used in I2C mode
#define SSD1309_I2C_ADDRESS 0x3C // I2C address for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long cycleStartTime = 0;
const unsigned long cycleDuration = 30000;  // 30 seconds cycle

void setup() {
  Serial.begin(115200);

  // Initialize ADC pin
  analogReadResolution(12); // 12-bit resolution (0-4095)
  
  // Setup PWM
  ledcSetup(pwmChannel, 1, pwmResolution); // 1Hz frequency
  ledcAttachPin(RELAY_PIN, pwmChannel);  // Attach PWM to the relay pin

  // Start with relay off
  ledcWrite(pwmChannel, 0);

  // Initialize the OLED display with I2C address (0x3C is the default I2C address)
  if (!display.begin(SSD1309_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("SSD1309 allocation failed"));
    for (;;);
  }

   // Clear the screen
   display.clearDisplay();

   // Set text size and color
   display.setTextSize(1);      
   display.setTextColor(SSD1306_WHITE);  
   display.setCursor(0, 0);   

}

void loop() {
  // Read the analog value from the variable resistor
  adcValue = analogRead(ADC_PIN);  // 0-4095 range

  // Convert ADC value to voltage (0 to 3.3V)
  float voltage = adcValue * (3.3 / 4095.0);
  
  // Map the ADC value (0-4095) to a duty cycle (0-255)
  int dutyCycle = map(adcValue, 0, 4095, 0, 255);
  
  // Output PWM signal to relay
  ledcWrite(pwmChannel, dutyCycle);

  // Get the time in the current 30-second cycle
  unsigned long cycleTime = millis() - cycleStartTime;
  if (cycleTime > cycleDuration) {
    cycleStartTime = millis();  // Reset the cycle start time after 30 seconds
  }

  // Determine if relay is on or off (based on duty cycle)
  bool relayState = (cycleTime < dutyCycle * (cycleDuration / 255));  // Scale duty cycle to 30 seconds

  // Debugging output
  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.print(" | Duty Cycle: ");
  Serial.print(dutyCycle);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 2);
  Serial.print(" V | Relay: ");
  Serial.println(relayState ? "ON" : "OFF");

  // Display the information on the OLED screen
  display.clearDisplay();
  
  // Display the voltage
  display.setCursor(0, 0);
  display.print("Voltage: ");
  display.print(voltage, 2);  // Display the voltage with 2 decimal places
  display.print(" V");

  // Display the duty cycle
  display.setCursor(0, 10);
  display.print("Duty Cycle: ");
  display.print(dutyCycle);
  display.print(" / 255");

  // Display the relay state
  display.setCursor(0, 20);
  display.print("Relay: ");
  display.print(relayState ? "ON" : "OFF");

  // Display the remaining time in the cycle (in seconds)
  int remainingTime = (cycleDuration - cycleTime) / 1000;
  display.setCursor(0, 30);
  display.print("Time Left: ");
  display.print(remainingTime);
  display.print(" s");

   // Update the display
   display.display();
  
  delay(100);  // Small delay to avoid flooding serial output
}
