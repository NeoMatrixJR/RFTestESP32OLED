#include <Arduino.h>
#include <WiFi.h>
#include <ESPNowW.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// OLED Display Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 16

TwoWire twi = TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);

// ESPNow Setup
#define PEER_ADDRESS "B0:A7:32:D7:98:48" // Replace with the MAC address of the non-OLED board

// Global variable to store the last send time
uint32_t last_send_time = 0;

void onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

void setup() {
  twi.begin(4,15);
  Serial.begin(115200);
  Serial.printf("Setting up Transciever\n");
  Serial.print("MAC: ");
  Serial.print(WiFi.macAddress());
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); //infinite loop
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  // Initialize ESPNow
  WiFi.mode(WIFI_STA); // MUST NOT BE WIFI_MODE_NULL
  WiFi.disconnect();
  ESPNow.init();
  ESPNow.add_peer((uint8_t *)PEER_ADDRESS);
  ESPNow.reg_recv_cb(onRecv);
  display.setCursor(0, 10);
  display.print("Hello! ");
  display.display();
}

void loop() {
  // Send packet every 5 seconds
  uint32_t now = millis(); // Update the time
  if (now - last_send_time >= 5000) {
    last_send_time = now;
    uint8_t packet[] = "Hello!";
    Serial.println("Sending data...");
    int status = ESPNow.send_message((uint8_t *)PEER_ADDRESS, packet, sizeof(packet));
    Serial.print("Send status: ");
    Serial.println(status);
    Serial.println("Data sent!");
  }
}

void onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
           mac_addr[5]);
  Serial.print("Last Packet Recv from: ");
  Serial.println(macStr);
  Serial.print("Last Packet Recv Data: ");
  // if it could be a string, print as one
  if (data[data_len - 1] == 0)
    Serial.printf("%s\n", data);
  // additionally print as hex
  for (int i = 0; i < data_len; i++) {
    Serial.printf("%x ", data[i]);
  }
  Serial.println("");
  
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("RTT: ");
  display.print(millis() - last_send_time);
  display.print(" ms");
  display.display();
}