#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>

#include <Keypad_I2C.h>
//#include <PCF8574_Class.h>
#include <RtcDS3231.h>
#include <MandJTimer.h>

#include "config.h"
//#include "MJAcquariumController.h"

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

/*byte Linea1 = 0; byte Linea2 = 1; byte Linea3 = 2;*/
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
	boolean Alba;	    		//stato alba
	boolean Tramonto;	  		//stato tramonto
	byte Fading;				//Valore di Fading
	boolean StatoPower;			//Tiene lo stato dell'alimentazione della linea
	unsigned long DeltaFading;	//Usato per lo storage del intervallo in millis per l'incremento/decremento del fading
	unsigned long Tempoprec;	//Usato per lo storage di millis() durante l'esecuzione del fading
	byte NrPin;					//Contiene il numero di pin delle singole linee
	byte Powerline;				//Contiene l'indirizzo del pin del pcf che comanda la scheda relé
};

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

byte OraOnPrec, OreTotPrec, MinOnPrec, MinTotPrec, OraOffPrec, MinOffPrec,
		OreFadPrec, MinFadPrec, OreLuceMax, MinLuceMax; // Variabili per controlli variazione
byte OreTot, MinTot, nrlinea;
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
#define I2C_RELE_ADDR 0x22
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
#define RELE3_PIN 10		//Alimentazione linea 2 Luci
#define RELE4_PIN 9 		//Alimentazione linea 3 Luci
//#define I2C_RELE5_PIN 8
//#define I2C_RELE6_PIN 7
//#define I2C_RELE7_PIN 6
//#define I2C_RELE8_PIN 5
/*===============================*/
//PCF8574_Class pcfRele(I2C_RELE_ADDR);

class MJAcquariumCOntroller {
public:
#include <Wire.h>
	RtcDS3231 RTC;
	//RtcDS3231<TwoWire> RTC(Wire);
	//RtcDS3231<TwoWire> RTC;
	RtcDateTime now;
	MandJTimer t;

	DallasTemperature sensors;
	DeviceAddress Termometro[MAX_TEMP_SENSOR];

	// MJAcquariumCOntroller();
	void inizializza();
	void inizializzaClock();
	void inizializzaSensoreTemp();
	String getDate();
	float getTemp();

	void saveSettings();
	void loadSettings();

	void MJAcquariumCOntroller::GestioneLuci (byte linea);
	void setRele(uint8_t pin, uint8_t stato){digitalWrite(pin, stato);};
};

void MJAcquariumCOntroller::inizializza() {
	//this->saveSettings();
	this->loadSettings();

	keypad.begin(makeKeymap(keys));
	keypad.addEventListener(keypadEvent); //add an event listener for this keypad

	this->inizializzaClock();
	this->inizializzaSensoreTemp();
}

