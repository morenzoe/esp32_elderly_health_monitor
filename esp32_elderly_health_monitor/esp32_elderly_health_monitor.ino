/*
 * Program Monitor Kesehatan Orang Tua
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
 * ESP32 | MPU6050
 * ---------------
 *  22   |  SCL
 *  21   |  SDA
 *  GND  |  GND
 *  3V3  |  VCC
 *
 * ESP32 | Buzzer
 * ---------------
 *  13   |  VCC
 *  GND  |  GND
 *
 * ESP32 | Button
 * ---------------
 *  14   |  VCC
 *  GND  |  GND
 *
 * Variabel dan Konstanta
 * MPU6050
 * gerakTerakhir: waktu terkahir terdeteksi pergerakan
 * terjatuh: kondisi terjatuh
 * waktuJatuh: waktu ketika terdeteksi terjatuh
 * 
 * AD8232
 * M: panjang buffer ECG
 * N: panjang buffer filter high pass
 * winSize: panjang jendela sampel yang akan dideteksi
 * HP_CONSTANT: koefisien filter high pass
 * RAND_RES: resolusi random number generator
 * foundTimeMicros: waktu ketika kompleks QRS ditemukan
 * old_foundTimeMicros: waktu ketika kompleks QRS sebelumnya ditemukan
 * bpm: nilai detak jantung pengguna
 * BPM_BUFFER_SIZE: panjang buffer detak jantung
 * bpm_buff: buffer detak jantung
 * bpm_buff_WR_idx: indeks penulisan nilai buffer detak jantung
 * bpm_buff_RD_idx: indeks pembacaan nilai buffer detak jantung
 * tmp: variabel penyimpan indeks sementara
 * ecg_buff: buffer ECG
 * ecg_buff_WR_idx: indeks penulisan nilai buffer ECG
 * ecg_buff_RD_idx: indeks pembacaan nilai buffer ECG
 * hp_buff: buffer filter high pass
 * hp_buff_WR_idx: indeks penulisan nilai buffer hasil filter high pass
 * hp_buff_RD_idx: indeks pembacaan nilai buffer hasil filter high pass
 * next_eval_pt: hasil filter low pass
 * hp_sum: jumlah buffer ECG untuk filter high pass
 * lp_sum: jumlah buffer filter high pass untuk filter low pass
 * treshold: nilai ambang batas deteksi kompleks QRS
 * triggered: kondisi tunggu pendeteksian kompleks QRS
 * trig_time: durasi tunggu pendeteksian kompleks QRS
 * win_max: nilai maksimum dari suatu jendela
 * win_idx: indeks sampel dalam jendela
 * number_iter: jumlah iterasi pengisian awal buffer
 * QRS_detected: kondisi pendeteksian kompleks QRS
 * leads_are_on: kondisi pemasangan elektroda
 * next_ecg_pt: hasil pengukuran modul AD8232
 * y1, y2: variabel perhitungan filter high pass
 * gamma: bobot sinyal tertinggi dalam perhitungan ambang batas
 * alpha: rasio kontribusi nilai ambang batas sebelumnya
 *
 * Email
 * WIFI_SSID: nama WiFi
 * WIFI_PASSWORD: password WiFi
 * SMTP_HOST: host server email
 * SMTP_PORT: port server email
 * AUTHOR_EMAIL: alamat email pengirim
 * AUTHOR_PASSWORD: password email pengirim
 * RECIPIENT_EMAIL: alamat email penerima
 * terkirim: kondisi pengiriman email
 * 
 * Referensi
 * B. Milner, “QRS.” Dec. 24, 2015. 
 * Accessed: Nov. 14, 2021. [Online]. 
 * Available: https://github.com/blakeMilner/real_time_QRS_detection/blob/master/QRS_arduino/QRS.ino
 *
 * K. Jarzębski, “MPU6050_free_fall.” Oct. 21, 2014. 
 * Accessed: Nov. 17, 2021. [Online]. 
 * Available: https://github.com/jarzebski/Arduino-MPU6050/blob/master/MPU6050_free_fall/MPU6050_free_fall.ino
 *
 * K. Jarzębski, “MPU6050_motion.” Oct. 21, 2014. 
 * Accessed: Nov. 17, 2021. [Online]. 
 * Available: https://github.com/jarzebski/Arduino-MPU6050/blob/master/MPU6050_motion/MPU6050_motion.ino
 *
 * Referensi
 * C. Newman, “Blink.” Sep. 08, 2016. 
 * Accessed: Nov. 04, 2021. [Online]. 
 * Available: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
 *
 * Referensi
 * C. Newman, “Blink.” Sep. 08, 2016. 
 * Accessed: Nov. 04, 2021. [Online]. 
 * Available: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
 *
 * R. Santos, “ESP32 Send Emails using an SMTP Server: 
 * HTML, Text, and Attachments (Arduino IDE).
 * ”https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/ (accessed Nov. 18, 2021).
 *
 * B. Rubell, “adafruitio_secure_esp32.” May 13, 2021. 
 * Accessed: Nov. 18, 2021. [Online].
 * Available: https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/examples/adafruitio_secure_esp32/adafruitio_secure_esp32.ino
 */

