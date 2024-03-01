#include "Transmitter.h"
#include "Peers.h"
#include "Time.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include <stdbool.h>
#include <stdio.h>

#define TRANSMITTER_TAG "PUBMOTE_TRANSMITTER"

// Function to send ESP-NOW data
void send_data_task(void *pvParameters) {
  ESP_LOGI(TRANSMITTER_TAG, "TX task started");
  u_int8_t my_data[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}; // Example data
#define TEST_TIME 50000
  static LatencyTestResults results;
  results.size = 0;
  results.results = malloc(TEST_TIME / TRANSMIT_TIME * sizeof(uint16_t));

  for (int i = 0; i < TEST_TIME; i += TRANSMIT_TIME) {
    int64_t newTime = get_current_time_ms();

// Check if the last command was sent less than 1000ms ago
#define COMMAND_TIMEOUT 1000
    if (newTime - LAST_COMMAND_TIME < COMMAND_TIMEOUT) {
      vTaskDelay(TRANSMIT_FREQUENCY);
      continue;
    }

    esp_err_t result = esp_now_send(PEER_MAC_ADDRESS, (uint8_t *)&my_data, sizeof(my_data));

    if (result != ESP_OK) {
      // Handle error if needed
      ESP_LOGE(TRANSMITTER_TAG, "Error sending data: %d", result);
    }

    LAST_COMMAND_TIME = newTime;
    ESP_LOGI(TRANSMITTER_TAG, "Sent command");
    vTaskDelay(TRANSMIT_FREQUENCY);
  }

  ESP_LOGI(TRANSMITTER_TAG, "TX task ended");
  // terminate self
  vTaskDelete(NULL);
}

void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  ESP_LOGI(TRANSMITTER_TAG, "SENT");
}