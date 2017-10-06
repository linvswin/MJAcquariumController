#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>
#include <RtcDS3231.h>
#include <Keypad_I2C.h>
#include <PCF8574_Class.h>

#define BAUD_RATE  9600
#define CLKDS3231

// software reset
#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

#define TXT_CONFERMARE          F("*CONFERMARE*")
#define TXT_IMP_TEMP_ACQUA      F("IMP. TEMP. ACQUA")
#define TXT_TEMP                F("Temp.:")
#define TXT_APP_NAME            F(" Aquarium Controller")
#define TXT_UNABLE_FIND_DEVICE  F("Unable to find address for Device ")
#define TXT_OFF                 F("Off")
#define TXT_ON                  F("On ")
#define TXT_AUTOMATICO          F("Aut")
//#define TXT_SPAZIO              F(" ")
//#define TXT_BARRA               F("/")
//#define TXT_DUEPUNTI            F(":")

const char* TXT_DUEPUNTI=":";
const char * TXT_BARRA="/";
const char * TXT_SPAZIO=" ";

#define TXT_PERCENTUALE         F("%")
#define TXT_INFOLUCI			F("     INFO LUCI      ")
#define TXT_IMPDATAORA			F("Imp Data/Ora")
#define TXT_IMPFOTOPERIODO		F("IMP.FOTOPERIODO L")
#define TXT_ACCLUNGSPG			F("Acc.   Lung.  Spg.  ")
#define TXT_IMPDURFADINGL		F("Imp. Dur. Fading L")
#define TXT_DURFADLUCEMAX		F("Dur.Fad.    Luce Max")
#define TXT_IMPFUNZLUMMAX		F("IMP. FUNZ. E LUM.MAX")
//#include "MJAcquariumController.h"

//////////////////////////////////////////////////////////
// Variabili per gestione RTC e impostazione data e ora //
//////////////////////////////////////////////////////////
//byte secondi, minuti, ora, giorno_sett, giorno, mese;
//int anno;
//byte datotempo;

//int annoimp,
int annoold;
//boolean Home;

//////////////////////////////////////////////
// Dichiarazioni per sensori di temperatura //
//////////////////////////////////////////////
#define MAX_TEMP_SENSOR   2
#define ONE_WIRE_BUS 4
#define TEMPERATURE_PRECISION 9

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress Termometro1, Termometro2;
float t1, t2, tmed,
	 Tempacqua, Tempacquaset, Tempvecchia;
byte Tempacquaint, Tempacquadec;
byte Tempind = 37;
boolean Tempmod;
unsigned long Tempolettura, TempoAttuale;
unsigned long tempolettura = 5000;
unsigned long tempoletturaprec = 0;
const float temprange = 1.5;
boolean alrmsonoro;
unsigned long tmplampprec;
byte statoalrm = 0;

///////////////////////////////////////////////////
// Dichiarazioni per gestione luci e fotoperiodi //
///////////////////////////////////////////////////
#define PIN_PWM_LINEA_LUCI_1  3
#define PIN_PWM_LINEA_LUCI_2  5
#define PIN_PWM_LINEA_LUCI_3  6

byte Linea1 = 0;
byte Linea2 = 1;
byte Linea3 = 2;

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
	boolean Alba;	    		//stato alba
	boolean Tramonto;	  		//stato tramonto
	byte Fading;				//Valore di Fading
	boolean StatoPower;			//Tiene lo stato dell'alimentazione della linea
	unsigned long DeltaFading;	//Usato per lo storage del intervallo in millis per l'incremento/decremento del fading
	unsigned long Tempoprec;	//Usato per lo storage di millis() durante l'esecuzione del fading
	byte NrPin;					//Contiene il numero di pin delle singole linee
	byte Powerline;				//Contiene l'indirizzo del pin del pcf che comanda la scheda relé
};

struct DatiLuci Plafo[3];

//byte OraOnOld, MinOnOld, OraOffOld, MinOffOld, OreFadOld, MinFadOld;
byte OraOnPrec, OreTotPrec, MinOnPrec, MinTotPrec, OraOffPrec, MinOffPrec,
		OreFadPrec, MinFadPrec, OreLuceMax, MinLuceMax; // Variabili per controlli variazione
byte OreTot, MinTot, nrlinea;
int Fadeinmin, ITinmin, Offinmin, Oninmin, FAinmin; // Variabili per calcolo inizio alba e tramonto
byte DatoFotoperiodo, TipoFotoperiodo, acquisizionedatifotoperiodo,
		Titoloimpostazionefotoperiodo, Parteimpostazione, LimitecaseInf,
		LimitecaseSup;
