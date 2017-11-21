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

//#define DEBUG

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

#ifdef DEBUG
const char * TXT_TRATTINO="-";
#endif
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
enum tMenu {tHome, tImp, tLuci, tVuoto};
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
char* VociMenuluci[] = {"    IMPOSTA LUCI",	"Fotoperiodo L1 ", "Fotoperiodo L2 ", "Fotoperiodo L3 ", "Funz/LMax Linee" };

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

// Variabili per gestione RTC e impostazione data e ora //
uint8_t giornoimp, meseimp, oraimp, minutiimp,datotempo;
int annoimp;

byte Tempacquadec;
unsigned long Tempolettura, TempoAttuale;
unsigned long tempolettura = 5000;
unsigned long tempoletturaprec = 0;
//const float temprange = 1.5;
boolean alrmsonoro;
unsigned long tmplampprec;
byte statoalrm = 0;

// Dichiarazioni per gestione luci e fotoperiodi //
#define PIN_PWM_LINEA_LUCI_1  3
#define PIN_PWM_LINEA_LUCI_2  5
#define PIN_PWM_LINEA_LUCI_3  6

//byte Linea1 = 0; byte Linea2 = 1; byte Linea3 = 2;
enum mjLinee{tLinea1,tLinea2,tLinea3};
enum mjFunzionamento {tOff, tOn, tAuto};

struct DatiLuci	//I valori di Pos, vengono usati per determinare l'indirizzo di memoria in cui vengono registrati gli orari
{
	byte OraOn;					//Pos 1
	byte MinOn;					//Pos 2
	byte OraOff;				//Pos 3
	byte MinOff;				//Pos 4
	byte OreFad;				//Pos 5
	byte MinFad;				//Pos 6
	byte OraFA;					//Pos 7
	byte MinFA;					//Pos 8
	byte OraIT;					//Pos 9
	byte MinIT;					//Pos 10
	byte Funzionamento;			//Pos 11 0 se la linea e Off in manuale, 1 se la linea ON in manuale, 2 se la linea è in AUT fuzionamento automatico
	byte MaxFading;				//Pos 12 Contiene il valore di luminosità massima impostata
};

struct DatiLuci2
{
	boolean Alba;	    		//stato alba
	boolean Tramonto;	  		//stato tramonto
	byte Fading;				//Valore di Fading
	boolean StatoPower;			//Tiene lo stato dell'alimentazione della linea
	unsigned long DeltaFading;	//Usato per lo storage del intervallo in millis per l'incremento/decremento del fading
	unsigned long Tempoprec;	//Usato per lo storage di millis() durante l'esecuzione del fading
	byte NrPin;					//Contiene il numero di pin delle singole linee
	int8_t Powerline;			//Contiene l'indirizzo del pin del pcf che comanda la scheda relé
};

DatiLuci2 Plafo2[3];

struct AcquariumSettings{
	float Tempacqua;			//  Temperatura acqua
	float TempRange; 			//  Temparatura range
	uint8_t lcdBacklightTime;	// durata retroilluminazione
	DatiLuci Plafo[3];
} settings = {
	20.0,						//  Temperatura acqua
	2.0,						//  Temparatura delta
	60,							// durata retroilluminazione
};


byte OraOnPrec,OreTotPrec,MinOnPrec,OraOffPrec,MinOffPrec,MinTotPrec,OreLuceMax,MinLuceMax,OreFadPrec, MinFadPrec; // Variabili per controlli variazione
byte OreTot,MinTot;
//, nrlinea;
int Fadeinmin, ITinmin, Offinmin, Oninmin, FAinmin; // Variabili per calcolo inizio alba e tramonto
byte DatoFotoperiodo, TipoFotoperiodo, acquisizionedatifotoperiodo,
		Titoloimpostazionefotoperiodo, Parteimpostazione, LimitecaseInf,
		LimitecaseSup;
int Lucegiorno;
byte linea;
//byte colonna;
unsigned long Temporeale, Accensione, Spegnimento, Finealba, Iniziotramonto;

struct Dati //Creo una struttura di variabili relative ad un Tasto
{
	byte Funzionamento;
	byte MaxFading;
};

struct Dati Appoggio[3];// Serve come appoggio dei dati durante la loro impostazione,
//altrimenti il loop vedrebbe le variabili mentre cambiano
// ed impazzirebbe

/************ Sensori ************/
//#define I2C_RELE_ADDR 0x22
// pin del PCF8574 connessi ai rele
/*#define I2C_RELE1_PIN 0		//Termostato
#define I2C_RELE2_PIN 1		//Alimentazione linea 1 Luci
#define I2C_RELE3_PIN 2		//Alimentazione linea 2 Luci
#define I2C_RELE4_PIN 3		//Alimentazione linea 3 Luci
#define I2C_RELE5_PIN 4
#define I2C_RELE6_PIN 5
#define I2C_RELE7_PIN 6
#define I2C_RELE8_PIN 7*/
#define RELE1_PIN 13		//Termostato
#define RELE2_PIN 12		//Alimentazione linea 1 Luci
#define RELE3_PIN 11		//Alimentazione linea 2 Luci
#define RELE4_PIN 10 		//Alimentazione linea 3 Luci
/*===============================*/
//PCF8574_Class pcfRele(I2C_RELE_ADDR);

float floatAppoVar=0.0;

#endif /* CONFIG_H_ */
