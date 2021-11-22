/*
 * Program Penghitung Detak Jantung
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
 * Variabel
 * count: jumlah puncak R selama 10 detik
 * onRPeak: mencegah duplikasi deteksi puncak R dalam satu detak jantung
 * hr: detak jantung (diinisialisasi pada 72)
 * hrv: variabilitas detak jantung
 * rPeakTime: waktu ketika terjadi puncak R
 * rrInterval: durasi antara puncak R kedua dengan puncak R pertama
 * hrDuration: durasi perhitungan detak jantung
 * value: nilai dari pin OUTPUT
 * 
 * Referensi
 * D. Shah, “Understating ECG Sensors and How to Program one to Diagnose 
 * Various Medical Conditions,” Jun. 01, 2021. 
 * https://circuitdigest.com/microcontroller-projects/understanding-ecg-sensor-and-program-ad8232-ecg-sensor-with-arduino-to-diagnose-various-medical-conditions (accessed Nov. 04, 2021).
 */

long rPeakTime=0, hrDuration;
double hrv =0, hr = 72, rrInterval = 0;
int value = 0, count = 0;  
bool onRPeak = 0;
#define threshold 50 // ambang batas nilai puncak R
#define timer_value 10000 // durasi perhitungan detak jantung selama 10 detik

void setup() {
  // memulai komunikasi serial
  Serial.begin(9600);
}

void loop() {
  // mendeteksi pemasangan elektroda 
  if((digitalRead(16) == 1)||(digitalRead(17) == 1)){
    // mode standby
    digitalWrite(18, LOW);
    
    // menginisiasi nilai waktu
    rPeakTime = micros();
    hrDuration = millis();
  }
  else {
    // mode normal
    digitalWrite(18, HIGH); 
    
    // pembacaan nilai pin OUTPUT
    value = analogRead(33);
    
    // skala penggambaran grafik
    value = map(value, 1600, 3200, 0, 100); 
    
    // deteksi puncak R
    if((value > threshold) && (!onRPeak)) {
      count++;  
      Serial.println("in");
      onRPeak = 1;
      rrInterval = micros() - rPeakTime;
      rPeakTime = micros(); 
    }
    else if((value < threshold)) {
      onRPeak = 0;
    }
    
    // perhitungan nilai detak jantung
    if ((millis() - hrDuration) > 10000) {
      hr = count*6;
      hrDuration = millis();
      count = 0; 
    }
    
    // perhitungan variabilitas detak jantung
    hrv = hr/60 - rrInterval/1000000;

    // pelaporan nilai
    Serial.print(hr);
    Serial.print(",");
    Serial.print(hrv);
    Serial.print(",");
    Serial.println(value);
    
    // menunggu selama 1 ms untuk mencegah saturasi data
    delay(1);
  }
}