/************************* Import Library *********************************/
// MPU6050
#include <Wire.h>
#include <MPU6050.h>

// AD8232
#include <stdio.h>
#include <stdlib.h>

// Button
#include <DebouncedInput.h>

// Email
#include <Arduino.h>
#include <ESP_Mail_Client.h>

// Dashboard Adafruit
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* Deklarasi Konstanta *********************************/

// MPU6050
// AD8232
#define M       5
#define N       30
#define winSize     250 
#define HP_CONSTANT   ((float) 1 / (float) M)
#define RAND_RES 100000000
#define BPM_BUFFER_SIZE 25
const long PERIOD = 1000000 / winSize;

// Email
#define WIFI_SSID "Galaxy Tab A8"
#define WIFI_PASSWORD "123456789"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "renatawilia23@gmail.com"
#define AUTHOR_PASSWORD "11renatawilia10"

/* Recipient's email*/
#define RECIPIENT_EMAIL "morenzoe@gmail.com"

// Dashboard Adafruit
#define WLAN_SSID "Morenzoe"
#define WLAN_PASS "123456789"

// mengatur informasi WiFi
#define WLAN_SSID "Morenzoe"
#define WLAN_PASS "123456789"

// mengatur informasi server adafruit
#define AIO_SERVER      "io.adafruit.com"

// mengatur port MQTT di 8883
#define AIO_SERVERPORT  8883

// mengatur informasi akun adafruit
#define AIO_USERNAME "morenzoe"
#define AIO_KEY      "aio_koos42yLEedUSso9pWfcMVOG7mfJ"

// mengatur informasi sertifikat autoritass adafruit.io
const char* adafruitio_root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
    "-----END CERTIFICATE-----\n";
    
/************************* Deklarasi Variabel *********************************/

// MPU6050
long gerakTerakhir;
boolean terjatuh = false;
unsigned long waktuJatuh = 0;
boolean resting = true;

// AD8232
unsigned long foundTimeMicros = 0;        
unsigned long old_foundTimeMicros = 0;
unsigned long previousMicros  = 0;
unsigned long currentMicros   = 0;
float bpm = 0;
unsigned long bpm_buff[BPM_BUFFER_SIZE] = {0};
int bpm_buff_WR_idx = 0;
int bpm_buff_RD_idx = 0;
int tmp = 0;
float ecg_buff[M + 1] = {0};
int ecg_buff_WR_idx = 0;
int ecg_buff_RD_idx = 0;
float hp_buff[N + 1] = {0};
int hp_buff_WR_idx = 0;
int hp_buff_RD_idx = 0;
float next_eval_pt = 0;
float hp_sum = 0;
float lp_sum = 0;
float treshold = 0;
boolean triggered = false;
int trig_time = 0;
float win_max = 0;
int win_idx = 0;
int number_iter = 0;

// Button
// Email
boolean terkirim = false;

// Dashboard Adafruit
unsigned long waktuPublish = 0;

/************************* Pengaturan *********************************/

// MPU6050
// mendeklarasi objek modul sensor
MPU6050 mpu;

