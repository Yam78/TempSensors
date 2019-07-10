// --- Tuodaan kirjastot projektiin
#include <OneWire.h>                  // OneWire-kirjasto lämpötila-antureiden datan lukemiseksi varten
#include <DallasTemperature.h>        // Rutiinit lämpötila-antureiden datan käsittelyä varten
#include <Wire.h>                     // i2c-kirjasto näytön ohjaamiseksi
#include <LCD.h>                      // Näytön kirjasto. Ei tarvita suoraan, mutta pakko olla LiquidCrystal_i2c -kirjastoa varten
#include <LiquidCrystal_I2C.h>        // F Malpartida's NewLiquidCrystal library
#include <Time.h>                     // Apukirjasto viiveiden käsittelemiseksi

#define ONE_WIRE_BUS 2                // Lämpötila-antureiden johto on kytketty Arduinon dataportin nastaan 2
#define TEMPERATURE_PRECISION 12      // Lämpötila-anturin luennan tarkkuus

OneWire oneWire(ONE_WIRE_BUS);        // Määritellään OneWire laitteiden "portti"
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature. 

#define I2C_ADDR    0x20              // Määritellään näytönohjaimen I2C-osoite
// --- Näytön nastat I2C-kortilla
#define BACKLIGHT_PIN  7              // Taustavalon nasta      
#define En_pin  4                     // Enable/disable -nasta
#define Rw_pin  5                     // Read/Write -nasta
#define Rs_pin  6                     // Rekisterin valintanasta
#define D4_pin  0                     // Data 4:n nasta
#define D5_pin  1                     // Data 5:n nasta
#define D6_pin  2                     // Data 6: nasta
#define D7_pin  3                     // Data 8:n nasta

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);  // Määritellään instanssi näyttöä varten. Jatkossa näyttöä ohjataan lcd-olion kautta

char c = 225;                         // Näytölle tulsotettava "ä" -merkki
  
DeviceAddress waterInTemp;            // Sisääntulevan veden lämpötila-anturin osoite
DeviceAddress waterOutTemp;           // Lähtevän veden lämpötila-anturin osoite
DeviceAddress inTemp;                 // Sisälämpötila-anturin osoite
DeviceAddress outTemp;                // Ulkolämpötila-anturin osoite
DeviceAddress saunaTemp;              // Saunan lämpötila-anturin osoite

int showDisplayNum=0;                 // Apumuuttuja lämpötilan näyttämiseksi: 0=Sisä, 1=Ulko, 2=Sauna

// ------------------------------------- ASETUKSET / INITIALISOINTI 
void setup()                          
{
  lcd.begin(20, 4);                   // Alustetaan näytön tila
  lcd.setCursor(0,0);                 // Asetetaan kursori riville 0 sarakkeeseen 0
  sensors.begin();                    // Aloitetaan sensorien käyttö
    
  // ----------------------------------- Haetaan lämpötila-anturien osoitteen. Jatkossa näitä osoitteita käytetään datan lukemiseksi. HUOM! Kommenteissa on oikeat osoitteet!  
  sensors.getAddress(waterInTemp, 0); //{ 0x28, 0xBA, 0x46, 0xD0, 0x4, 0x0, 0x0, 0x3C };    
  sensors.getAddress(waterOutTemp, 1); //{ 0x28, 0xDE, 0x17, 0xD1, 0x4, 0x0, 0x0, 0xF4 };  
  sensors.getAddress(inTemp, 2); //{ 0x28, 0x69, 0xE9, 0xD0, 0x4, 0x0, 0x0, 0xFD };  
  sensors.getAddress(outTemp, 3); //{ 0x28, 0xF9, 0x10, 0xD0, 0x4, 0x0, 0x0, 0x19 };  
  sensors.getAddress(saunaTemp, 4); //{ 0x28, 0x57, 0x2B, 0xD0, 0x4, 0x0, 0x0, 0xA2 };
  
  // ----------------------------------- Asetetaan lämpötilan lukutarkkuudet kullekin anturille
  sensors.setResolution(waterInTemp, TEMPERATURE_PRECISION);
  sensors.setResolution(waterOutTemp, TEMPERATURE_PRECISION);
  sensors.setResolution(inTemp, TEMPERATURE_PRECISION);
  sensors.setResolution(outTemp, TEMPERATURE_PRECISION);
  sensors.setResolution(saunaTemp, TEMPERATURE_PRECISION);
  
  lcd.setCursor(0,0);                 // Kursori 1. riville 1. sarakkeeseen
  delay(20);
  lcd.print("Tuleva vesi");           // Tulostetaan vakioteksti riville 1
  delay(20);
  lcd.setCursor(0,1);                 // Kursori 2. riville 1. sarakkeeseen
  delay(20);
  lcd.print("L");                     // Tulostetaan "Lähtevä vesi" (vakioteksti)
  delay(20);
  lcd.print(c);
  delay(20);
  lcd.print("htev");
  delay(20);
  lcd.print(c);
  delay(20);
  lcd.print(" vesi");
  delay(20);
  lcd.setCursor(0,2);                 // Kursori riville 3
  delay(20);
  lcd.print("--------------------");  // Tulostetaan vakioteksti
  delay(20);
}
  
