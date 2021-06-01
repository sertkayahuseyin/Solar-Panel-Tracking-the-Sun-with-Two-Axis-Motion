#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

String veriler;
File solar_panel;
int chip_Select = BUILTIN_SDCARD;


Servo horizontal;  // Yatay Servo Motor
int servo_h = 100;                         // Yatayda ki Servo motorun Harekete Başlaaycağı konum
int servohLimitHigh = 100;                // Yatayda ki Servo motorun ulaşabileceg en ust aci degeri
int servohLimitLow = 110;                  // Yatayda ki Servo motorun ulaşabileceg en alt aci degeri
int tol = 50;
int dvert;                                // Alt ve Üst Sensörlerin Farkı
int dhoriz;                               // Sağ ve Sol Sensörlerin Farkı
int avt;
int avr;
int avd;
int avl;

const int buttonPin = 4;                   // Button Pin
const int ledPin =  5;                    // LED Buton
int buttonState = 0;                       // Buton durumu belirlendi

Servo vertical;                            // Dikey Servo

int servo_v = 90;                          // Dikeyde ki Servo motorun Harekete Başlaaycağı konum
int servovLimitHigh = 100;                 // Dikeyde ki Servo motorun ulaşabileceg en ust aci degeri
int servovLimitLow = 15;                   // Yatayda ki Servo motorun ulaşabileceg en alt aci degeri

int L_1 = A8;                              // Sol Üst LDR Sensör Pini
int R1 = A7;                               // Sağ Üst LDR Sensör Pini
int L_2 = A9;                              // Sol Alt Sensör Pini
int R2 = A6;                               // Sağ Alt Sensör Pini
float deger = A5;                            // Güneş paneli gerilim ölçümü
float referans = 3.3;
unsigned long eskiZaman = 0;
unsigned long yeniZaman;

float current;
void setup()
{
  Serial.begin(9600);

  SD.begin(BUILTIN_SDCARD);
  horizontal.attach(3);                    // Yatay Ekseni Servo Motor Pini
  vertical.attach(2);                      // Dikey Ekseni Servo Motor Pini

  horizontal.write(180);                   // Yatay Eksen 180 dereceye gönderildi.
  vertical.write(45);

  pinMode(ledPin, OUTPUT);                // LED Çıkış olarak ayarlandı.
  pinMode(buttonPin, INPUT);              //Buton giriş olarak ayarlnadı.

  setSyncProvider(getTeensy3Time);

}

void loop()
{
    if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      setTime(t);
    }
  }

  
  yeniZaman = millis();
  if (yeniZaman - eskiZaman > 1000) {

    int lt = analogRead(L_1);                 // Sol Üst LDR Sensörün Degerini "lt" değişkenine atandı.
    int rt = analogRead(R1);                  // Sağ Üst LDR Sensörün Degerini "rt" değişkenine atandı.
    int ld = analogRead(L_2);                 // Sol Alt LDR Sensörün Degerini "ld" değişkenine atandı.
    int rd = analogRead(R2);                  // Sağ Alt LDR Sensörün Degerini "rd" değişkenine atandı.
    int deger = analogRead(deger);              // Güneş paneli gerilimini oku.

    int dtime = 10;
    int tol = 50;

    int avt = (lt + rt) / 2;                  // Üst Sensör Verilerinin Ortalaması
    int avd = (ld + rd) / 2;                  // Alt Sensör Verilerinin Oratalaması
    int avl = (lt + ld) / 2;                  // Sol Sensör Verilerinin Ortalaması
    int avr = (rt + rd) / 2;                  // Sağ Sensör Verilerinin Ortalaması

    int dvert = avt - avd;                    // Alt ve Üst Sensörlerin Farkı
    int dhoriz = avl - avr;                   // Sağ ve Sol Sensörlerin Farkı


    //Serial.print(avt);
    //Serial.print(" ");
    //Serial.print(avd);
    //Serial.print(" ");
    //Serial.print(avl);
    //Serial.print(" ");
    //Serial.print(avr);
    //Serial.print("   ");
    //Serial.print(dtime);
    //Serial.print("   ");
    //Serial.print(tol);
    //Serial.println(" ");
    buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH) {
      warning();
    } else {
      // turn LED off:
      digitalWrite(ledPin, LOW);
    }

    if (-1 * tol > dvert || dvert > tol)
    {
      if (avt > avd)
      {
        servo_v = ++servo_v;
        if (servo_v > servovLimitHigh)
        {
          servo_v = servovLimitHigh;
        }
      }
      else if (avt < avd)
      {
        servo_v = --servo_v;
        if (servo_v < servovLimitLow)
        {
          servo_v = servovLimitLow;
        }
      }
      vertical.write(servo_v);
    }

    if (-1 * tol > dhoriz || dhoriz > tol)
    {
      if (avl > avr)
      {
        servo_h = --servo_h;
        if (servo_h < servohLimitLow)
        {
          servo_h = servohLimitLow;
        }
      }
      else if (avl < avr)
      {
        servo_h = ++servo_h;
        if (servo_h > servohLimitHigh)
        {
          servo_h = servohLimitHigh;
        }
      }
      else if (avl = avr)
      {

      }
      horizontal.write(servo_h);
    }
    float volt = (deger * referans) / 1023 * 2;
    digitalClockDisplay();
    veriler += " Analog Deger:";
    veriler += deger;
    veriler += " Voltaj: ";
    veriler += volt;
    veriler += "V";
    veriler += " Akım:";
    current = volt / 2.2;
    veriler += current;
    veriler += "mA";
    veriler += " Yatay konum:";
    veriler += servo_v;
    veriler += " Dikey Konum:";
    veriler += servo_h;
 
    Serial.println(veriler);
    sd_card();
    eskiZaman = yeniZaman;
    veriler = "";
  }

}
void warning() {
  horizontal.write(servo_h);                   // Yatay Eksen 180 dereceye gönderildi.
  vertical.write(servo_v);
  digitalWrite(ledPin, HIGH);
}

void sd_card()
{
  solar_panel = SD.open("Veriler.txt", FILE_WRITE);
  if (solar_panel)
  {
    solar_panel.println(veriler);
    solar_panel.close();
  }

}

void digitalClockDisplay() {
  // RTC verileri veriler stringinin içerisine atılıyor.
 veriler += " Gün:";
 veriler += day();
 veriler += " Ay:";
 veriler += month();
 veriler += " Yıl:";
 veriler += year();
 veriler += " Saat:";
 veriler += hour();
 veriler += " Dakika:";
 veriler += minute();
 veriler += " Saniye:";
 veriler += second();
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
