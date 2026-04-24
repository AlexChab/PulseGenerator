#include <Arduino.h>

// Константы пинов
const int pwmPin = 18;  // Выход генератора
const int ledPin = 2;   // Встроенный светодиод (индикатор)

// Настройки ШИМ
const int ledcChannel = 0;
const int resolution = 8;  // 8 бит дают диапазон 0-255

// Текущие значения
uint32_t currentFreq = 5000;
int currentDutyPercent = 45;

void updatePWM(uint32_t f, int percent) {
  // Ограничиваем проценты для безопасности
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  // Переводим проценты (0-100) в значения для регистра (0-255)
  int dutyValue = map(percent, 0, 100, 0, 255);

  // Применяем настройки
  ledcSetup(ledcChannel, f, resolution);
  ledcWrite(ledcChannel, dutyValue);

  Serial.printf("Установлено: %u Гц | Заполнение: %d%%\n", f, percent);
}

void setup() {
  Serial.begin(115200);

  // Привязываем пины к каналу ШИМ
  ledcAttachPin(pwmPin, ledcChannel);
  ledcAttachPin(ledPin, ledcChannel);

  updatePWM(currentFreq, currentDutyPercent);

  Serial.println("---------------------------------------");
  Serial.println("Генератор готов к работе.");
  Serial.println("Формат ввода: ЧАСТОТА ПРОЦЕНТ");
  Serial.println("Пример: 100000 25 (100кГц, 25% заполнения)");
  Serial.println("---------------------------------------");
}

void loop() {
  if (Serial.available() > 0) {
    // Читаем всю строку до конца
    String input = Serial.readStringUntil('\n');
    input.trim();

    uint32_t f;
    int p;

    // Разбираем строку: ищем два целых числа
    if (sscanf(input.c_str(), "%u %d", &f, &p) == 2) {
      // Проверка диапазонов
      if (f >= 5000 && f <= 500000 && p >= 0 && p <= 100) {
        currentFreq = f;
        currentDutyPercent = p;
        updatePWM(currentFreq, currentDutyPercent);
      } else {
        Serial.println("ОШИБКА: Частота 5000-500000 Гц, Проценты 0-100%");
      }
    }
  }
}
