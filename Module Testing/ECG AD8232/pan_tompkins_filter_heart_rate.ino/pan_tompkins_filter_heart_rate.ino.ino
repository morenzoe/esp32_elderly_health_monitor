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
 * Referensi
 * B. Milner, “QRS.” Dec. 24, 2015. 
 * Accessed: Nov. 14, 2021. [Online]. 
 * Available: https://github.com/blakeMilner/real_time_QRS_detection/blob/master/QRS_arduino/QRS.ino
 */

// import library
#include <stdio.h>
#include <stdlib.h>

// deklarasi variabel
#define M       5
#define N       30
#define winSize     250 
#define HP_CONSTANT   ((float) 1 / (float) M)
#define RAND_RES 100000000
unsigned long foundTimeMicros = 0;        
unsigned long old_foundTimeMicros = 0;
unsigned long previousMicros  = 0;
unsigned long currentMicros   = 0;
float bpm = 0;
#define BPM_BUFFER_SIZE 25
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
const long PERIOD = 1000000 / winSize;

void setup() {
  // memulai komunikasi serial
  Serial.begin(115200);
  
  // mengatur mode pin
  pinMode(33, INPUT);
  pinMode(16, INPUT);
  pinMode(17, INPUT);
}

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
    
    if (leads_are_on) {           
      // membaca data ECG
      int next_ecg_pt = analogRead(33);
      
      // mendeteksi kompleks QRS
      QRS_detected = detect(next_ecg_pt);
            
      if (QRS_detected == true) {
        // menginisaisi variabel durasi antar kompleks QRS
        foundTimeMicros = micros();
        Serial.println("beat!");

        // mengisi buffer perhitungan bpm
        bpm_buff[bpm_buff_WR_idx] = (60.0 / (((float) (foundTimeMicros - old_foundTimeMicros)) / 1000000.0));
        bpm_buff_WR_idx++;
        bpm_buff_WR_idx %= BPM_BUFFER_SIZE;
        bpm += bpm_buff[bpm_buff_RD_idx];
        tmp = bpm_buff_RD_idx - BPM_BUFFER_SIZE + 1;
        if(tmp < 0) tmp += BPM_BUFFER_SIZE;
        bpm -= bpm_buff[tmp];
        bpm_buff_RD_idx++;
        bpm_buff_RD_idx %= BPM_BUFFER_SIZE;
        old_foundTimeMicros = foundTimeMicros;

        // melaporkan perhitungan bpm
        Serial.println(bpm / ((float) BPM_BUFFER_SIZE - 1));
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
  if(number_iter < M){
    // mengisi buffer dengan angka nol
    hp_sum += ecg_buff[ecg_buff_RD_idx];
    hp_buff[hp_buff_WR_idx] = 0;
  }
  else{
    // mengisi buffer dengan sinyal ECG
    hp_sum += ecg_buff[ecg_buff_RD_idx];
    
    tmp = ecg_buff_RD_idx - M;
    if(tmp < 0) tmp += M + 1;
    
    hp_sum -= ecg_buff[tmp];

    // menginisialisasi variabel perhitungan
    float y1 = 0;
    float y2 = 0;
    
    tmp = (ecg_buff_RD_idx - ((M+1)/2));
    if(tmp < 0) tmp += M + 1;
    
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

  if(number_iter < N){
    // mengisi buffer low pass dengan angka nol
    next_eval_pt = 0;    
  }
  else{
    // mengisi buffer low pass dengan hasil filter high pass
    tmp = hp_buff_RD_idx - N;
    if(tmp < 0) tmp += (N+1);
    
    lp_sum -= hp_buff[tmp] * hp_buff[tmp];
    
    next_eval_pt = lp_sum;
  }
  
  // menambah indeks buffer low pass
  hp_buff_RD_idx++;
  hp_buff_RD_idx %= (N+1);

  // mendeteksi kompleks QRS dengan ambang batas adaptif
  // menginisialisasi ambang batas
  if(number_iter < winSize) {
    if(next_eval_pt > treshold) {
      treshold = next_eval_pt;
    }
    number_iter++;
  }
  
  // menunggu kompleks QRS sebelumnya selesai
  if(triggered == true){
    trig_time++;
    
    if(trig_time >= 100){
      triggered = false;
      trig_time = 0;
    }
  }
  
  // memperbaharui nilai maksimum dalam suatu jendela
  if(next_eval_pt > win_max) win_max = next_eval_pt;
  
  // kompleks QRS ditemukan jika melebihi ambang batas
  if(next_eval_pt > treshold && !triggered) {
    triggered = true;

    // mengembalikan nilai true
    return true;
  }
       
  // menyesuaikan ambang batas adaptif berdasarkan sinyal sebelumnya
  if(win_idx++ >= winSize){
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
