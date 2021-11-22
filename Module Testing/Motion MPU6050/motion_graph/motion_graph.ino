/*
 * Program Grafik Pergerakan
 *
 * Koneksi Pin
 * ESP32 | MPU6050
 * ---------------
 *  22   |  SCL
 *  21   |  SDA
 *  GND  |  GND
 *  3V3  |  VCC
 *  
 *  Referensi
 *  siddacious, “plotter.” Adafruit, Oct. 16, 2019. 
 *  Accessed: Nov. 11, 2021. [Online]. 
 *  Available: https://github.com/adafruit/Adafruit_MPU6050/blob/master/examples/plotter/plotter.ino
 */

// import library
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// deklarasi objek modul sensor
Adafruit_MPU6050 mpu;

void setup(void) {
  // memulai komunikasi serial
  Serial.begin(115200);
  
  // menginisialisasi modul sensor
  if (!mpu.begin()) {
    Serial.println("Gagal mendeteksi MPU6050");
    while (1) {
      delay(10);
    }
  }

  // mengatur parameter sensor
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
}

void loop() {
  // membaca pengukuran
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // melaporkan pengukuran
  Serial.print(a.acceleration.x);
  Serial.print(",");
  Serial.print(a.acceleration.y);
  Serial.print(",");
  Serial.print(a.acceleration.z);
  Serial.println("");

  // menunggu selama 10 ms untuk mencegah saturasi data
  delay(10);
}
