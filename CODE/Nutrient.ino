/*--------------------------Library----------------------------------------------------------------------*/
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include "DHT.h"
#include "RTClib.h"
#include <EEPROM.h>

/*--------------------------Deklarasi Pin DHT11----------------------------------------------------------*/
#define pinDHT 2 //deklarasi pin DHT
#define tipeDHT DHT11 //definisi tipe DHT
/*--------------------------Deklarasi Pin Aktuator-------------------------------------------------------*/
#define pinBLDC 9
#define pinDosingpump 8
#define pinSolenoid 6
#define pinLampu 7
#define pinAerator 5
/*--------------------------Deklarasi Pin Ultrasonic-----------------------------------------------------*/
#define pinTrigger 3
#define pinEcho 4
/*--------------------------Deklarasi Pin TDS Meter------------------------------------------------------*/
#define pinTDS A0
/*--------------------------Deklarasi Pin LDR------------------------------------------------------------*/
#define pinLDR A1
/*--------------------------Deklarasi Pin Tombol---------------------------------------------------------*/
#define pinMenu A2
#define pinUp A3
#define pinDown 1
#define pinOk 0
/*--------------------------Deklarasi Pembacaan Button---------------------------------------------------*/
#define Menu digitalRead(pinMenu)
#define Up digitalRead(pinUp)
#define Down digitalRead(pinDown)
#define Ok digitalRead(pinOk)
/*--------------------------Deklarasi Objek--------------------------------------------------------------*/
LiquidCrystal_I2C lcd(0x27, 20, 4); //objek lcd
DHT dht(pinDHT, tipeDHT); //objek dht
RTC_DS1307 rtc;
char namaHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
/*--------------------------Variabel Button--------------------------------------------------------------*/
int lockMenu = 0;
int lockUp = 0;
int lockDown = 0;
int lockOk = 0;
int dis;
int dis1;
int mode = 0;
int mode1 = 0;
int mode2 = 0;
int mode3 = 0;
/*--------------------------Variabel DHT-----------------------------------------------------------------*/
float temp = 0.00;
float hum = 0.00;
/*--------------------------Variabel LDR-----------------------------------------------------------------*/
float adcLux = 0;
float vLux = 0;
float totalLux;
float averageLux;
float aLDR, bLDR, cLDR, dLDR = 0;
float batasLux = 55;
float ADC_value = 0.0048828125;
/*--------------------------Variabel TDS-----------------------------------------------------------------*/
float adcPPM = 0;
float aPPM, bPPM, cPPM, PPM;
float batasTds = 5;

/*--------------------------Variabel HCSR-4--------------------------------------------------------------*/
long duration, distance;
float aLiter, bLiter, cLiter, liter;
float batasLiter = 7.00;
float averageDistance;
float totalDistance;

/*--------------------------Variabel Status Aktuator-----------------------------------------------------*/
String sBLDC;
String sDosingpump;
String sSolenoid;
String sLampu;
/*--------------------------Variabel RTC-----------------------------------------------------------------*/
int inTahun, inBulan, inTanggal, inJam, inMenit, inDetik;
int saveTahun, saveBulan, saveTanggal, saveJam, saveMenit, saveDetik;
int batasTanggal;
/*--------------------------Variabel Umur-----------------------------------------------------------------*/
int inTahun1, inBulan1, inTanggal1, inJam1, inMenit1, inDetik1;
int saveTahun1, saveBulan1, saveTanggal1, saveJam1, saveMenit1, saveDetik1;
int batasTanggal1;
/*--------------------------Variabel Umur-----------------------------------------------------------------*/
unsigned long timeAwal, timeAkhir;
unsigned int umurSekarang = 0;
unsigned int readumurSekarang = 0;
unsigned long selisih;
int tanda = 0;
int data;
int lockUmur = 0;
int fasa;

float ppmFasa1 = 0.0;
float ppmFasa2 = 700.0;
float ppmFasa3 = 1050.0;

