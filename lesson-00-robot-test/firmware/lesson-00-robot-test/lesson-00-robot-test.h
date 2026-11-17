/*
lesson-00-robot-test.h — реализация тестов робота: Iskra Uno + Motor Shield + Troyka Shield.
Лежит рядом с lesson-00-robot-test.ino и открывается в Arduino IDE соседней вкладкой.
Вывод в порт — короткий: L, R — левый и правый, Hz — частота тиков, mm — путь, cm — расстояние.
...Print — только печатают, ...Test — двигают робота или серво.
F("...") хранит строку во flash, а не в оперативной памяти.
*/

#pragma once
#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

#define L_DIR 7  // левый мотор — канал M2
#define L_PWM 6
#define R_DIR 4  // правый мотор — канал M1
#define R_PWM 5

#define ENC_L_A 2  // левый энкодер: C1 — пин с прерыванием, C2 — направление
#define ENC_L_B 8
#define ENC_R_A 3  // правый энкодер
#define ENC_R_B 9
#define ENC_PER_REV 350  // тиков на оборот колеса: 7 на оборот вала мотора × редуктор 1:50
#define WHEEL_D 43       // диаметр колеса, мм

#define LINE_L A0
#define LINE_R A1
#define SHARP A2
#define HC_TRIG 12
#define HC_ECHO 13
#define SRV_1 10
#define SRV_2 11

Servo srv1, srv2;
volatile int32_t encL = 0, encR = 0;  // тики со знаком (назад — минус), меняются в прерываниях

// ---------- моторы: как в уроке 2; моторы стоят зеркально — инвертирован левый ----------
bool direction(int16_t speed, bool invDir) {
  bool dir = (speed > 0) ? true : false;
  return (invDir) ? !dir : dir;
}

void motors(int16_t left, int16_t right, bool invLDir = true, bool invRDir = false) {
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);
  digitalWrite(L_DIR, direction(left, invLDir));
  digitalWrite(R_DIR, direction(right, invRDir));
  analogWrite(L_PWM, abs(left));
  analogWrite(R_PWM, abs(right));
}

// ---------- энкодеры: фронт на C1, направление по C2; левый мотор стоит зеркально ----------
void encLeft() {
  encL += digitalRead(ENC_L_B) ? -1 : 1;
}
void encRight() {
  encR += digitalRead(ENC_R_B) ? 1 : -1;
}

int32_t ticks(volatile int32_t &enc) {  // прочитать счетчик, пока прерывание его не меняет
  noInterrupts();
  int32_t t = enc;
  interrupts();
  return t;
}

float mm(int32_t t) {
  return t * PI * WHEEL_D / ENC_PER_REV;
}

// ---------- тесты для loop(), печатают одной строкой ----------
void encPrint() {  // L тики Hz mm   R тики Hz mm; частота — за время с прошлого вызова
  static int32_t oldL = 0, oldR = 0;
  static uint32_t oldTime = 0;
  int32_t l = ticks(encL), r = ticks(encR);
  uint32_t dt = millis() - oldTime;
  if (dt == 0) dt = 1;
  Serial.print(F("L "));
  Serial.print(l);
  Serial.print(' ');
  Serial.print((l - oldL) * 1000 / (int32_t)dt);
  Serial.print(F("Hz "));
  Serial.print(mm(l), 0);
  Serial.print(F("mm  "));
  Serial.print(F("R "));
  Serial.print(r);
  Serial.print(' ');
  Serial.print((r - oldR) * 1000 / (int32_t)dt);
  Serial.print(F("Hz "));
  Serial.print(mm(r), 0);
  Serial.print(F("mm  "));
  oldL = l;
  oldR = r;
  oldTime += dt;
}

// Живы ли пины энкодеров: вызывать в loop() на каждом проходе, без таймера, остальное закомментировать.
// Крутить колесо рукой. Печать раз в 200 мс: D2 1/57 — на пине сейчас 1, уровень менялся 57 раз.
// У живого энкодера растут оба его пина.
void encPinsPrint() {
  const uint8_t pins[4] = { ENC_L_A, ENC_L_B, ENC_R_A, ENC_R_B };
  static uint8_t old[4];
  static uint32_t changes[4];
  static uint32_t last = 0;
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t v = digitalRead(pins[i]);
    if (v != old[i]) {
      changes[i]++;
      old[i] = v;
    }
  }
  if (millis() - last >= 200) {
    last = millis();
    for (uint8_t i = 0; i < 4; i++) {
      Serial.print('D');
      Serial.print(pins[i]);
      Serial.print(' ');
      Serial.print(old[i]);
      Serial.print('/');
      Serial.print(changes[i]);
      Serial.print(F("  "));
    }
    Serial.println();
  }
}

