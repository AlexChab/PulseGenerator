#include <Arduino.h>
#include "driver/rmt.h"

const int OUTPUT_PIN = 18;
const rmt_channel_t RMT_CHAN = RMT_CHANNEL_0;

// Параметры по умолчанию
uint32_t pulseUs = 500;
uint32_t pauseMs = 100;

void setup_rmt() {
    rmt_config_t config;
    config.rmt_mode = RMT_MODE_TX;
    config.channel = RMT_CHAN;
    config.gpio_num = (gpio_num_t)OUTPUT_PIN;
    config.mem_block_num = 1;
    config.clk_div = 1; // 80МГц / 80 = 1 тик = 1 мкс
    config.tx_config.loop_en = false;
    config.tx_config.carrier_en = false;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH; // Инвертированный сигнал (пауза = 1)
    config.tx_config.idle_output_en = true;

    rmt_config(&config);
    rmt_driver_install(RMT_CHAN, 0, 0);
}

void send_pulse() {
    // Формируем структуру импульса (длительность в тиках, уровень)
    // Уровень 0 (инверсия), потом уровень 1
    rmt_item32_t item = { 
        (uint16_t)pulseUs, 0,                      // Длительность импульса (0)
        (uint16_t)(pauseMs * 1000 & 0x7FFF), 1     // Длительность паузы (1)
    };
    
    rmt_write_items(RMT_CHAN, &item, 1, false);
}

void setup() {
    Serial.begin(115200);
    setup_rmt();
    Serial.println("RMT Ready. Send: PULSE <us> <ms>");
}

void loop() {
    // Каждые pauseMs отправляем импульс
    static uint32_t lastMillis = 0;
    if (millis() - lastMillis >= pauseMs) {
        lastMillis = millis();
        send_pulse();
    }

    // Слушаем порт без блокировки прерываниями
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        char mode[10];
        uint32_t pW, pP;

        if (sscanf(input.c_str(), "%s %u %u", mode, &pW, &pP) == 3) {
            pulseUs = constrain(pW, 1, 32767); // RMT item предел 32767 тиков
            pauseMs = constrain(pP, 10, 2000);
            Serial.printf("Updated: Pulse=%u, Pause=%u\n", pulseUs, pauseMs);
        }
    }
}
