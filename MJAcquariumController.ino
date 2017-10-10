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

	void GestioneLuci (byte linea);
	void Cicalino();
	void MantenimentoTempAcqua();
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
			if (Plafo2[linea].Fading == 0)	// Se sono arrivato al fading minimo e sono in OFF manuale disattivo lo switch
			{
				//settings.Plafo[linea].Funzionamento = 3;
				//setpinpcf(schrele, settings.Plafo[linea].Powerline, 1);
				this->setRele(Plafo2[linea].Powerline, LOW);
				Plafo2[linea].StatoPower = false;
			}else{
				if (((millis() - Plafo2[linea].Tempoprec) >= 110) && (Plafo2[linea].Fading > 0))
				{
					Plafo2[linea].Tempoprec = Plafo2[linea].Tempoprec + 110;
					Plafo2[linea].Fading --;
					analogWrite(Plafo2[linea].NrPin, Plafo2[linea].Fading);
				}
			}
			break;
		case tOn:		// Se è stato impostato ON manuale
			// Se sono arrivato al fading massimo e sono in ON manuale disattivo lo switch
			if (Plafo2[linea].Fading == settings.Plafo[linea].MaxFading){
				//settings.Plafo[linea].Funzionamento = 3;
			}else{
				//setpinpcf(schrele, settings.Plafo[linea].Powerline, 0);		// come sopra, ma accendo.
				this->setRele(Plafo2[linea].Powerline, HIGH);
				Plafo2[linea].StatoPower = true;
				if (((millis() - Plafo2[linea].Tempoprec) >= 110) && (Plafo2[linea].Fading < settings.Plafo[linea].MaxFading))
				{
					Plafo2[linea].Tempoprec = Plafo2[linea].Tempoprec + 110;
					Plafo2[linea].Fading ++;
					analogWrite(Plafo2[linea].NrPin, Plafo2[linea].Fading);
				}
			}
			break;
		case tAuto:
			if (Plafo2[linea].Alba == true)
			{
				if ((millis() - Plafo2[linea].Tempoprec) >= Plafo2[linea].DeltaFading)
				{
					Plafo2[linea].Tempoprec = Plafo2[linea].Tempoprec + Plafo2[linea].DeltaFading;
					if (Plafo2[linea].Fading < settings.Plafo[linea].MaxFading)
					{
						Plafo2[linea].Fading += 1;
						analogWrite(Plafo2[linea].NrPin, Plafo2[linea].Fading);
					}else Plafo2[linea].Alba = false;
				}
			}else if (Plafo2[linea].Tramonto == true){
				if ((millis() - Plafo2[linea].Tempoprec) >= Plafo2[linea].DeltaFading)
				{
					Plafo2[linea].Tempoprec = Plafo2[linea].Tempoprec + Plafo2[linea].DeltaFading;
					if (Plafo2[linea].Fading > 0)
					{
						Plafo2[linea].Fading -= 1;
						analogWrite(Plafo2[linea].NrPin, Plafo2[linea].Fading);
					}else{
						Plafo2[linea].Tramonto = false;
						//setpinpcf(schrele, settings.Plafo[linea].Powerline, 1);
						this->setRele(Plafo2[linea].Powerline, LOW);
						Plafo2[linea].StatoPower = false;
					}
				}
			}else{
				if ( (settings.Plafo[linea].OraOn == this->now.Hour())
						&& (settings.Plafo[linea].MinOn == this->now.Minute())
						&& (Plafo2[linea].Alba == false) )
				{
					Plafo2[linea].Alba = true;
					Plafo2[linea].Tempoprec = millis();
					Plafo2[linea].Fading = 0;
					//setpinpcf(schrele, settings.Plafo[linea].Powerline, 0);
					this->setRele(Plafo2[linea].Powerline, HIGH);
					Plafo2[linea].StatoPower = true;
				}else{
					if ((settings.Plafo[linea].OraIT == this->now.Hour()) && (settings.Plafo[linea].MinIT == this->now.Minute()) && (Plafo2[linea].Tramonto == false))
					{
						Plafo2[linea].Tramonto = true;
						Plafo2[linea].Tempoprec = millis();
						Plafo2[linea].Fading = settings.Plafo[linea].MaxFading;
					}
				}
			}

			break;
	}
}

void MJAcquariumCOntroller::Cicalino()
{
	if (beep > 0 && millis() - lastCicalino > 30) {
		if (statoBeep == 0)
			digitalWrite(buzzer, HIGH);
		else {
			digitalWrite(buzzer, LOW);
			beep--;
		}
		statoBeep = !statoBeep;
		lastCicalino = millis();
	}
}