// Button
// mendeklarasi objek button pada pin 14, debounce 20 ms,
// internal pull up, dan menyimpan tiga waktu tekan
DebouncedInput button(14, 20, true, 3);

// Email
// mengatur objek pengirim email
SMTPSession smtp;

// Dashboard Adafruit
// mendeklarasi kelas klien WiFi
WiFiClientSecure client;

// mengatur kelas MQTT dengan klien WiFi
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// mendeklarasi objek pengiriman data ke dashboard Adafruit
Adafruit_MQTT_Publish HeartRate = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/HeartRate");
Adafruit_MQTT_Publish FallIndicator = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/FallIndicator");

void setup() {
    // memulai komunikasi serial
    Serial.begin(115200);

    // memulai mode debugging email
    smtp.debug(1);
    
    // memulai fungsi button
    button.begin();

    // meghubungkan WiFi
    Serial.println(); Serial.println();
    Serial.print("Menghubungkan WiFi ");
    Serial.println(WLAN_SSID);

    delay(1000);

    WiFi.begin(WLAN_SSID, WLAN_PASS);
    delay(2000);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println();

    Serial.println("WiFi terhubung");
    Serial.println("Alamat IP: "); 
    Serial.println(WiFi.localIP());

    // mengatur sertifikat autoritas adafruit.io
    client.setCACert(adafruitio_root_ca);
    
    // menghubungkan MQTT
    MQTT_connect();

    // menginisiasi modul MPU6050
    while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))  {
        delay(500);
    }
    
    // mengatur parameter sensor MPU6050
    mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
    mpu.setDHPFMode(MPU6050_DHPF_5HZ);
    
    // mengatur nilai ambang batas deteksi pergerakan
    mpu.setMotionDetectionThreshold(2);
    mpu.setMotionDetectionDuration(5);
    
    // mengatur lampu LED_BUILTIN ESP32
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    // mengatur nilai ambang batas deteksi terjatuh
    mpu.setFreeFallDetectionThreshold(70);
    mpu.setFreeFallDetectionDuration(2);  
    
    // mengatur active buzzer
    pinMode(13, OUTPUT);
    digitalWrite(2, LOW);
    
    // mengatur button
    pinMode(14,INPUT);
    
    // mengatur AD8232
    pinMode(33, INPUT);
    pinMode(16, INPUT);
    pinMode(17, INPUT);  
}

/************************* Algoritma Utama *********************************/

