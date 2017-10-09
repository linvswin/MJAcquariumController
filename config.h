/*
 * config.h
 *
 *  Created on: 09 ott 2017
 *      Author: M&J
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define BAUD_RATE  9600
#define CLKDS3231

// software reset
#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

// Testi menù
#define TXT_CONFERMARE          F("*CONFERMARE*")
#define TXT_IMP_TEMP_ACQUA      F("IMP. TEMP. ACQUA")
#define TXT_TEMP                F("Temp.:")
#define TXT_APP_NAME            F("AquariumController")
#define TXT_UNABLE_FIND_DEVICE  F("Unable to find address for Device ")
#define TXT_OFF                 F("Off")
#define TXT_ON                  F("On ")
#define TXT_AUTOMATICO          F("Aut")

const char * TXT_DUEPUNTI=":";
const char * TXT_BARRA="/";
const char * TXT_SPAZIO=" ";
const char * TXT_PERCENTUALE="%";

#define TXT_INFOLUCI			F("     INFO LUCI      ")
#define TXT_IMPDATAORA			F("Imp Data/Ora")
#define TXT_IMPFOTOPERIODO		F("IMP.FOTOPERIODO L")
#define TXT_ACCLUNGSPG			F("Acc.   Lung.  Spg.  ")
#define TXT_IMPDURFADINGL		F("Imp. Dur. Fading L")
#define TXT_DURFADLUCEMAX		F("Dur.Fad.    Luce Max")
#define TXT_IMPFUNZLUMMAX		F("IMP.FUNZ.E.LUM.MAX")
//=======================================================

//======= Tastierino =========================
#define ROWS  4  // Four rows
#define COLS  4  // Four columns

const char keys[ROWS][COLS] = { // Define the Keymap
		{ '1', '2', '3', 'A' },
		{ '4', '5', '6', 'B' },
		{ '7', '8', '9', 'C' },
		{ '*', '0', '#', 'D' } };

#define I2C_ADDR_KEYPAD 0x21 //keypad
byte rowPins[ROWS] = { 3, 2, 1, 0 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 7, 6, 5, 4 }; //connect to the column pinouts of the keypad

enum Tasti { tNull, tsx, tdx, tinc, tdec, tok, tesc };
Tasti tasto;

Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2C_ADDR_KEYPAD, PCF8574);
//======================================

//===== Dichiarazioni per display ==========
#define I2C_ADDR_LCD   0x20
#define BACKLIGHT_PIN  7
#define En_pin  4
#define Rw_pin  5
#define Rs_pin  6
#define D4_pin  0
#define D5_pin  1
#define D6_pin  2
#define D7_pin  3

LiquidCrystal_I2C lcd(I2C_ADDR_LCD, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin,	D6_pin, D7_pin, BACKLIGHT_PIN, NEGATIVE);
//======================================

// Dichiarazioni per sensori di temperatura //
#define MAX_TEMP_SENSOR   2
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 9
//======================================

// Dichiarazioni per i menù e gestione degli stessi //
enum tMenu {tHome, tImp, tLuci};
tMenu menu;

// Intestazione e voci del menù principale
char* VociMenuPrincipale[] = { "    IMPOSTAZIONI",
							   "Data/Ora    ",
							   "Imposta Luci",
							   "Temperatura ",
							   "Info Luci   ",
							   "Salva       ",
							   "Carica      "};
// Intestazione e voci del menu luci
char* VociMenuluci[] = {"    IMPOSTA LUCI",	"Fotoperiodo L1 ", "Fotoperiodo L2 ", "Fotoperiodo L3 ", "Funz/LMax Linee", };

byte Menuprincipale, MenuLuci;
boolean avvio, stampato, conferma, initfunc;
byte funzionamento;

// vettore che contiene i dati per creare una freccia rivolta verso l'alto per evidenziare i dati da impostare nei vari menu
byte frecciaalto[8] = {B00100,B01110,B11111,B00000,B00000,B00000,B00000,B00000 };

//======================================

// Dichiarazioni per i segnali acustici //
#define buzzer 7
byte beep = 1;
byte statoBeep = 0; //0 = low, 1 = high;
uint32_t lastCicalino = millis();
//======================================

float floatAppoVar=0.0;

#endif /* CONFIG_H_ */