int Lucegiorno;
byte linea, IndBase, colonna;
unsigned long Temporeale, Accensione, Spegnimento, Finealba, Iniziotramonto;

struct Dati //Creo una struttura di variabili relative ad un Tasto
{
	byte Funzionamento;
	byte MaxFading;
};

struct Dati Appoggio[3];// Serve come appoggio dei dati durante la loro impostazione,
//altrimenti il loop vedrebbe le variabili mentre cambiano
// ed impazzirebbe

////////////////////////////////////////////////////
// Dichiarazioni per periferiche collegate ai PCF //
////////////////////////////////////////////////////
byte riscaldatore;
//const int keyboard = 0x21; //PCF8574AP A0 VCC A1,A2 GND
const int schrele = 0x3A;  //PCF8574AP A0 GND, A1 VCC, A2 GND

// Indirizzi dei pin del PCF dedicato alla scheda relé
//#define rele1		0x1		//Termostato
//#define rele2		0x2		//Alimentazione linea 1 Luci
//#define rele3		0x4		//Alimentazione linea 2 Luci
//#define rele4		0x8		//Alimentazione linea 3 Luci

const byte rele1 = 0x1;  //Termostato
const byte rele2 = 0x2;  //Alimentazione linea 1 Luci
const byte rele3 = 0x4;  //Alimentazione linea 2 Luci
const byte rele4 = 0x8;  //Alimentazione linea 3 Luci
/*const byte rele5 = 0x10; // Per gli usi futuri
 const byte rele6 = 0x20;
 const byte rele7 = 0x40;
 const byte rele8 = 0x80;*/

byte appoggio; //, tasto;
volatile boolean counter = false;
boolean rstpcf;

enum Tasti { tNull, tsx, tdx, tinc, tdec, tok, tesc };
Tasti tasto;

/*const byte sx = 193;
 const byte dx = 194;
 const byte dec = 200;
 const byte inc = 196;
 const byte ok = 208;
 const byte esc = 224;
 */
unsigned long oldtasto;

//////////////////////////////////////////
// Dichiarazioni per i segnali acustici //
//////////////////////////////////////////
#define buzzer 7
byte beep = 1;
byte statoBeep = 0; //0 = low, 1 = high;
uint32_t lastCicalino = millis();

//////////////////////////////////////////////////////
// Dichiarazioni per i menù e gestione degli stessi //
//////////////////////////////////////////////////////
enum tMenu {
	tHome, tImp, tLuci
};
tMenu menu;
// Intestazione e voci del menù principale
char* VociMenuPrincipale[] = { "    IMPOSTAZIONI", "Data/Ora    ", "Imposta Luci", "Temperatura ", "Info Luci   " };
// Intestazione e voci del menu luci
char* VociMenuluci[] = {"    IMPOSTA LUCI",	"Fotoperiodo L1 ", "Fotoperiodo L2 ", "Fotoperiodo L3 ", "Funz/LMax Linee", };

byte Menuprincipale, MenuLuci;
//boolean Impostazioni, Luci;
boolean avvio;
boolean stampato;
byte funzionamento;
boolean conferma, initfunc;

//======================================
#define ROWS  4  // Four rows
#define COLS  4  // Four columns

const char keys[ROWS][COLS] = { // Define the Keymap
		{ '1', '2', '3', 'A' }, { '4', '5', '6', 'B' }, { '7', '8', '9', 'C' },
				{ '*', '0', '#', 'D' } };

#define I2C_ADDR_KEYPAD 0x21 //keypad
byte rowPins[ROWS] = { 3, 2, 1, 0 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 7, 6, 5, 4 }; //connect to the column pinouts of the keypad

Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS,
		I2C_ADDR_KEYPAD, PCF8574);
//======================================

///////////////////////////////
// Dichiarazioni per display //
///////////////////////////////
#define I2C_ADDR_LCD    0x20
#define BACKLIGHT_PIN  7
#define En_pin  4
#define Rw_pin  5
#define Rs_pin  6
#define D4_pin  0
#define D5_pin  1
#define D6_pin  2
#define D7_pin  3

LiquidCrystal_I2C lcd(I2C_ADDR_LCD, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin,
		D6_pin, D7_pin, BACKLIGHT_PIN, NEGATIVE);
