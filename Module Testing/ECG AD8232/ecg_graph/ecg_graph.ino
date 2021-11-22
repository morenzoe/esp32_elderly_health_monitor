/*
 * Program Grafik Detak Jantung
 *
 * Koneksi Pin
 * ESP32 | AD8232
 * ---------------
 *  16   |  LO+
 *  17   |  LO-
 *  18   |  SDN
 *  33   | OUTPUT
 *  GND  |  GND
 *  3V3  |  VCC
 *  
 * Referensi
 * CaseyTheRobot, “Heart Rate Display Arduino.” Sparkfun, 
 * May 03, 2014. Accessed: Nov. 14, 2021. [Online]. 
 * Available: https://github.com/sparkfun/AD8232_Heart_Rate_Monitor/blob/master/Software/Heart_Rate_Display_Arduino/Heart_Rate_Display_Arduino.ino
 */

void setup() {
  // memulai komunikasi serial
  Serial.begin(9600);
  delay(200);
}

void loop() {
  // mendeteksi pemasangan elektroda
  if((digitalRead(16) == 1)||(digitalRead(17) == 1)){
    Serial.println('!');
  }
  else{
    // mengirimkan pengukuran ke serial monitor
      Serial.println(analogRead(33));
      
  }
  // menunggu selama 1 ms untuk mencegah saturasi data
  delay(1);
}
