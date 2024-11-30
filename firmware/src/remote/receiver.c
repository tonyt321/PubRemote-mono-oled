#include "receiver.h"
#include "connection.h"
#include "display.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "peers.h"
#include "powermanagement.h"
#include "stats.h"
#include "time.h"
#include "utilities/conversion_utils.h"
#include <freertos/queue.h>
#include <math.h>
#include <remote/settings.h>
#include <stdlib.h>
#include <string.h>
#include <ui/ui.h>

static const char *TAG = "PUBREMOTE-RECEIVER";
#define RX_QUEUE_SIZE 1

// Structure to hold ESP-NOW data
typedef struct {
  uint8_t mac_addr[ESP_NOW_ETH_ALEN];
  uint8_t *data;
  int len;
} esp_now_event_t;

static QueueHandle_t espnow_queue;

static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  // This callback runs in WiFi task context!
  ESP_LOGD(TAG, "RECEIVED");
  esp_now_event_t evt;
  memcpy(evt.mac_addr, recv_info->src_addr, ESP_NOW_ETH_ALEN);
  evt.data = malloc(len);
  memcpy(evt.data, data, len);
  evt.len = len;

#if RX_QUEUE_SIZE > 1
  // Send to queue for processing in application task
  if (uxQueueSpacesAvailable() == 0) {
    // reset the queue
    xQueueReset(espnow_queue);
  }
  if (xQueueSend(espnow_queue, &evt, portMAX_DELAY) != pdTRUE) {
#else
  // overwrite the previous data
  if (xQueueOverwrite(espnow_queue, &evt) != pdTRUE) {
#endif
    ESP_LOGE(TAG, "Queue send failed");
    free(evt.data);
  }
}