void MJAcquariumCOntroller::inizializzaClock() {
	//Adding time
	//this->RTC=RtcDS3231<TwoWire> Rtc(Wire);
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

void MJAcquariumCOntroller::inizializzaSensoreTemp(){
	this->sensors.setOneWire(&OneWire(ONE_WIRE_BUS));
	this->sensors.begin();

	for (int8_t i = 0; i < MAX_TEMP_SENSOR; i++) {
		if (!this->sensors.getAddress(this->Termometro[i], i))
		{
			Serial.print(TXT_UNABLE_FIND_DEVICE);
			Serial.println(i);
		}
		this->sensors.setResolution(this->Termometro[i], 9);
	}
}

float MJAcquariumCOntroller::getTemp(){
	this->sensors.requestTemperatures();
	return ((this->sensors.getTempC(this->Termometro[0]) + this->sensors.getTempC(this->Termometro[1])) / 2);
}

void MJAcquariumCOntroller::saveSettings(void) {
	byte* p = (byte*) &settings;
	for (int i = 0; i < sizeof(AcquariumSettings); i++)
		EEPROM.write(i, p[i]);
}

void MJAcquariumCOntroller::loadSettings(void) {
	byte* p = (byte*) &settings;
	for (int i = 0; i < sizeof(AcquariumSettings); i++)
		p[i] = EEPROM.read(i);
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

void MJAcquariumCOntroller::GestioneLuci (byte linea)
{
	switch (settings.Plafo[linea].Funzionamento)
	{
		case tOff:								// Se è stato impostato OFF manuale
			if (settings.Plafo[linea].Fading == 0)	// Se sono arrivato al fading minimo e sono in OFF manuale disattivo lo switch
			{
				//settings.Plafo[linea].Funzionamento = 3;
				//setpinpcf(schrele, settings.Plafo[linea].Powerline, 1);
				this->setRele(settings.Plafo[linea].Powerline, LOW);
				settings.Plafo[linea].StatoPower = false;
			}else{
				if (((millis() - settings.Plafo[linea].Tempoprec) >= 110) && (settings.Plafo[linea].Fading > 0))
				{
					settings.Plafo[linea].Tempoprec = settings.Plafo[linea].Tempoprec + 110;
					settings.Plafo[linea].Fading --;
					analogWrite(settings.Plafo[linea].NrPin, settings.Plafo[linea].Fading);
				}
			}
			break;
		case tOn:		// Se è stato impostato ON manuale
			// Se sono arrivato al fading massimo e sono in ON manuale disattivo lo switch
			if (settings.Plafo[linea].Fading == settings.Plafo[linea].MaxFading){
				//settings.Plafo[linea].Funzionamento = 3;
			}else{
				//setpinpcf(schrele, settings.Plafo[linea].Powerline, 0);		// come sopra, ma accendo.
				this->setRele(settings.Plafo[linea].Powerline, HIGH);
				settings.Plafo[linea].StatoPower = true;
				if (((millis() - settings.Plafo[linea].Tempoprec) >= 110) && (settings.Plafo[linea].Fading < settings.Plafo[linea].MaxFading))
				{
					settings.Plafo[linea].Tempoprec = settings.Plafo[linea].Tempoprec + 110;
					settings.Plafo[linea].Fading ++;
					analogWrite(settings.Plafo[linea].NrPin, settings.Plafo[linea].Fading);
				}
			}
			break;
		case tAuto:
			if (settings.Plafo[linea].Alba == true)
			{
				if ((millis() - settings.Plafo[linea].Tempoprec) >= settings.Plafo[linea].DeltaFading)
				{
					settings.Plafo[linea].Tempoprec = settings.Plafo[linea].Tempoprec + settings.Plafo[linea].DeltaFading;
					if (settings.Plafo[linea].Fading < settings.Plafo[linea].MaxFading)
					{
						settings.Plafo[linea].Fading += 1;
						analogWrite(settings.Plafo[linea].NrPin, settings.Plafo[linea].Fading);
					}else settings.Plafo[linea].Alba = false;
				}
			}else{
				if (settings.Plafo[linea].Tramonto == true)
				{
					if ((millis() - settings.Plafo[linea].Tempoprec) >= settings.Plafo[linea].DeltaFading)
					{
						settings.Plafo[linea].Tempoprec = settings.Plafo[linea].Tempoprec + settings.Plafo[linea].DeltaFading;
						if (settings.Plafo[linea].Fading > 0)
						{
							settings.Plafo[linea].Fading -= 1;
							analogWrite(settings.Plafo[linea].NrPin, settings.Plafo[linea].Fading);
						}else{
							settings.Plafo[linea].Tramonto = false;
							//setpinpcf(schrele, settings.Plafo[linea].Powerline, 1);
							this->setRele(settings.Plafo[linea].Powerline, LOW);
							settings.Plafo[linea].StatoPower = false;
						}
					}
				}else{
					if ((settings.Plafo[linea].OraOn == this->now.Hour()) && (settings.Plafo[linea].MinOn == this->now.Minute()) && (settings.Plafo[linea].Alba == false))
					{
						settings.Plafo[linea].Alba = true;
						settings.Plafo[linea].Tempoprec = millis();
						settings.Plafo[linea].Fading = 0;
						//setpinpcf(schrele, settings.Plafo[linea].Powerline, 0);
						this->setRele(settings.Plafo[linea].Powerline, HIGH);
						settings.Plafo[linea].StatoPower = true;
					}else{
						if ((settings.Plafo[linea].OraIT == this->now.Hour()) && (settings.Plafo[linea].MinIT == this->now.Minute()) && (settings.Plafo[linea].Tramonto == false))
						{
							settings.Plafo[linea].Tramonto = true;
							settings.Plafo[linea].Tempoprec = millis();
							settings.Plafo[linea].Fading = settings.Plafo[linea].MaxFading;
						}
					}
				}
			}
			break;
	}
}

int8_t timerPrintStandby=0,
	   timerLCDbacklight=0,
	   timerCheck=0;

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
			timerPrintStandby=mjAcquariumController.t.every(1, standby);
		}
		standby();
	}
}