void sensors() {  // как в уроке 2: L, R, err = L − R
  int16_t l = analogRead(LINE_L), r = analogRead(LINE_R);
  Serial.print(F("L "));
  Serial.print(l);
  Serial.print(F(" R "));
  Serial.print(r);
  Serial.print(F(" err "));
  Serial.print(l - r);
  Serial.print(F("  "));
}

void sharpPrint() {  // значение АЦП
  Serial.print(F("Sharp "));
  Serial.print(analogRead(SHARP));
  Serial.print(F("  "));
}

void hcPrint() {  // HC-SR04, cm; до 30 мс ждем эхо, нет эха — «---»
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);
  uint32_t us = pulseIn(HC_ECHO, HIGH, 30000);
  Serial.print(F("HC "));
  if (us == 0) Serial.print(F("---"));
  else {
    Serial.print(us / 58.0, 1);
    Serial.print(F("cm"));
  }
  Serial.print(F("  "));
}

void servoTest() {  // оба серво 0 → 180 → 0, шаг 10 градусов за вызов
  static int16_t angle = 0, step = 10;
  angle += step;
  if (angle == 0 || angle == 180) step = -step;
  srv1.write(angle);
  srv2.write(angle);
  Serial.print(F("Srv "));
  Serial.print(angle);
  Serial.print(F("  "));
}

/*
Проверка моторов и энкодеров на месте: робот крутится вокруг оси — левое колесо вперед, правое назад.
ШИМ плавно 0 → 255 → 0, шаг 1 раз в stepMs (8 мс — около 2 с на разгон и 2 с на торможение), потом стоп.
Неблокирующая: вызывать в loop() на каждом проходе, без таймера; темп задает свой таймер.
Возвращает true, когда цикл закончен. Энкодеры печатает encPrint() — у правого тики уменьшаются.
*/
bool motorsTest(uint16_t stepMs = 8) {
  static int16_t pwm = 0, step = 1;
  static bool done = false;
  static uint32_t last = 0;
  if (done) return true;
  if (millis() - last < stepMs) return false;
  last = millis();
  pwm += step;
  if (pwm >= 255) step = -1;
  if (pwm <= 0) done = true;
  motors(pwm, -pwm);
  return done;
}

// ---------- тесты для setup(), блокирующие ----------
void wheelsTest() {  // L+ L- R+ R- LR+ по 1 с: «+» — колесо должно катить робота вперед
  Serial.println(F("L+"));
  motors(150, 0);
  delay(1000);
  Serial.println(F("L-"));
  motors(-150, 0);
  delay(1000);
  Serial.println(F("R+"));
  motors(0, 150);
  delay(1000);
  Serial.println(F("R-"));
  motors(0, -150);
  delay(1000);
  Serial.println(F("LR+"));
  motors(150, 150);
  delay(1000);
  motors(0, 0);
}

// Обертка для setup(): крутит motorsTest() до конца цикла, раз в 100 мс печатает encPrint()
void motorsTestSetup() {
  uint32_t lastPrint = 0;
  while (!motorsTest()) {
    if (millis() - lastPrint >= 100) {
      lastPrint = millis();
      encPrint();
      Serial.println();
    }
  }
}

/*
Заезд по прямой на полу без кабеля: motors(speed, speed) на ms миллисекунд, потом motors(0, 0) — выбег.
Нужно около метра свободного места; робот с разными моторами уедет дугой — замеру это не мешает.
Результат пишется в EEPROM. При старте функция сначала печатает прошлый заезд:
подключил USB — плата перезагрузилась — отчет в порту, потом через 2 с новый заезд.
Отчет:
  v      — скорость колес на ровном ходу (после разгона 400 мс), mm/s
  coast  — выбег после motors(0, 0): путь колес, mm, и время до остановки, ms
  rev    — обороты колес за время с включенными моторами и разница d, % (насколько моторы разные)
  pwm    — ШИМ, при котором колеса шли бы одинаково: быстрому убавить (примерно — ШИМ и скорость не пропорциональны)
*/