// Funktio lämpötilan tulostamiseksi. Parametrit:
// deviceAddress = anturin osoite
// cpos = kursorin kohta
// row = rivi, jolle tulostetaan (0-based)
void printTemperature(DeviceAddress deviceAddress, int cpos, int row)
{
  float tempC = sensors.getTempC(deviceAddress);  // Haetaan lämpötila
  int wholes;                         // Muuttuja kokonaisia varten lämpötilasta
  if (tempC >= 0.0)                   // Jos lämpötila on vähintään 0
  {
    wholes = floor(tempC);            //   pyöristetään alaspäin
  }
  else                                // Jos lämpötila on alle nollan
  {
    wholes = ceil(tempC);             //   Pyöristetään alaspäin
  }
    
  int frac;                           // Määritellään muuttuja kymmenesosa-asteille
  frac = (tempC - wholes)*10;         // Poimitaan 1/10-osat lämpötilasta 
  if(abs(wholes)<10)                  // Jos lämpötilan absoluuttinen arvo on alle 10 (-9.9 ... 9.9)
  {
    lcd.setCursor(cpos+1,row);        // Siirretään tulostuskohtaa yhtä edemmäs, jotta lämpötilat loppuisivat samaan kohtaan
  }  
  else if(abs(wholes)>=100)           // Jos lämpötila on yli 100 
  {
    lcd.setCursor(cpos-1,row);        // Siirretään tulostuskohtaa yhtä pykälää vasemmalle, jotta lämpötilat loppuisivat samaan kohtaan
  }
  else                                // Jos lämpötila on vlillä 10...99
  {
    lcd.setCursor(cpos,row);          // Asetetaan kursori siihen kohtaan, mihin se on käsketty laittaa parametreissa
  }
  
  delay(20);
  if (tempC > 0.0)                    // Jos lämpötila on yli nollan
  {
    lcd.print("+");                   // Tulostetaan etumerkki "+". HUOM! "-" -merkki tulee automaattisesti
  }
  delay(20);
  lcd.print(wholes);                  // Tulosta kokonaiset asteet
  delay(20);
  lcd.print(".");                     // Tulosta piste
  delay(20);
  lcd.print(frac);                    // Tulosta kymmenesosat
  delay(20);
}

// ------------------------------------- Varsinainen ohjelmakoodi
void loop()
{
  sensors.requestTemperatures();      // Lue lämpötilat

  lcd.setCursor(14,0);                // Aseta kursori riville 1, merkkiin 15
  lcd.print("     ");                 // Tyhjennä tulostusalue
  delay(20);
  printTemperature(waterInTemp,14,0);    // Tulosta tulevan veden lämpötila
  delay(20);
  
  lcd.setCursor(14,1);                // Aseta kursori riville 2 merkkiin 15
  lcd.print("     ");                 // Tyhjennä tulostusalue
  delay(20);
  printTemperature(waterOutTemp, 14,1);  // Tulosta lähtevän veden lämpötila
  delay(20);

  clearRow(3);                        // Tyhjennä rivi 4
  delay(20);
  lcd.setCursor(0,3);                 // Aseta kursori rivin 4 alkuun
  delay(20);
  if(showDisplayNum==0)               // Jos tulostettava näyttö on 0
  {                                   // Tulosta sisälämpötila
    lcd.print("Sis");
    delay(20);
    lcd.print(c);
    delay(20);
    lcd.print("ll");
    delay(20);
    lcd.print(c);
    delay(20);
    printTemperature(inTemp,14,3);
    delay(20);
  }
  else if(showDisplayNum==1)         // Jos tulostettava näyttö on 1
  {                                  // Tulosta ulkolämpötila
    lcd.print("Ulkona");            
    delay(20);
    printTemperature(outTemp,14,3);
    delay(20);
  }
  else if(showDisplayNum==2)         // Jos tulostettava näyttö on 2
  {                                  // Tulosta saunan lämpötila
    lcd.print("Saunassa");
    delay(20);
    printTemperature(saunaTemp,14,3);
    delay(20);
  }
  
  delay(1000);                       // Odota sekunti

  while(second() % 5!=0)             // Odota kunnes kuluvan sekunnin jakojäännös on 0 (joka 5. sekunti vaihdetaan näyttöä -> aloitetaan varsinainen ohjelmakoodi alusta
  {
  }

  showDisplayNum++;                  // Kasvata tulostettavan näytön laskuria
  if(showDisplayNum>2) showDisplayNum = 0;  // Jos näyttö on suurempi kuin 2, aseta tulostettavaksi näytöksi 0 (aloita looppi alusta)
}

// Funktio rivin tyhjentämiseksi
// Parametrina ryhjennettävä rivi. (0-based)
void clearRow(int row)              
{
  lcd.setCursor(0,row);                // Aseta kursori näytölle
  lcd.print("                    ");   // Tulosta "tyhjää"
  lcd.setCursor(0, row);               // Aseta kursori jälleen rivin alkuun
}

