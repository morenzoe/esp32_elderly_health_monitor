/*
 * Program Filter Pan Tompkins
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
 * raw : sinyal ECG awal dari modul AD8232
 * out : sinyal hasil filter
 * sum : penjumlahan moving window integration
 * N : lebar jendela integrasi
 * inp, lpfout, hpfout, sqout: list data untuk buffer
 *  
 * Referensi
 * J. Pan and W. J. Tompkins, “A Real-Time QRS Detection Algorithm,” 
 * IEEE Transactions on Biomedical Engineering, vol. BME-32, no. 3, 
 * pp. 230–236, Mar. 1985, doi: 10.1109/TBME.1985.325532.
 */

#include <CircularBuffer.h>

CircularBuffer<int,33> inp;
CircularBuffer<int,33> lpfout;
CircularBuffer<int,33> hpfout;
CircularBuffer<int,33> sqout;

int raw;
int out; 
int sum = 0;
const int N = 30;

void setup() {
  // memulai komunikasi serial
  Serial.begin(9600);
  delay(2000);

  // memenuhi buffer dengan angka nol
  for (int i = 0; i <= 32; i++) {
    //sig = analogRead(A0);
    inp.unshift(0);
    lpfout.unshift(0);
    hpfout.unshift(0);
    sqout.unshift(0);
}

}

void loop() {
  // mendeteksi pemasangan elektroda
  if((digitalRead(16) == 1)||(digitalRead(17) == 1)){
    
    // mode standby
    digitalWrite(18, LOW);
    
  }
  else{
    // mode normal
    digitalWrite(18, HIGH); 
      raw = analogRead(33);
      inp.unshift(raw);
      
      // lowpass filter
      out = 2*lpfout[0] - lpfout[1] + inp[0] - 2*inp[6] + inp[12];
      lpfout.unshift(out);  // this becomes lpfout[0] for the next loop
      
      // highpass filter
      //filt = hpfout[0] - lpfout[0]/32 + lpfout[16] - lpfout[17] + lpfout[32]/32;
      out = 32*lpfout[16] - (hpfout[0] + lpfout[0] - lpfout[32]);
      hpfout.unshift(out);

      // derivative step
      out = (0.125)*(-1*hpfout[4]-2*hpfout[3]+2*hpfout[1]+hpfout[0]);

      // squaring
      out = pow(out,2);
      sqout.unshift(out);

      // moving-window integration
      sum = (sum + sqout[0]/N - sqout[N]/N);

      // mengirimkan pengukuran ke serial monitor
      Serial.println(val);
      
  }
  
  // frekuensi 200 sampel per detik, periode 5 ms
  delay(5);
}
