// Da decimale a binario
byte decToBcd(byte val) {
	return ((val / 10 * 16) + (val % 10));
}
// Da binario a decimale
byte bcdToDec(byte val) {
	return ((val / 16 * 10) + (val % 16));
}

/*void LetturaDataOra() {
	Wire.beginTransmission(0x68);
	 Wire.write((byte)0x00);
	 Wire.endTransmission();

	 Wire.requestFrom(0x68, 7);
	 //recupero i 7 byte relativi ai corrispondenti registri

	 secondi = bcdToDec(Wire.read() & 0x7f);
	 minuti = bcdToDec(Wire.read());
	 ora = bcdToDec(Wire.read() & 0x3f);
	 giorno_sett = Wire.read();
	 giorno = bcdToDec(Wire.read());
	 mese = bcdToDec(Wire.read());
	 anno = bcdToDec(Wire.read()) + 2000;
}*/

/*void stampadataora(byte gg, byte mm, int aa, byte hh, byte mi, byte ss)      // Creata per semplificare il codice dell'impostazione di data e ora
 {
 lcd.setCursor(0, 1);
 stampa0(gg);
 lcd.print(F("/"));
 stampa0(mm);
 lcd.print(F("/"));
 lcd.print(aa);
 lcd.print(F("  "));
 stampa0(hh);
 lcd.print(F(":"));
 stampa0(mi);
 lcd.print(F(":"));
 stampa0(ss);
 }*/

uint8_t giornimese[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // creo un'array per il controllo sull'immisione dei giorni

void ImpostaData()   // Funzione per impostazione data
{
	uint8_t giornoimp, meseimp, oraimp, minutiimp, secondiimp, datotempo;
	int annoimp;

	if (initfunc == true)//If di inizializzazazione della procedura, viene eseguita na sola volta
	{
		giornoimp = mjAcquariumController.now.Day();
		meseimp = mjAcquariumController.now.Month();
		annoimp = mjAcquariumController.now.Year();
		oraimp = mjAcquariumController.now.Hour();
		minutiimp = mjAcquariumController.now.Minute();
		initfunc = false;
		datotempo = 1;
		lcd.clear();
		lcd.setCursor(4, 0);       // stampo la schermata impostazioni
		lcd.print(TXT_IMPDATAORA);

		//int bisestile = anno % 4;
		( (annoimp % 4)==0 ? giornimese[1]=29 : giornimese[1] = 28);
		//if (bisestile == 0) giornimese[1] = 29;	// se l'anno è bisestile febbraio ha 29 giorni
	}

	//stampadataora();
	//stampadataora(giorno, mese, anno, ora, minuti, secondi);
	lcd.setCursor(0, 1);
	lcd.print(printDigit(giornoimp));
	lcd.print(*TXT_BARRA);
	lcd.print(printDigit(meseimp));
	lcd.print(*TXT_BARRA);
	lcd.print(annoimp);
	lcd.setCursor(12, 1);
	lcd.print(printDigit(oraimp));
	lcd.print(*TXT_DUEPUNTI);
	lcd.print(printDigit(minutiimp));

	if (conferma == true) {
		if (tasto == tok) {
			datotempo = 6;
			conferma = false; // disattivo questa if in modo che Il tasto OK funzioni solo con le if di case datotempo = 6
			tasto = tNull;
		}

		if (tasto == tesc) menu = tHome;
		if (tasto == tdx) SoglieCiclo(datotempo, 1, 5, 1);
		if (tasto == tsx) SoglieCiclo(datotempo, 1, 5, 0);
	}
	lcd.setCursor(0, 2);

	switch (datotempo) {
	case 1: // Imposto giorno ////////////////////////////////////////////////////////////////////////////////////
		stampafrecce(0);
		//lcd.print(F("                  "));
		//lcd.print(F("                    "));

		if (tasto == tinc) SoglieCiclo(giornoimp, 1, giornimese[mjAcquariumController.now.Month() - 1], 1);
		if (tasto == tdec) SoglieCiclo(giornoimp, 1, giornimese[mjAcquariumController.now.Month() - 1], 0);
		break; // chiude case 1 di datotempo per impostazione data

	case 2: // Imposto mese ////////////////////////////////////////////////////////////////////////////////////////
		//lcd.print(F("   "));
		stampafrecce(3);
		//lcd.print(F("               "));

		if (tasto == tinc) SoglieCiclo(meseimp, 1, 12, 1);
		if (tasto == tdec) SoglieCiclo(meseimp, 1, 12, 0);
		break;  // Chiude case 2 di dato tempo per impostazione mese

	case 3: // Imposto anno ////////////////////////////////////////////////////////////////////////////////////
		//lcd.print(F("      "));
		stampafrecce(8);

		//lcd.print(F("          "));
		if (tasto == tinc) annoimp++;
		if (tasto == tdec) annoimp--;
		break;  // Chiude case 3 di datotempo per imposstazione anno

	case 4: // Imposto ora ////////////////////////////////////////////////////////////////////////////////////////////////////////
		//lcd.print(F("            "));
		stampafrecce(12);
		//lcd.print(F("     "));

		if (tasto == tinc) SoglieCiclo(oraimp, 0, 23, 1);
		if (tasto == tdec) SoglieCiclo(oraimp, 0, 23, 0);
		break;	// Chiude case 4 di datotempo per imposstazione ora

	case 5: // Imposto minuti ///////////////////////////////////////////////////////////////////////////////////////
		//lcd.print(F("               "));
		stampafrecce(15);

		if (tasto == tinc) SoglieCiclo(minutiimp, 0, 59, 1);
		if (tasto == tdec) SoglieCiclo(minutiimp, 0, 59, 0);
		break;  // Chiude case 5 di datotempo per impostazione minuti

	case 6:
		if (conferma == false) {
			//lcd.print(F("          "));
			lcd.setCursor(4, 3);
			lcd.print(TXT_CONFERMARE);

			if (tasto == tok) {
				//mjAcquariumController.RTC.SetDateTime(compiled);

				Wire.beginTransmission(0x68);//il primo byte stabilisce il registro iniziale da scivere
				Wire.write((byte) 0x00);
				Wire.write(decToBcd(mjAcquariumController.now.Second()));
				Wire.write(decToBcd(minutiimp));
				Wire.write(decToBcd(oraimp));
				Wire.write(decToBcd(1));
				Wire.write(decToBcd(giornoimp));
				Wire.write(decToBcd(meseimp));
				Wire.write(decToBcd(annoimp - 2000));
				Wire.endTransmission();
				conferma = true;
				menu = tHome;
				initfunc = true;
			}
			if (tasto == tdx || tasto == tsx || tasto == tinc || tasto == tdec) {
				datotempo = 1;
				conferma = true;
			}
		}
		break;	// Chiude case 6 di datotempo per impostazione RTC
	}	// chiude switch dato tempo
}