static void process_data(esp_now_event_t evt) {
  uint8_t *data = evt.data;
  int len = evt.len;
  int64_t deltaTime = get_current_time_ms() - LAST_COMMAND_TIME;
  LAST_COMMAND_TIME = 0;
  ESP_LOGD(TAG, "RTT: %lld", deltaTime);

  if (pairing_state == PAIRING_STATE_UNPAIRED && len == 6) {
    memcpy(pairing_settings.remote_addr, data, 6);
    ESP_LOGI(TAG, "Got Pairing request from VESC Express");
    ESP_LOGI(TAG, "packet Length: %d", len);
    ESP_LOGI(TAG, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", data[0], data[1], data[2], data[3], data[4], data[5]);
    // ESP_LOGI(TAG, "Incorrect MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2],
    //          mac_addr[3], mac_addr[4], mac_addr[5]);
    uint8_t TEST[1] = {420}; // TODO - FIX THIS
    esp_now_peer_info_t peerInfo = {};
    peerInfo.channel = 1; // Set the channel number (0-14)
    peerInfo.encrypt = false;
    memcpy(peerInfo.peer_addr, pairing_settings.remote_addr, sizeof(pairing_settings.remote_addr));
    // ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));
    uint8_t *mac_addr = pairing_settings.remote_addr;
    if (!esp_now_is_peer_exist(mac_addr)) {
      esp_now_add_peer(&peerInfo);
    }

    esp_err_t result = esp_now_send(mac_addr, (uint8_t *)&TEST, sizeof(TEST));
    if (result != ESP_OK) {
      // Handle error if needed
      ESP_LOGE(TAG, "Error sending data: %d", result);
    }
    else {
      ESP_LOGI(TAG, "Sent response back to VESC Express");
      pairing_state = PAIRING_STATE_PAIRING;
    }
  }
  else if (pairing_state == PAIRING_STATE_PAIRING && len == 4) {
    // grab secret code
    ESP_LOGI(TAG, "Grabbing secret code");
    ESP_LOGI(TAG, "packet Length: %d", len);
    pairing_settings.secret_code = (int32_t)(data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    ESP_LOGI(TAG, "Secret Code: %li", pairing_settings.secret_code);
    char *formattedString;
    asprintf(&formattedString, "%ld", pairing_settings.secret_code);
    LVGL_lock(-1);
    lv_label_set_text(ui_PairingCode, formattedString);
    LVGL_unlock();
    free(formattedString);
    pairing_state = PAIRING_STATE_PENDING;
  }
  else if (pairing_state == PAIRING_STATE_PENDING && len == 4) {
    // grab secret code
    ESP_LOGI(TAG, "Grabbing response");
    ESP_LOGI(TAG, "packet Length: %d", len);
    int response = (int32_t)(data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    ESP_LOGI(TAG, "Response: %i", response);
    LVGL_lock(-1);
    if (response == -1) {
      pairing_state = PAIRING_STATE_PAIRED;
      save_pairing_data();
      connect_to_default_peer();
      lv_disp_load_scr(ui_StatsScreen);
    }
    else {
      pairing_state = PAIRING_STATE_UNPAIRED;
    }
    LVGL_unlock();
  }
  else if ((connection_state == CONNECTION_STATE_CONNECTED || connection_state == CONNECTION_STATE_RECONNECTING ||
            connection_state == CONNECTION_STATE_CONNECTING) &&
           len == 32) {
    start_or_reset_deep_sleep_timer();

    uint8_t mode = data[0];
    uint8_t fault_code = data[1];
    float pitch_angle = (int16_t)((data[2] << 8) | data[3]) / 10.0;
    float roll_angle = (int16_t)((data[4] << 8) | data[5]) / 10.0;
    uint8_t state = data[6];
    uint8_t switch_state = data[7];
    remoteStats.switchState = switch_state;
    float input_voltage_filtered = (int16_t)((data[8] << 8) | data[9]) / 10.0;
    int16_t rpm = (int16_t)((data[10] << 8) | data[11]);
    float speed = (int16_t)((data[12] << 8) | data[13]) / 10.0;
    remoteStats.speed = convert_ms_to_kph(fabs(speed)); // Store in kph
    float tot_current = (int16_t)((data[14] << 8) | data[15]) / 10.0;
    float duty_cycle_now = (float)data[16] / 100.0 - 0.5;
    remoteStats.dutyCycle = (uint8_t)(fabs(duty_cycle_now) * 100);
    float distance_abs;
    memcpy(&distance_abs, &data[17], sizeof(float));
    float fet_temp_filtered = (float)data[21] / 2.0;
    float motor_temp_filtered = (float)data[22] / 2.0;
    uint32_t odometer = (uint32_t)((data[23] << 24) | (data[24] << 16) | (data[25] << 8) | data[26]);
    float battery_level = (float)data[27] / 2.0;
    remoteStats.batteryPercentage = battery_level;
    int32_t super_secret_code = (int32_t)((data[28] << 24) | (data[29] << 16) | (data[30] << 8) | data[31]);

    remoteStats.lastUpdated = get_current_time_ms();
    // Print the extracted values
    // ESP_LOGI(TAG, "Mode: %d", mode);
    // ESP_LOGI(TAG, "Fault Code: %d", fault_code);
    // ESP_LOGI(TAG, "Pitch Angle: %.1f", pitch_angle);
    // ESP_LOGI(TAG, "Roll Angle: %.1f", roll_angle);
    // ESP_LOGI(TAG, "State: %d", state);
    // ESP_LOGI(TAG, "Switch State: %d", switch_state);
    // ESP_LOGI(TAG, "Input Voltage Filtered: %.1f", input_voltage_filtered);
    // ESP_LOGI(TAG, "RPM: %d", rpm);
    // ESP_LOGI(TAG, "Speed: %.1f", speed);
    // ESP_LOGI(TAG, "Total Current: %.1f", tot_current);
    // ESP_LOGI(TAG, "Duty Cycle Now: %.2f", duty_cycle_now);
    // ESP_LOGI(TAG, "Distance Absolute: %.2f", distance_abs);
    // ESP_LOGI(TAG, "FET Temperature Filtered: %.1f", fet_temp_filtered);
    // ESP_LOGI(TAG, "Motor Temperature Filtered: %.1f", motor_temp_filtered);
    // ESP_LOGI(TAG, "Odometer: %lu", odometer);
    // ESP_LOGI(TAG, "Battery Level: %.1f", battery_level);
    update_stats_display(); // TODO - use callbacks to update the UI instead of direct calls
  }
  else {
    ESP_LOGI(TAG, "Invalid data length %d", len);
  }
}

static void receiver_task(void *pvParameters) {
  espnow_queue = xQueueCreate(RX_QUEUE_SIZE, sizeof(esp_now_event_t));
  ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));
  ESP_LOGI(TAG, "Registered RX callback");
  esp_now_event_t evt;
  while (1) {
    if (xQueueReceive(espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
      process_data(evt);
      free(evt.data);
    }
  }

  // The task will not reach this point as it runs indefinitely
  ESP_LOGI(TAG, "RX task ended");
  vTaskDelete(NULL);
}

void init_receiver() {
  ESP_LOGI(TAG, "Starting receiver task");
  xTaskCreatePinnedToCore(receiver_task, "receiver_task", 4096, NULL, 20, NULL, 0);
}