/**
 * stampa lo schermo principale
*/
void standby()
{
	if (menu==tHome)
	{
		//lcd.clear();
		// prima riga
		lcd.setCursor(1, 0);
		lcd.print(TXT_APP_NAME);
		// seconda riga
		lcd.setCursor(0, 1);
		lcd.print(mjAcquariumController.getDate());
		// terza riga
		lcd.setCursor(0, 2);
		lcd.print(F("T: "));
		lcd.print(mjAcquariumController.getTemp());
		lcd.write(0b011011111);
		lcd.setCursor(12, 2);
		lcd.print(F("pH:0,0")); // funzione non ancora implementata stampo una riga con valori finti
		// quarta riga
		lcd.setCursor(0, 3);
		lcd.print(F("LMed:"));
		int luxmed =(((settings.Plafo[0].Fading + settings.Plafo[1].Fading + settings.Plafo[2].Fading) / 3) * 100) / 255;
		lcd.print(luxmed);
		lcd.print(*TXT_PERCENTUALE);
		lcd.setCursor(12, 3);
		lcd.print(F("Cond:000")); // funzione non ancora implementata stampo una riga con valori finti
	}
}

/**
 * evento da eseguire dopo  settings.lcdBacklightTime secondi
 * spegne schermo e ritorno home
 */
void timerDoLCDbacklight() {
	if (menu != tHome) {
		menu=tHome;
		initfunc=true;
		//standby();
	}
	//lcd.noBacklight();
}

void timerDoCheck(){
	MantenimentoTempAcqua();
}

void salva(){
	if (initfunc == true)
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(F("Salvato"));
		initfunc=false;
	}
}

void carica(){
	if (initfunc == true)
	{
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print(F("Caricato"));
		initfunc=false;
	}
}

