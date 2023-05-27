
#include <Arduino.h>

#include <LiquidCrystal.h> //wlaczenie biblioteki

LiquidCrystal lcd(10, 9, 8, 7, 6, 5); // deklaracja pinów wyswietlacza polaczonych z Arduino

#define clk 2
#define dat 3

boolean data[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 24 bit boolean array for storing the received data
// to jest pierwsza cyfra absolutna (chyba zawsze +)
boolean data_r[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// relatywna druga
boolean mm_in = 0; // 0 for mm, 1 for inch

int received_bit_location = 0;
float measured_value = 0.0;
void process_data();
long last_received = 0;
int offset = 100; // offset między ramkami
char bitsy = 'a';

void clk_ISR()
{
  if (millis() - last_received > offset)
  {
    received_bit_location = 0;
  }

  if (received_bit_location <= 23)
  {
    data[received_bit_location] = !digitalRead(dat); // Wykrzyknik (!) inverting the received data due to the hardware circuit level shifting
    received_bit_location++;
  }

  if (received_bit_location == 23)
  {
    received_bit_location = 0;
    process_data();
  }

  last_received = millis();
}

void setup()
{

  pinMode(clk, INPUT);
  pinMode(dat, INPUT);
  attachInterrupt(digitalPinToInterrupt(clk), clk_ISR, FALLING);
  lcd.begin(16, 2); // inicjalizacja wyswitlacza
  //   lcd.setCursor(0, 0); // ustawienie kursora pierwszej linii
  //   lcd.print("X: ");    // wyswietlenie tekstu
  //   lcd.setCursor(0, 1); // ustawienie kursora pierwszej linii
  //   lcd.print("Y: ");    // wyswietlenie tekstu
  //
  Serial.begin(115200);
}

void process_data()
{

  measured_value = 0.0;
  for (int x = 0; x <= 47; x++)
  {
  }

  if (data[23] == 0)
  { // if it's in the mm mode
    mm_in = 0;
    // converting binary to decimal value
    for (int i = 1; i <= 15; i++)
    {
      measured_value += data[i] * pow(2, i) / 100.0;
    }
    if (data[20] == 1)
    {
      measured_value = measured_value * -1; // add the negative sign if it exists
    }
  }

  if (data[23] == 1)
  { // if it's in the inch mode
    mm_in = 1;
    // converting binary to decimal value
    for (int i = 1; i <= 19; i++)
    {
      measured_value += data[i] * pow(2, (i - 1)) / 1000.0;
    }
    if (data[0] == 1)
    {
      measured_value += 0.0005; // add the 0.5 mil sign if it exists
    }
    if (data[20] == 1)
    {
      measured_value = measured_value * -1; // add the negative sign if it exists
    }
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (mm_in == 1)
  { // if it's in the inch mode
    // lcd.setCursor(3, 0);
    // lcd.print(String(measured_value, 4));
  }
  else
  {
    // lcd.setCursor(3, 1);
    // lcd.print(String(measured_value, 2));
  }
  // for (int i = 3; i <= 37; i++)
  // {
  //   if (i < 19)
  //   {
  //     lcd.setCursor(i - 3, 0);
  //     lcd.print(data[i]);
  //   }
  //   else
  //   {
  //     lcd.setCursor(i - 22, 1);
  //     lcd.print(data[i]);
  //   }
  // }

  Serial.println(measured_value);
  delay(500);
}