#define ACCEL_MS 400      // разгон — на нем скорость не меряем
#define DRIVE_ADDR 0      // адрес результата в EEPROM
#define DRIVE_KEY 0x5244  // признак, что в EEPROM лежит наш заезд

struct DriveResult {
  uint16_t key;
  int16_t speed;
  uint16_t ms, coastMs;
  float vL, vR, coastL, coastR, revL, revR;
};

void drivePrint(const DriveResult &d) {
  Serial.print(F("speed "));
  Serial.print(d.speed);
  Serial.print(' ');
  Serial.print(d.ms);
  Serial.println(F("ms"));
  Serial.print(F("v L "));
  Serial.print(d.vL, 0);
  Serial.print(F(" R "));
  Serial.print(d.vR, 0);
  Serial.println(F(" mm/s"));
  Serial.print(F("coast L "));
  Serial.print(d.coastL, 0);
  Serial.print(F(" R "));
  Serial.print(d.coastR, 0);
  Serial.print(F(" mm "));
  Serial.print(d.coastMs);
  Serial.println(F("ms"));
  float d100 = (d.revL - d.revR) / ((d.revL + d.revR) / 2) * 100;
  Serial.print(F("rev L "));
  Serial.print(d.revL, 2);
  Serial.print(F(" R "));
  Serial.print(d.revR, 2);
  Serial.print(F(" d "));
  Serial.print(d100, 1);
  Serial.println('%');
  int16_t pwmL = d.speed, pwmR = d.speed;  // быстрому мотору убавляем ШИМ в отношении скоростей
  if (d.vL > d.vR) pwmL = d.speed * d.vR / d.vL;
  else pwmR = d.speed * d.vL / d.vR;
  Serial.print(F("pwm L "));
  Serial.print(pwmL);
  Serial.print(F(" R "));
  Serial.println(pwmR);
}

void driveTest(uint16_t ms = 2000, int16_t speed = 150) {
  DriveResult d;
  EEPROM.get(DRIVE_ADDR, d);
  if (d.key == DRIVE_KEY) drivePrint(d);
  else Serial.println(F("EEPROM empty"));

  delay(2000);  // поставить робота и убрать руки
  int32_t l0 = ticks(encL), r0 = ticks(encR);
  uint32_t t0 = millis();
  motors(speed, speed);
  delay(min(ACCEL_MS, ms / 2));
  int32_t l1 = ticks(encL), r1 = ticks(encR);
  uint32_t t1 = millis();
  delay(ms - (t1 - t0));
  int32_t l2 = ticks(encL), r2 = ticks(encR);
  uint32_t t2 = millis();
  motors(0, 0);

  // выбег: ждем, пока тики не перестанут меняться 200 мс
  int32_t l3 = l2, r3 = r2;
  uint32_t tStop = t2;
  while (millis() - tStop < 200) {
    int32_t l = ticks(encL), r = ticks(encR);
    if (l != l3 || r != r3) {
      l3 = l;
      r3 = r;
      tStop = millis();
    }
    if (millis() - t2 > 3000) {  // колеса не встали — EEPROM не трогаем
      Serial.println(F("no stop"));
      return;
    }
  }

  d.key = DRIVE_KEY;
  d.speed = speed;
  d.ms = ms;
  d.vL = mm(l2 - l1) * 1000 / (t2 - t1);
  d.vR = mm(r2 - r1) * 1000 / (t2 - t1);
  d.coastL = mm(l3 - l2);
  d.coastR = mm(r3 - r2);
  d.coastMs = tStop - t2;
  d.revL = (float)(l2 - l0) / ENC_PER_REV;
  d.revR = (float)(r2 - r0) / ENC_PER_REV;
  EEPROM.put(DRIVE_ADDR, d);
  drivePrint(d);
}

// все подключение — вызвать первой строкой в setup()
void robotInit() {
  Serial.begin(115200);
  pinMode(L_DIR, OUTPUT);
  pinMode(L_PWM, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  pinMode(R_PWM, OUTPUT);
  pinMode(ENC_L_A, INPUT_PULLUP);
  pinMode(ENC_L_B, INPUT_PULLUP);
  pinMode(ENC_R_A, INPUT_PULLUP);
  pinMode(ENC_R_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), encLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), encRight, RISING);
  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
  srv1.attach(SRV_1);  // Servo занимает Timer1: нет ШИМ на D9, D10 (моторам на D5, D6 не мешает)
  srv2.attach(SRV_2);
}