void MJAcquariumCOntroller::MantenimentoTempAcqua ()
{
	/*Serial.print("temp read:");
	Serial.print(mjAcquariumController.getTemp());
	Serial.print(" - temp setting:");
	Serial.println(settings.Tempacqua);*/

	if (this->getTemp() < settings.Tempacqua) this->setRele(RELE1_PIN, HIGH);
	else this->setRele(RELE1_PIN, LOW);

	//lcd.setCursor(3, 2);
	if ((this->getTemp() < settings.Tempacqua - settings.TempRange) || (this->getTemp() > settings.Tempacqua + settings.TempRange))
	{
		statoalrm = 1;
		if (tasto == tesc) alrmsonoro = false;

		if (millis() - tmplampprec > 1000)
		{
			tmplampprec = millis();
			statoalrm = 1 - statoalrm;
		}
		/*if (menu==tHome)
		{
			if (statoalrm == 1)
			{
				lcd.print(tmed);
				lcd.write(0b011011111);
			}

			if (statoalrm == 0) lcd.print(F("       "));
		}*/

		if (alrmsonoro == true) beep = 1;
	} else {
		if (menu==tHome)
		{
			//lcd.print(tmed);
			//lcd.write(0b011011111);
			tmplampprec = millis();
			alrmsonoro = true;
		}
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
		//standby();
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
		int luxmed =(((Plafo2[0].Fading + Plafo2[1].Fading + Plafo2[2].Fading) / 3) * 100) / 255;
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

/*
 * evento da eseguire*/
void timerDoCheck(){
	mjAcquariumController.MantenimentoTempAcqua();
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
	lcd.createChar(0, frecciaalto);		// creo la freccia verso l'alto usando i dati del vettore frecciaalto
	lcd.backlight();

	//Tempmod = true;
	menu = tHome;
	conferma = true;

	mjAcquariumController.t.startTimer();
	mjAcquariumController.inizializza();

	Plafo2[0].Powerline = RELE2_PIN;
	Plafo2[1].Powerline = RELE3_PIN;
	Plafo2[2].Powerline = RELE4_PIN;

	Plafo2[0].NrPin = PIN_PWM_LINEA_LUCI_1;
	Plafo2[1].NrPin = PIN_PWM_LINEA_LUCI_2;
	Plafo2[2].NrPin = PIN_PWM_LINEA_LUCI_3;

	timerPrintStandby=mjAcquariumController.t.every(1, standby);
	timerLCDbacklight = mjAcquariumController.t.every(settings.lcdBacklightTime, timerDoLCDbacklight);
	//timerLCDbacklight = mjAcquariumController.t.every(60, timerDoLCDbacklight);
	timerCheck=mjAcquariumController.t.every(10, timerDoCheck);

	Statoluci(tLinea1);
	Statoluci(tLinea2);
	Statoluci(tLinea3);

	alrmsonoro = true;
	// attivo watchdog 8s
	wdt_enable(WDTO_8S);
}

void loop() {
	keypad.getKey();
	mjAcquariumController.now = mjAcquariumController.RTC.GetDateTime();

	mjAcquariumController.Cicalino();
	mjAcquariumController.GestioneLuci(tLinea1);
	mjAcquariumController.GestioneLuci(tLinea2);
	mjAcquariumController.GestioneLuci(tLinea3);

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
	String temp = F("");
	if (digits < 10) temp += F("0");
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
				case 'A':
#ifdef DEBUG
					Serial.println("Fun-MaxFa-Alba-Tramo-Fading-SatPow-PowLi");
					Serial.print(settings.Plafo[0].Funzionamento);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[0].MaxFading);
					Serial.print(*&TXT_TRATTINO);
					Serial.print(settings.Plafo[0].Alba);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[0].Tramonto);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[0].Fading);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[0].StatoPower);
					Serial.print(*TXT_TRATTINO);
					Serial.println(settings.Plafo[0].Powerline);

					Serial.print(settings.Plafo[1].Funzionamento);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[1].MaxFading);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[1].Alba);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[0].Tramonto);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[1].Fading);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[1].StatoPower);
					Serial.print(*TXT_TRATTINO);
					Serial.println(settings.Plafo[1].Powerline);

					Serial.print(settings.Plafo[0].Funzionamento);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[2].MaxFading);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[2].Alba);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[0].Tramonto);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[2].Fading);
					Serial.print(*TXT_TRATTINO);
					Serial.print(settings.Plafo[2].StatoPower);
					Serial.print(*TXT_TRATTINO);
					Serial.println(settings.Plafo[2].Powerline);

/*					byte OraOn;					//Pos 1
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
						*/

#endif
					break;
				case 'B': menu = tVuoto; InfoLuci(); break;
				case '6': tasto = tdx; break;
				case '4': tasto = tsx; break;
				case '2': tasto = tdec; break;
				case '8': tasto = tinc; break;
				case 'D': lcd.noBacklight(); break;
				default: break;
			}
	}
}