//LiquidCrystal_I2C lcd(I2C_ADDR_LCD, 20, 4); // Imposto l'indirizzo del display a 0x38 e dichiaro 20 colonne e 4 righe

/**
 * vettore che contiene i dati per creare una freccia rivolta verso l'alto per evidenziare i dati
 * da impostare nei vari menu
 */
byte frecciaalto[8] = {
B00100,
B01110,
B11111,
B00000,
B00000,
B00000,
B00000,
B00000 };

void keypadEvent(KeypadEvent eKey) {
	//Serial.println(eKey);

#ifdef DEBUG
#ifdef DEBUG_KEY
	Serial.print(F("Tasto: "));
	Serial.println(eKey);
	lcd.setCursor(0, 2);
	lcd.print(eKey);
#endif
#endif

	switch (keypad.getState()) {
	case PRESSED:
		lcd.backlight();

		switch (eKey) {
		case '#':
			tasto = tok;
			break;
		case '*':                 //* is to reset password attempt
			tasto = tesc;
			break;
		case 'A':
			break;
		case 'B':
			//tasto = tok;
			break;
		case '6':
			tasto = tdx;
			break;
		case '4':
			tasto = tsx;
			break;
		case '2':
			tasto = tdec;
			break;
		case '8':
			Serial.println(eKey);
			tasto = tinc;
			break;
		case 'D':
			lcd.noBacklight();
			break;
		default:
			break;
		}
	}
}

class MJAcquariumCOntroller {
public:
	RtcDS3231 RTC;
	RtcDateTime now;

	// MJAcquariumCOntroller();
	void inizializza();
	void inizializzaClock();
	String getDate();
};

void MJAcquariumCOntroller::inizializza() {
	//this->saveSettings();
	//this->loadSettings();

	keypad.begin(makeKeymap(keys));
	keypad.addEventListener(keypadEvent); //add an event listener for this keypad

	this->inizializzaClock();
	//  this->inizializzaLed();
	//  this->inizializzaSensori();
}

void MJAcquariumCOntroller::inizializzaClock() {
	//Adding time
	this->RTC.Begin();
	RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
	if (!this->RTC.IsDateTimeValid())
		this->RTC.SetDateTime(compiled);

	this->now = this->RTC.GetDateTime();
	if (this->now < compiled) {
		Serial.println("RTC is older than compile time!  (Updating DateTime)");
		this->RTC.SetDateTime(compiled);
	} else if (this->now > compiled) {
		Serial.println("RTC is newer than compile time. (this is expected)");
	} else if (this->now == compiled) {
		Serial.println(
				"RTC is the same as compile time! (not expected but all is fine)");
	}
	// never assume the Rtc was last configured by you, so
	// just clear them to your needed state
	this->RTC.Enable32kHzPin(false);
	this->RTC.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

}

/**
 * funzione che formatta per stampa su LCD di data e ora
 */
String MJAcquariumCOntroller::getDate() {
	String txt = printDigit(this->now.Day()) + *TXT_BARRA;
	txt += printDigit(this->now.Month()) + *TXT_BARRA;
	txt += printDigit(this->now.Year());
	txt += *TXT_SPAZIO;
	txt += *TXT_SPAZIO;
	txt += printDigit(this->now.Hour()) + *TXT_DUEPUNTI;
	txt += printDigit(this->now.Minute()) + *TXT_DUEPUNTI;
	txt += printDigit(this->now.Second());

#ifdef  DEBUG
#ifdef DEBUG_PRINTDATA
	Serial.print(F("date: "));
	Serial.println(txt);
#endif
#endif
	return txt;
}

MJAcquariumCOntroller mjAcquariumController;

void FunzionamentoNormale()
{
	if (tasto == tok) {
		avvio = true;
		menu = tImp;
		//Luci = false;
		Menuprincipale = 0;
		//menu=tHome;
		stampato = false;
	} else {
		if (initfunc == true) {
			lcd.clear();
			initfunc = false;
		}

		lcd.setCursor(0, 0);        // posiziono il cursore
		lcd.print(TXT_APP_NAME);

		// Stampo data e ora
		//stampadataora(giorno, mese, anno, ora, minuti, secondi);
		lcd.setCursor(0, 1);
		lcd.print(mjAcquariumController.getDate());

		lcd.setCursor(0, 2);
		lcd.print(F("T: "));
		lcd.setCursor(12, 2);
		lcd.print(F("pH:0,0")); // funzione non ancora implementata stampo una riga con valori finti

		int luxmed =(((Plafo[0].Fading + Plafo[1].Fading + Plafo[2].Fading) / 3) * 100) / 255;

		lcd.setCursor(0, 3);
		lcd.print(F("LMed:"));
		lcd.print(luxmed);
		lcd.print(F("%"));

		lcd.setCursor(12, 3);
		lcd.print(F("Cond:000")); // funzione non ancora implementata stampo una riga con valori finti
	}
}