float ppmPenambahan = 0.0;
float nutrisi = 0.0;
float onDosing = 0.0;
unsigned long waktuSekarang = 0;
unsigned long waktuSebelum = 0;
unsigned long sekarang = 0;
unsigned long sebelum = 0;
unsigned long startFasa;
unsigned long previousFasa;

int address = 0;

#include <UnixTime.h>

UnixTime stamp(7);  // указать GMT (3 для Москвы)
uint32_t unix;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  dht.begin();
  /*--------------------------Konfidurasi Pin IO--------------------------------------------------------------*/
  pinMode(pinLDR, INPUT);
  pinMode(pinTDS, INPUT);
  pinMode(pinMenu,  INPUT_PULLUP);
  pinMode(pinUp,    INPUT_PULLUP);
  pinMode(pinDown,  INPUT_PULLUP);
  pinMode(pinOk,    INPUT_PULLUP);
  pinMode(pinBLDC, OUTPUT);
  pinMode(pinDosingpump, OUTPUT);
  pinMode(pinSolenoid, OUTPUT);
  pinMode(pinLampu, OUTPUT);
  pinMode(pinAerator, OUTPUT);
  pinMode(pinTrigger, OUTPUT);
  pinMode(pinEcho, INPUT);
  /*--------------------------Kondisi Awal Aktuator--------------------------------------------------------------*/
  digitalWrite(pinBLDC, LOW);
  digitalWrite(pinDosingpump, HIGH);
  digitalWrite(pinSolenoid, HIGH);
  digitalWrite(pinLampu, HIGH);

  lcd.setCursor(1, 0);
  lcd.print("Mini Plant Factory");
  lcd.setCursor(4, 1);
  lcd.print("Yusril Fahmi H");
  lcd.setCursor(1, 2);
  lcd.print("Universitas  Tidar");
  lcd.setCursor(5, 3);
  lcd.print("V1.5  2022");

  if (! rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC GAGAL");
    while (1);
  }
  readumurSekarang = EEPROM.read(address);
  umurSekarang = readumurSekarang;
  delay(3000);
  lcd.clear();
}

void loop() {
  if ((sekarang - sebelum > 30000) && dis == 1) {
    lcd.clear();
    tanda = 0;
    dis = 0;
    sebelum = millis();
  }
  tampil();
  bacaSuhu();
  bacaUltra();
  bacaLDR();
  bacaRTC();
  bacaPPM();
  bacaSetup();

}
void tampil() {
  /*--------------------------Fungsi Tampil Awal--------------------------------------------------------------*/
  if (tanda == 0) {
    dis = 0;
    DateTime now = rtc.now();
    lcd.setCursor(0, 0);
    lcd.print(String(now.day(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.year(), DEC) + "  " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC));
    lcd.setCursor(0, 1);
    lcd.print("Umur   : " + String(umurSekarang) + "   ");
    lcd.setCursor(0, 2);
    lcd.print("PPM    : " + String(PPM) + "   ");
    lcd.setCursor(0, 3);
    lcd.print("Volume : " + String(liter) + "   ");
  }
}
/*--------------------------Fungsi Pembacaan Suhu--------------------------------------------------------------*/
void bacaSuhu() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
}
/*--------------------------Fungsi Pembacaan Sensor LDR--------------------------------------------------------*/
void bacaLDR() {
  adcLux = analogRead(pinLDR);
  int sampel = 100;
  for (int i = 0; i < sampel; i++) {
    totalLux += adcLux;
    delay(1);
  }
  averageLux = totalLux / sampel;
  totalLux = 0;
  //aLDR = ((-0.0005) * pow(averageLux, 3));
  //bLDR = ((0.2273) * pow(averageLux, 2));
  //cLDR = (32.761 * averageLux);
  //dLDR = 1793;
  //vLux = aLDR + bLDR - cLDR + dLDR;
  vLux = (250.000000 / (ADC_value *  averageLux)) - 50.000000;

  DateTime now = rtc.now();
  if (now.hour() >= 18 && now.hour() <= 00) {
    digitalWrite(pinLampu, HIGH);
    sLampu = "OFF";
  } else {
    if (vLux < (batasLux - 5)) {
      digitalWrite(pinLampu, LOW);
      sLampu = "ON ";
    }
    if (vLux > (batasLux + 5)) {
      digitalWrite(pinLampu, HIGH);
      sLampu = "OFF";
    }
  }
  delay(100);
}
/*--------------------------Fungsi Pembacaan Ultrasonic HCSR-04------------------------------------------------*/
void bacaUltra() {
  digitalWrite(pinTrigger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigger, LOW);
  duration = pulseIn(pinEcho, HIGH);
  distance = duration / 58.2;

  int sampelDistance = 50;
  for (int i = 0; i < sampelDistance; i++) {
    totalDistance += distance;
    delay(1);
  }
  averageDistance = totalDistance / sampelDistance;
  totalDistance = 0;

  aLiter = ((0.0168) * pow(averageDistance, 2));
  bLiter = (1.6091 * averageDistance);
  cLiter = 36.256;
  liter = aLiter - bLiter + cLiter;
  delay(1);  // delay in between reads for stability

  if (liter < (batasLiter - 2.5)) {
    digitalWrite(pinSolenoid, LOW);
  } else if (liter > (batasLiter + 2.5)) {
    digitalWrite(pinSolenoid, HIGH);
    startFasa = millis();
    if (startFasa - previousFasa > 60000) {
      fasaTanam();
    }
  }
  delay(100);
}
/*--------------------------Fungsi Pembacaan RTC---------------------------------------------------------------*/
void bacaRTC() {
  DateTime now = rtc.now();
}

