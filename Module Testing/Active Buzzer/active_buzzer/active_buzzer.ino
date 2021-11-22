/*
 * Program Blink Active Buzzer
 *
 * Koneksi Pin
 * ESP32 | Buzzer
 * ---------------
 *  13   |  VCC
 *  GND  |  GND
 *  
 * Referensi
 * C. Newman, “Blink.” Sep. 08, 2016. 
 * Accessed: Nov. 04, 2021. [Online]. 
 * Available: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
 */

void setup() {
  // memulai komunikasi serial
  Serial.begin(9600);
  // mendeklarasikan mode pin
  pinMode(13,OUTPUT);
}

void loop() {
  // menyalakan active buzzer selama 1 detik
  digitalWrite(13,HIGH);
  Serial.println("Buzzer Menyala");
  delay(1000);
  // mematikan active buzzer selama 1 detik
  digitalWrite(13,LOW);
  Serial.println("Buzzer Mati");
  delay(1000);
}
