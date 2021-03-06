
/**
 * Impostazione della temperatura dell'acqua, e sua memorizzazione in memoria,
 * potendo memorizzare in memoria solo byte (interi da 0 a 255) ho stabilitto 40° come valore massimo
 * e incrementi di immissione di 0,5° in modo che quando
 * scrivo in memoria divido il valore acquisito per l'incremento in modo da ottenere l'intero da memorizzare,
 * quando invece leggo il valore, lo rimoltiplico per l'incremento per ottenere nuovamente il valore impostato
 * esempio imposto il valore a 27,5° in memoria scrivo 55 ossia 27,5/0,5
 * leggo il valore in memoria: 55, lo moltiplico per 0,5 ed ottengo 27,5 ossia il valore impostato
 * se decidessimo per un incremento di 0,25 sarebbe la stessa cosa, ma mi è sembrato eccessivo.
*/
void Impostatempacqua()
{
	if (initfunc == true) // Leggo in memoria il valore impostato e predispongo la schermata del display
	{
		//Tempacquaset = (EEPROM.read(Tempind)) * 0.5;
		floatAppoVar= settings.Tempacqua;
		//Serial.println(floatAppoVar);
		//Tempvecchia = Tempacquaset;
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print(TXT_IMP_TEMP_ACQUA);
		lcd.setCursor(0, 2);
		lcd.print(TXT_TEMP);
		initfunc = false;
		conferma = false;
	}

	lcd.setCursor(6, 2);
	//if (Tempacquaset < 10.0) lcd.print(F("0"));


	lcd.print(floatAppoVar);
	lcd.write(0b011011111);

	if (tasto == tesc)
	{
		menu=tHome;
		initfunc = true;
		conferma = false;
	}

	if (!conferma)
	{
		lcd.setCursor(0, 3);
		stampafrecce(6);

		if (tasto == tinc)
		{
			if (floatAppoVar < 40.0) floatAppoVar = floatAppoVar + 0.5;
			else beep = 3;
		}

		if (tasto == tdec)
		{
			if (floatAppoVar > 0) floatAppoVar = floatAppoVar - 0.5;
			else beep = 3;
		}

		if (tasto == tok) {
			conferma = true; // disattivo questa if in modo che Il tasto OK funzioni solo con la if di conferma definitiva
			lcd.setCursor(3, 3);
			lcd.print(TXT_CONFERMARE);
			tasto = tNull;
		}
	} else {
		if (tasto == tok)
		{
			/*if (Tempvecchia != Tempacquaset)
			{
				Tempacquaint = (Tempacquaset / 0.5);
				EEPROM.write(Tempind, Tempacquaint);
			}*/
			settings.Tempacqua=floatAppoVar;
			//Serial.println(*settings.Tempacqua);
			floatAppoVar=0.0;
			//Tempmod = true;
			menu=tHome;
			initfunc = true;
			conferma = false;
			tasto = tNull;
		}

		if ((tasto == tdx) || (tasto == tsx) || (tasto == tinc) || (tasto == tdec))
		{
			lcd.setCursor(0, 3);
			lcd.print(F("                    "));
			conferma = false;
		}
	}
}