void bacaPPM() {
  adcPPM = analogRead(pinTDS);
  aPPM = ((0.003) * pow(adcPPM, 2));
  bPPM = (0.5631 * adcPPM);
  cPPM = 57.065;
  PPM = aPPM + bPPM + cPPM;
}

void fasaTanam() {
  if (umurSekarang <= 5 && umurSekarang >= 0) {
    fasa = 1;
  }
  if (umurSekarang <= 13 && umurSekarang >= 6) {
    fasa = 2;
  }
  if (umurSekarang <= 35 && umurSekarang >= 14) {
    fasa = 3;
  }
  bacaPPM();
  if (fasa == 1) {
    ppmPenambahan = 0;
  }
  if (fasa == 2) {
    if (PPM < 700) {
      ppmPenambahan = ppmFasa2 - PPM;
      nutrisi = (ppmPenambahan * 5 * liter) / 2.0 * 500.0;
      onDosing = (1.9935 * nutrisi) - 0.1036;
      waktuSekarang = millis();
      if ((waktuSekarang - waktuSebelum) > (onDosing + 60000)) {
        digitalWrite(pinDosingpump, HIGH);
        digitalWrite(pinAerator, HIGH);
        waktuSebelum = millis();
        previousFasa = millis();
      } else {
        digitalWrite(pinDosingpump, LOW);
        digitalWrite(pinAerator, LOW);
      }
    } else {
      digitalWrite(pinDosingpump, HIGH);
    }
  }
  if (fasa == 3) {
    if (PPM < 1050) {
      ppmPenambahan = ppmFasa3 - PPM;
      nutrisi = (ppmPenambahan * 5 * liter) / 2.0 * 500.0;
      onDosing = (1.9935 * nutrisi) - 0.1036;

      waktuSekarang = millis();
      if ((waktuSekarang - waktuSebelum) > (onDosing + 60000)) {
        digitalWrite(pinDosingpump, HIGH);
        digitalWrite(pinAerator, HIGH);
        waktuSebelum = millis();
        previousFasa = millis();
      } else {
        digitalWrite(pinDosingpump, LOW);
        digitalWrite(pinAerator, LOW);
      }
    }
  }
}

