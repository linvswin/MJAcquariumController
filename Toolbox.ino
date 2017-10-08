void Cicalino()
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

/*
 *  Controllo di tutte le variabili che devono ciclare in incremento o decremento
 */
void SoglieCiclo(byte &n, byte Min, byte Max, byte op)
{
	switch (op) {
	case 1:
		if (n < Max) n = n + 1;
		else n = Min;
		break;
	case 0:
		if (n > Min) n = n - 1;
		else n = Max;
		break;
	}
}

void stampafrecce(byte pos) // Creata per evidenziare i dati in aggiornamento semplicemente due frecce consecutive
{
	for (byte i = 0; i < 19; i++)
	{
		if (i==pos){
			lcd.write((byte) 0);
			lcd.write((byte) 0);
			i+=1;
		}
		lcd.print(F(" "));
	}
}

void stampa0(int dato_tempo) {
	if (dato_tempo < 10) lcd.print(F("0"));
	lcd.print(dato_tempo);
}

void Scorrimenu(byte &Menu, byte NrVoci, char* arraymenu[]) {
	if (avvio) {
		funzionamento = 1; // Faccio partire il menu sempre dalla prima voce
		lcd.clear(); //Cancello il display e stampo il campo zero dell'array che contiene sempre l'intestazione del menu
		lcd.setCursor(0, 0);
		lcd.print(arraymenu[0]);
		lcd.setCursor(0, 2);
		lcd.write(0b01111110);
		avvio = false;
	}

	if (tasto == tinc) {
		SoglieCiclo(funzionamento, 1, NrVoci, 1);
		stampato = false;
	}

	if (tasto == tdec) {
		SoglieCiclo(funzionamento, 1, NrVoci, 0);
		stampato = false;
	}

	if (tasto == tok) {
		initfunc = true;
		Menu = funzionamento;
	}

	if (tasto == tesc) {
		//Home = true;
		menu = tHome;
		initfunc = true;
	}

	// stampo le voci n base al tasto premuto
	if (stampato == false) {
		lcd.setCursor(2, 1);
		if (funzionamento == 1)
			lcd.print(arraymenu[NrVoci]);
		else
			lcd.print(arraymenu[funzionamento - 1]);

		lcd.setCursor(1, 2);
		lcd.print(arraymenu[funzionamento]);

		lcd.setCursor(2, 3);
		if (funzionamento == NrVoci)
			lcd.print(arraymenu[1]);
		else
			lcd.print(arraymenu[funzionamento + 1]);

		stampato = true;
	}
}