void loop() {
    // menginisiasi variabel waktu periode fungsi
    currentMicros = micros();
    
    // menjalankan fungsi setiap periode
    if (currentMicros - previousMicros >= PERIOD) {
        // memperbaharui variabel waktu fungsi sebelumnya
        previousMicros = currentMicros;

       // menginisiasi pendeteksian kompleks QRS
       boolean QRS_detected = false;
        
       // membaca pemasangan elektroda
       boolean leads_are_on = (digitalRead(16) == 0) && (digitalRead(17) == 0);
        
        if(leads_are_on) {
            // membaca data ECG
            int next_ecg_pt = analogRead(33);
          
            // mendeteksi kompleks QRS
            QRS_detected = detect(next_ecg_pt);
                
            if (QRS_detected == true) {
                // menginisaisi variabel durasi antar kompleks QRS ketika terdeteksi
                foundTimeMicros = micros();
                Serial.println("beat!");
            
                if (resting) {
                    // menjalankan perhitungan detak jantung ketika tidak bergerak
                    bpm_buff[bpm_buff_WR_idx] = (60.0 / (((float) (foundTimeMicros - old_foundTimeMicros)) / 1000000.0));
                    bpm_buff_WR_idx++;
                    bpm_buff_WR_idx %= BPM_BUFFER_SIZE;

                    bpm += bpm_buff[bpm_buff_RD_idx];
                
                    tmp = bpm_buff_RD_idx - BPM_BUFFER_SIZE + 1;
                    if(tmp < 0) tmp += BPM_BUFFER_SIZE;
                
                    bpm -= bpm_buff[tmp];
                    
                    bpm_buff_RD_idx++;
                    bpm_buff_RD_idx %= BPM_BUFFER_SIZE;
                }

            old_foundTimeMicros = foundTimeMicros;
            }
        }    
    }
  
    // mendeteksi pergerakan
    Activites act = mpu.readActivites();

    if (act.isActivity) {
    // menyalakan LED dan menginisiasi variabel waktu gerakan jika terdeteksi pergerakan
    digitalWrite(2, HIGH);
    gerakTerakhir = millis();    
    } 
    else {
        // mematikan LED jika tidak terdeteksi pergerakan
    digitalWrite(2, LOW);
    }

    if (millis() - gerakTerakhir >= 3000) {
    // mengubah variabel resting menjadi true jika tidak terdeteksi pergerakan selama tiga detik
    resting = true;
     
    // memulai perhitungan detak jantung
    float heartRate = bpm / ((float) BPM_BUFFER_SIZE - 1);
    Serial.println(heartRate);
    Serial.println("Menghitung detak jantung");

        if (millis() > 30000) {
            // melaporkan hasil perhitungan setelah 30 detik alat bekerja
            if ((millis() - waktuPublish) > 5000) {
                // mwelaporkan hasil perhitungan setiap lima detik
                HeartRate.publish(heartRate);
                waktuPublish = millis();
            }
        }
    }
    else {
    // mengubah variabel resting menjadi false jika terdeteksi pergerakan selama tiga detik
    resting = false;
    
        if ((act.isFreeFall)&&(!terjatuh)) {            
        // mengaktivasi sistem peringatan jika terjatuh
            waktuJatuh = millis();
            digitalWrite(13,HIGH);
            terjatuh = true;
            FallIndicator.publish(1);
        }
    
    // menulis keadaan sistem untuk pengawakutuan
    Serial.println("Tidak menghitung detak jantung");
    }



    if ((terjatuh) && ((millis() - waktuJatuh) > 5000) && (!terkirim)) {
        // mendeklarasi objek sesi pengiriman email
        ESP_Mail_Session session;

        // mengatur informasi sesi
        session.server.host_name = SMTP_HOST;
        session.server.port = SMTP_PORT;
        session.login.email = AUTHOR_EMAIL;
        session.login.password = AUTHOR_PASSWORD;
        session.login.user_domain = "";

        // mendeklarasi objek pesan
        SMTP_Message message;

        // mengatur informasi pesan
        message.sender.name = "Elderly Health Monitor";
        message.sender.email = AUTHOR_EMAIL;
        message.subject = "PERHATIAN!";
        message.addRecipient("Moren", RECIPIENT_EMAIL); 

        /// mengirim pesan teks
        String textMsg = "Pengguna membutuhkan bantuan!";
        message.text.content = textMsg.c_str();
        message.text.charSet = "us-ascii";
        message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

        message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
        message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

        // menghubungkan sesi server
        if (!smtp.connect(&session)) {
            Serial.println("can't connect");
        }

        // memulai pengiriman email
        if (!MailClient.sendMail(&smtp, &message)) {
            Serial.println("Error sending Email, " + smtp.errorReason());
        }
        
        // mengubah kondisi pengiriman email
        terkirim = true;
    }

    if (button.changedTo(LOW)) {
        if ((button.getLowTime(2) > 0) && (millis() - button.getLowTime(2) < 2000)) {
        // mengaktivasi sistem peringatan berdasarkan kondisi
        Serial.println("Aktivasi manual");
        button.clearTimes();

            if (terjatuh) {
                //  mematikan peringatan
                Serial.println("Tertangani-------------");
                digitalWrite(13,LOW);
                terjatuh = false;
                FallIndicator.publish(0); 
                terkirim = false;
            }
            else {
                // menyalakan peringatan
                Serial.println("Butuh Pertolongan------");
                digitalWrite(13,HIGH);
                terjatuh = true;
                waktuJatuh = millis();
                FallIndicator.publish(1);
            }
        }
    }
}