/*--------------------------Fungsi Setup Menu---------------------------------------------------------------*/
void bacaSetup() {
  if (Menu == 0 && lockMenu == 0) {
    lockMenu = 1;
  }
  if (Menu == 0 && dis == 0) {
    lcd.clear();
    lockMenu = 0;
    dis = 1;
    tanda = 1;
  }
  /*--------------------------Tampilan Awal Menu-------------------------------------------------------------*/
  if (dis == 1) {
    sekarang = millis();
    lcd.setCursor(5, 0);
    lcd.print("MENU UTAMA");
    lcd.setCursor(1, 1);
    lcd.print("DHT22");
    lcd.setCursor(1, 2);
    lcd.print("LDR");
    lcd.setCursor(1, 3);
    lcd.print("TDS");
    lcd.setCursor(11, 1);
    lcd.print("HCSR-04");
    lcd.setCursor(11, 2);
    lcd.print("RTC");
    lcd.setCursor(11, 3);
    lcd.print("UMUR");

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      mode++;
      if (mode > 5) {
        mode = 0;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      mode--;
      if (mode < 0) {
        mode = 5;
      }
    }

    switch (mode) {
      case 0 : lcd.setCursor(0, 1); lcd.print(">"); break;
      case 1 : lcd.setCursor(0, 2); lcd.print(">"); break;
      case 2 : lcd.setCursor(0, 3); lcd.print(">"); break;
      case 3 : lcd.setCursor(10, 1); lcd.print(">"); break;
      case 4 : lcd.setCursor(10, 2); lcd.print(">"); break;
      case 5 : lcd.setCursor(10, 3); lcd.print(">"); break;
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1 && mode == 0) { //masuk ke menu DHT
      lockOk = 0;
      dis = 2;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode == 1) { //masuk ke menu LDR
      lockOk = 0;
      dis = 3;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode == 2) { //masuk ke menu TDS
      lockOk = 0;
      dis = 4;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode == 3) { //masuk ke menu HCSR-04
      lockOk = 0;
      dis = 5;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode == 4) { //masuk ke menu RTC
      lockOk = 0;
      dis = 6;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode == 5) { //masuk ke menu RTC
      lockOk = 0;
      dis = 13;
      lcd.clear();
    }
  }
  /*--------------------------Menu DHT-------------------------------------------------------------*/
  if (dis == 2) {
    lcd.setCursor(1, 0);
    lcd.print("SUHU && KELEMBAPAN");
    lcd.setCursor(0, 1);
    lcd.print("Temp: " + String(temp) + " C");
    lcd.setCursor(0, 2);
    lcd.print("Hum : " + String(hum) + " %");

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      dis = 1;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 2) {
      lcd.clear();
      lockMenu = 0;
      dis = 1;
    }
  }
  /*--------------------------Menu LDR-------------------------------------------------------------*/
  if (dis == 3) {
    lcd.setCursor(5, 0);
    lcd.print("PENCAHAYAAN");
    lcd.setCursor(0, 1);
    lcd.print("Lux Value: " + String(vLux, 2) + "   ");

    if (vLux < (batasLux - 5)) {
      sLampu = "ON ";
      lcd.setCursor(0, 2);
      lcd.print("Lampu    : " + String(sLampu));
    }
    if (vLux > (batasLux + 5)) {
      sLampu = "OFF";
      lcd.setCursor(0, 2);
      lcd.print("Lampu    : " + String(sLampu));
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      dis = 1;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 3) {
      lcd.clear();
      lockMenu = 0;
      dis = 1;
    }
  }
  /*--------------------------Menu TDS-------------------------------------------------------------*/
  if (dis == 4) {
    lcd.setCursor(5, 0);
    lcd.print("TDS  METER");
    lcd.setCursor(0, 1);
    lcd.print("Fasa: " + String(fasa));
    lcd.setCursor(0, 2);
    lcd.print("PPM : " + String(PPM, 2) + "   ");
    if (PPM < batasTds) {
      sBLDC = "ON ";
      lcd.setCursor(0, 3);
      lcd.print("Dosing Pump: " + String(sBLDC));
    } else {
      sBLDC = "OFF";
      lcd.setCursor(0, 3);
      lcd.print("Dosing Pump: " + String(sBLDC));
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      dis = 1;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 4) {
      lcd.clear();
      lockMenu = 0;
      dis = 1;
    }
  }
  /*--------------------------Menu HCSR-04-------------------------------------------------------------*/
  if (dis == 5) {
    lcd.setCursor(5, 0);
    lcd.print("VOLUME AIR");
    lcd.setCursor(0, 1);
    lcd.print("Set Volume: " + String(batasLiter));
    lcd.setCursor(0, 2);
    lcd.print("Value:" + String(liter, 2) + "   ");
    lcd.setCursor(16, 2);
    lcd.print("ltr");
    lcd.setCursor(0, 3);
    lcd.print("Solenoid:" + String(sSolenoid));

    if (liter < (batasLiter - 0.2)) {
      sSolenoid = "ON ";
    } if (liter > (batasLiter + 0.2)) {
      sSolenoid = "OFF";
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      dis = 1;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 5) {
      lcd.clear();
      lockMenu = 0;
      dis = 1;
    }
  }
  /*--------------------------Menu RTC-------------------------------------------------------------*/
  if (dis == 6) {
    DateTime now = rtc.now();
    lcd.setCursor(0, 0);
    lcd.print(String(now.day(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.year(), DEC) + "  " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC));
    lcd.setCursor(1, 1);
    lcd.print("Tahun");
    lcd.setCursor(1, 2);
    lcd.print("Bulan");
    lcd.setCursor(1, 3);
    lcd.print("Tanggal");
    lcd.setCursor(11, 1);
    lcd.print("Jam");
    lcd.setCursor(11, 2);
    lcd.print("Menit");
    lcd.setCursor(11, 3);
    lcd.print("Detik");

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      mode1++;
      if (mode1 > 5) {
        mode1 = 0;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      mode1--;
      if (mode1 < 0) {
        mode1 = 5;
      }
    }

    switch (mode1) {
      case 0 : lcd.setCursor(0, 1); lcd.print(">"); break;
      case 1 : lcd.setCursor(0, 2); lcd.print(">"); break;
      case 2 : lcd.setCursor(0, 3); lcd.print(">"); break;
      case 3 : lcd.setCursor(10, 1); lcd.print(">"); break;
      case 4 : lcd.setCursor(10, 2); lcd.print(">"); break;
      case 5 : lcd.setCursor(10, 3); lcd.print(">"); break;
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1 && mode1 == 0) { //masuk ke setup tahun
      lockOk = 0;
      DateTime now = rtc.now();
      inTahun = now.year();
      dis = 7;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode1 == 1) { //masuk ke setup bulan
      lockOk = 0;
      DateTime now = rtc.now();
      inBulan = now.month();
      dis = 8;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode1 == 2) { //masuk ke setup tanggal
      lockOk = 0;
      DateTime now = rtc.now();
      inTanggal = now.day();
      dis = 9;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode1 == 3) { //masuk ke setup jam
      lockOk = 0;
      DateTime now = rtc.now();
      inJam = now.hour();
      dis = 10;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode1 == 4) { //masuk ke setup menit
      lockOk = 0;
      DateTime now = rtc.now();
      inMenit = now.minute();
      dis = 11;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode1 == 5) { //masuk ke setup detik
      lockOk = 0;
      DateTime now = rtc.now();
      inDetik = now.second();
      dis = 12;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 6) {
      lcd.clear();
      lockMenu = 0;
      dis = 1;
    }
  }
  /*--------------------------Sub Menu Setup Tahun-------------------------------------------------------------*/
  if (dis == 7) {
    lcd.setCursor(0, 0);
    lcd.print("Tahun: " + String(inTahun));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inTahun++;
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inTahun--;
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveTahun = inTahun;
      mode1 = 1;
      dis = 6;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 7) {
      lcd.clear();
      lockMenu = 0;
      dis = 6;
    }
  }
  /*--------------------------Sub Menu Setup Bulan-------------------------------------------------------------*/
  if (dis == 8) {
    lcd.setCursor(0, 0);
    lcd.print("Bulan: " + String(inBulan));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inBulan++;
      if (inBulan > 12) {
        inBulan = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inBulan--;
      if (inBulan < 1) {
        inBulan = 12;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveBulan = inBulan;
      mode1 = 2;
      dis = 6;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 8) {
      lcd.clear();
      lockMenu = 0;
      dis = 6;
    }
  }
  /*--------------------------Sub Menu Setup Tanggal-------------------------------------------------------------*/
  if (dis == 9) {
    if (saveBulan == 1 || saveBulan == 3 || saveBulan == 5 || saveBulan == 7 || saveBulan == 8 || saveBulan == 10 || saveBulan == 12) {
      batasTanggal = 31;
    } else if (saveBulan == 4 || saveBulan == 6 || saveBulan == 9 || saveBulan == 11) {
      batasTanggal = 30;
    } else if (saveBulan == 2) {
      batasTanggal = 28;
    }

    lcd.setCursor(0, 0);
    lcd.print("Tanggal: " + String(inTanggal));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inTanggal++;
      if (inTanggal > batasTanggal) {
        inTanggal = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inTanggal--;
      if (inTanggal < 1) {
        inTanggal = batasTanggal;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveTanggal = inTanggal;
      mode1 = 3;
      dis = 6;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 9) {
      lcd.clear();
      lockMenu = 0;
      dis = 6;
    }
  }
  /*--------------------------Sub Menu Setup Jam-------------------------------------------------------------*/
  if (dis == 10) {
    lcd.setCursor(0, 0);
    lcd.print("Jam: " + String(inJam));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inJam++;
      if (inJam > 60) {
        inJam = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inJam--;
      if (inJam < 1) {
        inJam = 60;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveJam = inJam;
      mode1 = 4;
      dis = 6;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 10) {
      lcd.clear();
      lockMenu = 0;
      dis = 6;
    }
  }
  /*--------------------------Sub Menu Setup Menit-------------------------------------------------------------*/
  if (dis == 11) {
    lcd.setCursor(0, 0);
    lcd.print("Menit: " + String(inMenit));
    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inMenit++;
      if (inMenit > 60) {
        inMenit = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inMenit--;
      if (inMenit < 1) {
        inMenit = 60;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveMenit = inMenit;
      mode1 = 5;
      dis = 6;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 11) {
      lcd.clear();
      lockMenu = 0;
      dis = 6;
    }
  }
  /*--------------------------Sub Menu Setup Detik-------------------------------------------------------------*/
  if (dis == 12) {
    lcd.setCursor(0, 0);
    lcd.print("Detik: " + String(inDetik));
    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inDetik++;
      if (inDetik > 60) {
        inDetik = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inDetik--;
      if (inDetik < 1) {
        inDetik = 60;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveDetik = inDetik;
      mode1 = 0;
      rtc.adjust(DateTime(saveTahun, saveBulan, saveTanggal, saveJam, saveMenit, saveDetik));
      dis = 6;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 12) {
      lcd.clear();
      lockMenu = 0;
      dis = 6;
    }
  }
  /*--------------------------Menu Umur-------------------------------------------------------------*/
  if (dis == 13) {
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0) {
      lcd.clear();
      lockMenu = 0;
      dis = 13;
    }
    if (lockUmur == 0) {
      umurSekarang = 0;
    }
    if (lockUmur == 1) {
      DateTime now = rtc.now();
      timeAkhir = now.unixtime();
      selisih = timeAkhir - timeAwal;
      umurSekarang = selisih / 86400;
      EEPROM.write(address, umurSekarang);
    }

    lcd.setCursor(1, 0);
    lcd.print("Masa Tanam: " + String(umurSekarang) + " Hari");
    lcd.setCursor(1, 1);
    lcd.print("Set Mulai Tanam");

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      mode2++;
      if (mode2 > 1) {
        mode2 = 0;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      mode2--;
      if (mode2 < 0) {
        mode2 = 1;
      }
    }

    switch (mode2) {
      case 0 : lcd.setCursor(0, 0); lcd.print(">"); break;
      case 1 : lcd.setCursor(0, 1); lcd.print(">"); break;
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1 && mode2 == 0) { //masuk ke umur
      lockOk = 0;
      dis = 14;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode2 == 1) { //masuk ke setup start tanam
      lockOk = 0;
      dis = 15;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 13) {
      lcd.clear();
      lockMenu = 0;
      dis = 1;
    }
  }

  if (dis == 14) {
    if (lockUmur == 0) {
      umurSekarang = 0;
    }
    if (lockUmur == 1) {
      DateTime now = rtc.now();
      timeAkhir = now.unixtime() - timeAwal;
      umurSekarang = timeAkhir / 86400;
    }
    EEPROM.write(address, umurSekarang);
    lcd.setCursor(1, 0);
    lcd.print("Umur: " + String(umurSekarang) + " Hari");
    lcd.setCursor(1, 1);
    lcd.print("Fasa Tanam: " + String(fasa));
    lcd.setCursor(1, 2);
    lcd.print("+PPM: " + String(ppmPenambahan));
    lcd.setCursor(1, 3);
    lcd.print("Time Dosing: " + String(onDosing / 1000) + " s");
    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      dis = 13;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 14) {
      lcd.clear();
      lockMenu = 0;
      dis = 13;
    }
  }

  if (dis == 15) {
    DateTime now = rtc.now();
    lcd.setCursor(2, 0);
    lcd.print("Setup Waktu Tanam");
    lcd.setCursor(1, 1);
    lcd.print("Tahun");
    lcd.setCursor(1, 2);
    lcd.print("Bulan");
    lcd.setCursor(1, 3);
    lcd.print("Tanggal");
    lcd.setCursor(11, 1);
    lcd.print("Jam");
    lcd.setCursor(11, 2);
    lcd.print("Menit");
    lcd.setCursor(11, 3);
    lcd.print("Detik");

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      mode3++;
      if (mode3 > 5) {
        mode3 = 0;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      mode3--;
      if (mode3 < 0) {
        mode3 = 5;
      }
    }

    switch (mode3) {
      case 0 : lcd.setCursor(0, 1); lcd.print(">"); break;
      case 1 : lcd.setCursor(0, 2); lcd.print(">"); break;
      case 2 : lcd.setCursor(0, 3); lcd.print(">"); break;
      case 3 : lcd.setCursor(10, 1); lcd.print(">"); break;
      case 4 : lcd.setCursor(10, 2); lcd.print(">"); break;
      case 5 : lcd.setCursor(10, 3); lcd.print(">"); break;
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1 && mode3 == 0) { //masuk ke setup tahun
      lockOk = 0;
      DateTime now = rtc.now();
      inTahun1 = now.year();
      dis = 16;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode3 == 1) { //masuk ke setup bulan
      lockOk = 0;
      DateTime now = rtc.now();
      inBulan1 = now.month();
      dis = 17;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode3 == 2) { //masuk ke setup tanggal
      lockOk = 0;
      DateTime now = rtc.now();
      inTanggal1 = now.day();
      dis = 18;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode3 == 3) { //masuk ke setup jam
      lockOk = 0;
      DateTime now = rtc.now();
      inJam1 = now.hour();
      dis = 19;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode3 == 4) { //masuk ke setup menit
      lockOk = 0;
      DateTime now = rtc.now();
      inMenit1 = now.minute();
      dis = 20;
      lcd.clear();
    }
    if (Ok != 0 && lockOk == 1 && mode3 == 5) { //masuk ke setup detik
      lockOk = 0;
      DateTime now = rtc.now();
      inDetik1 = now.second();
      dis = 21;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 15) {
      lcd.clear();
      lockMenu = 0;
      dis = 13;
    }
  }

  /*--------------------------Sub Menu Setup Tahun-------------------------------------------------------------*/
  if (dis == 16) {
    lcd.setCursor(0, 0);
    lcd.print("Tahun: " + String(inTahun1));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inTahun1++;
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inTahun1--;
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveTahun1 = inTahun1;
      mode3 = 1;
      dis = 15;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 16) {
      lcd.clear();
      lockMenu = 0;
      dis = 15;
    }
  }
  /*--------------------------Sub Menu Setup Bulan-------------------------------------------------------------*/
  if (dis == 17) {
    lcd.setCursor(0, 0);
    lcd.print("Bulan: " + String(inBulan1));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inBulan1++;
      if (inBulan1 > 12) {
        inBulan1 = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inBulan1--;
      if (inBulan1 < 1) {
        inBulan1 = 12;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveBulan1 = inBulan1;
      mode3 = 2;
      dis = 15;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 17) {
      lcd.clear();
      lockMenu = 0;
      dis = 15;
    }
  }
  /*--------------------------Sub Menu Setup Tanggal-------------------------------------------------------------*/
  if (dis == 18) {
    if (saveBulan1 == 1 || saveBulan1 == 3 || saveBulan1 == 5 || saveBulan1 == 7 || saveBulan1 == 8 || saveBulan1 == 10 || saveBulan1 == 12) {
      batasTanggal1 = 31;
    } else if (saveBulan1 == 4 || saveBulan1 == 6 || saveBulan1 == 9 || saveBulan1 == 11) {
      batasTanggal1 = 30;
    } else if (saveBulan == 2) {
      batasTanggal1 = 28;
    }

    lcd.setCursor(0, 0);
    lcd.print("Tanggal: " + String(inTanggal1));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inTanggal1++;
      if (inTanggal1 > batasTanggal1) {
        inTanggal1 = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inTanggal1--;
      if (inTanggal1 < 1) {
        inTanggal1 = batasTanggal1;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveTanggal1 = inTanggal1;
      mode3 = 3;
      dis = 15;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 18) {
      lcd.clear();
      lockMenu = 0;
      dis = 15;
    }
  }
  /*--------------------------Sub Menu Setup Jam-------------------------------------------------------------*/
  if (dis == 19) {
    lcd.setCursor(0, 0);
    lcd.print("Jam: " + String(inJam1));

    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inJam1++;
      if (inJam1 > 60) {
        inJam1 = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inJam1--;
      if (inJam1 < 1) {
        inJam1 = 60;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveJam1 = inJam1;
      mode3 = 4;
      dis = 15;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 19) {
      lcd.clear();
      lockMenu = 0;
      dis = 15;
    }
  }
  /*--------------------------Sub Menu Setup Menit-------------------------------------------------------------*/
  if (dis == 20) {
    lcd.setCursor(0, 0);
    lcd.print("Menit: " + String(inMenit1));
    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inMenit1++;
      if (inMenit1 > 60) {
        inMenit1 = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inMenit1--;
      if (inMenit1 < 1) {
        inMenit1 = 60;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      saveMenit1 = inMenit1;
      mode3 = 5;
      dis = 15;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 20) {
      lcd.clear();
      lockMenu = 0;
      dis = 15;
    }
  }
  /*--------------------------Sub Menu Setup Detik-------------------------------------------------------------*/
  if (dis == 21) {
    lcd.setCursor(0, 0);
    lcd.print("Detik: " + String(inDetik1));
    if (Down == 0 && lockDown == 0) {
      lockDown = 1;
    }
    if (Down != 0 && lockDown == 1) {
      lcd.clear();
      lockDown = 0;
      inDetik1++;
      if (inDetik1 > 60) {
        inDetik1 = 1;
      }
    }

    if (Up == 0 && lockUp == 0) {
      lockUp = 1;
    }
    if (Up != 0 && lockUp == 1) {
      lcd.clear();
      lockUp = 0;
      inDetik1--;
      if (inDetik1 < 1) {
        inDetik1 = 60;
      }
    }

    if (Ok == 0 && lockOk == 0) {
      lockOk = 1;
    }
    if (Ok != 0 && lockOk == 1) {
      lockOk = 0;
      lockUmur = 1;
      saveDetik1 = inDetik1;
      mode3 = 0;
      stamp.setDateTime(saveTahun1, saveBulan1, saveTanggal1, saveJam1, saveMenit1, saveDetik1);
      timeAwal = stamp.getUnix();
      dis = 13;
      lcd.clear();
    }
    if (Menu == 0 && lockMenu == 0) {
      lockMenu = 1;
    }
    if (Menu == 0 && dis == 21) {
      lcd.clear();
      lockMenu = 0;
      dis = 15;
    }
  }
}