void setup() {
	Wire.begin();
	Serial.begin(BAUD_RATE);

	lcd.begin(20, 4);					// inizializzazione del display
	lcd.createChar(0, frecciaalto); // creo la freccia verso l'alto usando i dati del vettore frecciaalto
	lcd.backlight();

	pinMode(buzzer, OUTPUT);
	pinMode(PIN_PWM_LINEA_LUCI_1, OUTPUT); // PWM linea luci 1
	pinMode(PIN_PWM_LINEA_LUCI_2, OUTPUT); // PWM linea luci 2
	pinMode(PIN_PWM_LINEA_LUCI_3, OUTPUT); // PWM linea luci 3

	Plafo[0].Powerline = rele2;
	Plafo[1].Powerline = rele3;
	Plafo[2].Powerline = rele4;
	Plafo[0].NrPin = PIN_PWM_LINEA_LUCI_1;
	Plafo[1].NrPin = PIN_PWM_LINEA_LUCI_2;
	Plafo[2].NrPin = PIN_PWM_LINEA_LUCI_3;

	Tempmod = true;

	sensors.begin();

	for (int8_t i = 0; i < MAX_TEMP_SENSOR; i++) {
		Serial.print(TXT_UNABLE_FIND_DEVICE);
		Serial.println(i);
	}
	/*if (!sensors.getAddress(Termometro1, 0))
	 {
	 //Serial.println(F("Unable to find address for Device 0"));
	 Serial.print(TXT_UNABLE_FIND_DEVICE);
	 Serial.println(F("0"));
	 }
	 if (!sensors.getAddress(Termometro2, 1))
	 {
	 //Serial.println(F("Unable to find address for Device 1"));
	 Serial.print(TXT_UNABLE_FIND_DEVICE);
	 Serial.println(F("1"));
	 }*/

	sensors.setResolution(Termometro1, 9);
	sensors.setResolution(Termometro2, 9);

	menu = tHome;
	conferma = true;

	mjAcquariumController.inizializza();

	//  LetturaDataOra();
	for (byte i = 0; i <= 2; i++) {
		Statoluci(i);
		Plafo[i].MaxFading = EEPROM.read(IndBase + 12);
	}

	alrmsonoro = true;

	// attivo watchdog 8s
	wdt_enable(WDTO_8S);
}

void loop() {
	keypad.getKey();
	mjAcquariumController.now = mjAcquariumController.RTC.GetDateTime();

	Cicalino();
	//leggitasto();
	//LetturaDataOra();
	GestioneLuci(Linea1);
	GestioneLuci(Linea2);
	GestioneLuci(Linea3);
	leggitemp();
	MantenimentoTempAcqua();

	//if (Home == true)
	switch (menu) {
	case tHome:
		FunzionamentoNormale();
		break;
	case tImp:
		switch (Menuprincipale) {
		case 0:
			Scorrimenu(Menuprincipale, 4, VociMenuPrincipale);
			break;

		case 1:
			ImpostaData();
			break;

		case 2:
			//Impostazioni = false;
			MenuLuci = 0;
			stampato = false;
			avvio = true;
			//Luci = true;
			menu = tLuci;
			break;

		case 3:
			Impostatempacqua();
			break;

		case 4:
			InfoLuci();
			break;
		}

		break;
	case tLuci:
		switch (MenuLuci) {
		case 0:
			Scorrimenu(MenuLuci, 4, VociMenuluci);
			break;

		case 1:
			ImpDatiFotoperiodo(Linea1);
			break;

		case 2:
			ImpDatiFotoperiodo(Linea2);
			break;

		case 3:
			ImpDatiFotoperiodo(Linea3);
			break;

		case 4:
			ImpostaFunzLinee();
			break;
		}
		break;
	}

	tasto = tNull;
	// reset il  watchdog
	wdt_reset();
}

String printDigit(int digits) {
	String temp = "";
	if (digits < 10)
		temp += "0";
	temp += digits;
	return temp;
}