void setup() {
	Wire.begin();
	Serial.begin(BAUD_RATE);

	pinMode(RELE1_PIN, OUTPUT);
	pinMode(RELE2_PIN, OUTPUT);
	pinMode(RELE3_PIN, OUTPUT);
	pinMode(RELE4_PIN, OUTPUT);

	digitalWrite(RELE1_PIN,LOW);
	digitalWrite(RELE2_PIN,LOW);
	digitalWrite(RELE3_PIN,LOW);
	digitalWrite(RELE4_PIN,LOW);

	pinMode(buzzer, OUTPUT);
	pinMode(PIN_PWM_LINEA_LUCI_1, OUTPUT); // PWM linea luci 1
	pinMode(PIN_PWM_LINEA_LUCI_2, OUTPUT); // PWM linea luci 2
	pinMode(PIN_PWM_LINEA_LUCI_3, OUTPUT); // PWM linea luci 3

	lcd.begin(20, 4);					// inizializzazione del display
	lcd.createChar(0, frecciaalto); // creo la freccia verso l'alto usando i dati del vettore frecciaalto
	lcd.backlight();

	settings.Plafo[0].Powerline = RELE2_PIN;
	settings.Plafo[1].Powerline = RELE3_PIN;
	settings.Plafo[2].Powerline = RELE4_PIN;

	settings.Plafo[0].NrPin = PIN_PWM_LINEA_LUCI_1;
	settings.Plafo[1].NrPin = PIN_PWM_LINEA_LUCI_2;
	settings.Plafo[2].NrPin = PIN_PWM_LINEA_LUCI_3;

	//Tempmod = true;
	menu = tHome;
	conferma = true;

	mjAcquariumController.t.startTimer();
	mjAcquariumController.inizializza();
	//timerPrintStandby=mjAcquariumController.t.every(1, standby);
	timerLCDbacklight = mjAcquariumController.t.every(settings.lcdBacklightTime, timerDoLCDbacklight);
	//timerLCDbacklight = mjAcquariumController.t.every(60, timerDoLCDbacklight);
	timerCheck=mjAcquariumController.t.every(2, timerDoCheck);

	for (byte i = 0; i <= 2; i++) {
		Statoluci(i);
	}

	alrmsonoro = true;
	// attivo watchdog 8s
	wdt_enable(WDTO_8S);
}

void loop() {
	keypad.getKey();
	mjAcquariumController.now = mjAcquariumController.RTC.GetDateTime();

	Cicalino();
	mjAcquariumController.GestioneLuci(tLinea1);
	mjAcquariumController.GestioneLuci(tLinea2);
	mjAcquariumController.GestioneLuci(tLinea3);
	//MantenimentoTempAcqua();

	switch (menu) {
		case tHome:
			FunzionamentoNormale();
			break;
		case tImp:
			//mjAcquariumController.t.stop(timerPrintStandby);
			switch (Menuprincipale) {
			case 0: Scorrimenu(Menuprincipale, 6, VociMenuPrincipale); break;
			case 1: ImpostaData(); break;
			case 2:
				//Impostazioni = false;
				MenuLuci = 0;
				stampato = false;
				avvio = true;
				//Luci = true;
				menu = tLuci;
				break;
			case 3: Impostatempacqua(); break;
			case 4: InfoLuci(); break;
			case 5:
				mjAcquariumController.saveSettings();
				salva();
				break;
			case 6:
				mjAcquariumController.loadSettings();
				carica();
				break;
			}
			break;
		case tLuci:
			switch (MenuLuci) {
			case 0: Scorrimenu(MenuLuci, 4, VociMenuluci);break;
			case 1: ImpDatiFotoperiodo(tLinea1);break;
			case 2: ImpDatiFotoperiodo(tLinea2);break;
			case 3: ImpDatiFotoperiodo(tLinea3);break;
			case 4: ImpostaFunzLinee();break;
			}
			break;
	}

	tasto = tNull;
	mjAcquariumController.t.update();
	// reset il  watchdog
	wdt_reset();
}

// stampa0 ???
String printDigit(int digits) {
	String temp = "";
	if (digits < 10) temp += "0";
	temp += digits;
	return temp;
}

void keypadEvent(KeypadEvent eKey) {
	switch (keypad.getState()) {
		case PRESSED:
			lcd.backlight();
			mjAcquariumController.t.stop(timerLCDbacklight);
			timerLCDbacklight = mjAcquariumController.t.every(settings.lcdBacklightTime,timerDoLCDbacklight);

			switch (eKey) {
				case '#': tasto = tok;break;
				case '*':
					tasto = tesc;
					menu=tHome;
					initfunc = true;
					Serial.println(eKey);
					break;
				case 'A': break;
				case 'B': break;
				case '6': tasto = tdx; break;
				case '4': tasto = tsx; break;
				case '2': tasto = tdec; break;
				case '8': tasto = tinc; break;
				case 'D': lcd.noBacklight(); break;
				default: break;
			}
	}
}
