/*
 * Program Deteksi Pergerakan
 * 
 * Koneksi Pin
 * ESP32 | MPU6050
 * ---------------
 *  22   |  SCL
 *  21   |  SDA
 *  GND  |  GND
 *  3V3  |  VCC
 *  
 * Referensi
 * K. Jarzębski, “MPU6050_motion.” Oct. 21, 2014. 
 * Accessed: Nov. 17, 2021. [Online]. 
 * Available: https://github.com/jarzebski/Arduino-MPU6050/blob/master/MPU6050_motion/MPU6050_motion.ino
 */

// import library
#include <Wire.h>
#include <MPU6050.h>

// deklarasi objek modul sensor
MPU6050 mpu;

void setup() 
{
  // memulai komunikasi serial
  Serial.begin(115200);

  // menginisiasi modul sensor
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
  {
    Serial.println("Gagal mendeteksi MPU6050");
    while (1) {
      delay(10);
    }
  }

  // mengatur parameter sensor
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);

  // mengatur nilai ambang batas deteksi
  mpu.setMotionDetectionThreshold(2);
  mpu.setMotionDetectionDuration(5);

  // mengatur lampu LED_BUILTIN ESP32
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); // LED mati di awal
}

void loop()
{
  // membaca pengukuran
  Activites act = mpu.readActivites();

  if (act.isActivity)
  { // menyalakan LED jika terdeteksi pergerakan
    digitalWrite(2, HIGH);
    Serial.println("Bergerak");
  } else
  { // mematikan LED jika tidak terdeteksi pergerakan
    digitalWrite(2, LOW);
    Serial.println("Tidak Bergerak");
  }

  // menunggu selama 50 ms untuk mencegah saturasi data
  delay(50);
}
