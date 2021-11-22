/*
 * Program Gmail
 * 
 * Referensi
 * R. Santos, “ESP32 Send Emails using an SMTP Server: 
 * HTML, Text, and Attachments (Arduino IDE).
 * ”https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/ (accessed Nov. 18, 2021).
 */

// import library
#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>

// mengatur iformasi wifi
#define WIFI_SSID "Galaxy Tab A8"
#define WIFI_PASSWORD "123456789"

// mengatur informasi email
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// mengatur informasi akun pengirim
#define AUTHOR_EMAIL "renatawilia23@gmail.com"
#define AUTHOR_PASSWORD "11renatawilia10"

// mengatur informasi akun penerima
#define RECIPIENT_EMAIL "morenzoe@gmail.com"

// mengatur objek pengirim email
SMTPSession smtp;

void setup(){
  // memulai komunikasi serial
  Serial.begin(115200);
  Serial.println();

  // memulai koneksi WiFi  
  Serial.print("Menghubungkan WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi terhubung.");
  Serial.println("Alamat IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  // memulai mode debugging
  smtp.debug(1); 
}

void loop(){
  // mengirim email setiap 10 detik
  if (millis() == 10000) {
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
    message.subject = "Attention!";
    message.addRecipient("Moren", RECIPIENT_EMAIL); 
   
    // mengirim pesan teks
    String textMsg = "Testing!";
    message.text.content = textMsg.c_str();
    message.text.charSet = "us-ascii";
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    // menghubungkan sesi server
    if (!smtp.connect(&session)){
      Serial.println("Tidak bisa menghubungkan server.");
    }

    // memulai pengiriman email
    if (!MailClient.sendMail(&smtp, &message)) {
      Serial.println("Gagal mengirim email: " + smtp.errorReason());
    }  
  }
}