// fungsi deteksi kompleks QRS
boolean detect(float new_ecg_pt) {
    // mengisi buffer sinyal ECG
    ecg_buff[ecg_buff_WR_idx++] = new_ecg_pt;  
    ecg_buff_WR_idx %= (M+1);

    // filter high pass
    if(number_iter < M) {
        // mengisi buffer dengan angka nol
        hp_sum += ecg_buff[ecg_buff_RD_idx];
        hp_buff[hp_buff_WR_idx] = 0;
    }
    else {
        // mengisi buffer dengan sinyal ECG
        hp_sum += ecg_buff[ecg_buff_RD_idx];

        tmp = ecg_buff_RD_idx - M;
        
        if (tmp < 0) {
            tmp += M + 1;
        }
        
        hp_sum -= ecg_buff[tmp];

        // menginisialisasi variabel perhitungan
        float y1 = 0;
        float y2 = 0;

        tmp = (ecg_buff_RD_idx - ((M+1)/2));
        
        if (tmp < 0) {
            tmp += M + 1;
        }
        
        y2 = ecg_buff[tmp];

        y1 = HP_CONSTANT * hp_sum;

        hp_buff[hp_buff_WR_idx] = y2 - y1;
    }

    // menambah index buffer ECG
    ecg_buff_RD_idx++;
    ecg_buff_RD_idx %= (M+1);

    // menambah indeks buffer high pass
    hp_buff_WR_idx++;
    hp_buff_WR_idx %= (N+1);

    // filter low pass
    // menjumlahkan nilai filter high pass
    lp_sum += hp_buff[hp_buff_RD_idx] * hp_buff[hp_buff_RD_idx];

    if (number_iter < N) {
        // mengisi buffer low pass dengan angka nol
        next_eval_pt = 0;    
    }
    else {
        // mengisi buffer low pass dengan hasil filter high pass
        tmp = hp_buff_RD_idx - N;
    
        if (tmp < 0) {
            tmp += (N+1);
        }

        lp_sum -= hp_buff[tmp] * hp_buff[tmp];

        next_eval_pt = lp_sum;
    }

    // menambah indeks buffer low pass
    hp_buff_RD_idx++;
    hp_buff_RD_idx %= (N+1);

    // mendeteksi kompleks QRS dengan ambang batas adaptif
    // menginisialisasi ambang batas
    if (number_iter < winSize) {
        if (next_eval_pt > treshold) {
            treshold = next_eval_pt;
        }
        
        number_iter++;
    }

    // menunggu kompleks QRS sebelumnya selesai
    if (triggered == true) {
        trig_time++;

        if (trig_time >= 100) {
            triggered = false;
            trig_time = 0;
        }
    }

    // memperbaharui nilai maksimum dalam suatu jendela
    if (next_eval_pt > win_max) {
        win_max = next_eval_pt;
    }

    // kompleks QRS ditemukan jika melebihi ambang batas
    if (next_eval_pt > treshold && !triggered) {
        triggered = true;

        // mengembalikan nilai true
        return true;
    }

    // menyesuaikan ambang batas adaptif berdasarkan sinyal sebelumnya
    if (win_idx++ >= winSize) {
        // deklarasi variabel koefisien perhitungan
        float gamma = 0.175;
        float alpha = 0.01 + ( ((float) random(0, RAND_RES) / (float) (RAND_RES)) * ((0.1 - 0.01)));

        // menghitung nilai ambang batas adaptif baru
        treshold = alpha * gamma * win_max + (1 - alpha) * treshold;

        // mengembalikan nilai indeks jendela ke awal
        win_idx = 0;
        win_max = -10000000;
    }

    // mengembalikan nilai false jika tidak terdeteksi
    return false;
}


// fungsi untuk memulai koneksi MQTT
void MQTT_connect() {
    int8_t koneksi; // kondisi koneksi, 0 jika terkoneksi

    // memulai koneksi jika belum terkoneksi
    if (mqtt.connected()) {
        return;
    }

    Serial.print("Menghubungkan MQTT... ");

    uint8_t coba = 3;
    // mencoba memulai koneksi sebanyak tiga kali
    while ((koneksi = mqtt.connect()) != 0) { 
        Serial.println(mqtt.connectErrorString(koneksi));
        Serial.println("Mencoba ulang koneksi MQTT dalam 5 detik...");
        mqtt.disconnect();
        delay(5000);
        coba--;
        
        if (coba == 0) {
            // menunggu reset
            while (1);
        }
    }
    
    Serial.println("MQTT terkoneksi!");
}
