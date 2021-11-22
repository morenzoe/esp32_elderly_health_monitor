/*
 * Program Dashboard Adafruit.io
 * 
 * Variabel
 * count: nilai counter untuk diupload
 * koneksi: kondisi koneksi WiFi
 * coba: jumlah coba ulang koneksi
 *
 * Referensi 
 * B. Rubell, “adafruitio_secure_esp32.” May 13, 2021. 
 * Accessed: Nov. 18, 2021. [Online].
 * Available: https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/examples/adafruitio_secure_esp32/adafruitio_secure_esp32.ino
 */

// install library
#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// deklarasi variabel
int count = 0;

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

// mendeklarasi kelas klien WiFi
WiFiClientSecure client;

// mengatur kelas MQTT dengan klien WiFi
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

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

// mengatur pengiriman data ke adafruit
Adafruit_MQTT_Publish Counter = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Counter");

void setup() {
  // memulai komunikasi serial
  Serial.begin(115200);
  delay(10);
  
  // memulai koneksi WiFi
  Serial.print("Menghubungkan WiFi");
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
}

void loop() {
  // menghubungkan MQTT
  MQTT_connect();

  // mengirimkan nilai
  Serial.print("Mengirim nilai: ");
  Serial.println(count);
  if (! Counter.publish(count++)) {
    Serial.println(F("Tidak Terkirim"));
  } else {
    Serial.println(F("Terkirim"));
  }
  
  // tunggu selama dua detik sesuai limit
  delay(2000);  
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
