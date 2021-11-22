/*
 * Program Button Tiga Kali
 *
 * Koneksi Pin
 * ESP32 | Button
 * ---------------
 *  14   |  VCC
 *  GND  |  GND
 *  
 * Variabel
 * kondisiLED = kondisi nyala mati LED
 *  
 * Referensi
 * C. Newman, “Blink.” Sep. 08, 2016. 
 * Accessed: Nov. 04, 2021. [Online]. 
 * Available: https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink
 */

// import library
#include <DebouncedInput.h>

// deklarasi variabel
boolean kondisiLED = LOW;

// deklarasi objek button pada pin 14, debounce 20 ms,
// internal pull up, dan menyimpan tiga waktu tekan
DebouncedInput button(14, 20, true, 3);

void setup() {
	// memulai komunikasi serial
  Serial.begin(9600);

  // memulai fungsi button
	button.begin();

  // mengatur lampu LED_BUILTIN ESP32
  pinMode(2, OUTPUT);
}

void loop() {
  // mengatur nyala LED sesuai kondisi
  digitalWrite(2, kondisiLED);
  
  // jika button berubah state dari HIGH ke LOW
	if (button.changedTo(LOW)) {
    // jika waktu dua tekan sebelumnya kurang dari 2 detik
		if ((button.getLowTime(2) > 0) && (millis() - button.getLowTime(2) < 2000)) {
      // aktivasi fungsi
			kondisiLED = !kondisiLED;
			Serial.println("Fungsi teraktivasi");
            
      // menghapus waktu tekan yang disimpan
			button.clearTimes();
		}
	}
}
