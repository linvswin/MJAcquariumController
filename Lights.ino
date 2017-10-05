
void StampaValFading ()		//Nella funzione ImpostaFunzLinee() stampa i valori di fading in modo he siano sempre allineati nel modo giusto
{ 
  lcd.setCursor(16, 2);

  if (Appoggio[linea].MaxFading > 99)
    lcd.print(Appoggio[linea].MaxFading);
  else if ((Appoggio[linea].MaxFading > 9) && (Appoggio[linea].MaxFading < 100))
  {
    lcd.print(F(" "));
    lcd.print(Appoggio[linea].MaxFading);
  }else{
    lcd.print(F("  "));
    lcd.print(Appoggio[linea].MaxFading);
  }
}

void StampaFunzionamento()	//Nella funzione ImpostaFunzLinee() Stampa il modo di funzionamento delle linee, in particolare:
// 0 = OFF manuale		1 = ON manuale		2 = Funzionemanto automatico in base al fotoperiodo impostato
{ 
  lcd.setCursor(10, 2);
  if (Appoggio[linea].Funzionamento == 0)
    lcd.print(TXT_OFF);
  if (Appoggio[linea].Funzionamento == 1)
    lcd.print(TXT_ON);
  if (Appoggio[linea].Funzionamento == 2)
    lcd.print(TXT_AUTOMATICO);
}

void ImpostaFunzLinee() 	// Impostazione del modo di funzionamento e della luminosita massima delle singole linee
{ 
  if (initfunc == true)	// Attivazione schermata ed inizializzazione delle variabili
  { 
    for (byte linea = 0; linea <= 2; linea++)	//Carico i dati in una struct di appoggio
    { 
      IndBase = (linea * 12) + 11;
      Appoggio[linea].Funzionamento = EEPROM.read(IndBase);
      Appoggio[linea].MaxFading = EEPROM.read(IndBase + 1);
    }
    linea = 0;
    lcd.clear();	//Scrivo su display le cose fisse
    lcd.setCursor(0, 0);
    lcd.print(F("IMP. FUNZ. E LUM.MAX"));
    lcd.setCursor(0, 2);
    lcd.print(F("Linea["));
    lcd.print(linea);
    lcd.print(F("]:["));
    StampaFunzionamento();
    lcd.print(F("] ["));
    StampaValFading();
    lcd.print(F("]"));
    conferma = true;
    initfunc = false;
    Parteimpostazione = 0;
  }

  if (conferma == true)
  { 
    if (tasto == tok)
    { 
      Parteimpostazione = 3;
      conferma = false;
      tasto = tNull;
    }

    if (tasto == tdx)
      SoglieCiclo(Parteimpostazione, 0, 2, 1);

    if (tasto == tsx)
      SoglieCiclo(Parteimpostazione, 0, 2, 0);
  }

  if (tasto == tesc)
  { 
    menu=tHome;
    initfunc = true;
  }

  lcd.setCursor(6, 3);

  switch (Parteimpostazione)
  { 
    case 0:		// Scelta della linea da impostare
      lcd.print(F("^             "));
      if (tasto == tinc)
        SoglieCiclo(linea, 0, 2, 0);

      if (tasto == tdec)
        SoglieCiclo(linea, 0, 2, 1);

      lcd.setCursor(6, 2);
      lcd.print(linea + 1);
      StampaFunzionamento();
      StampaValFading();
      break;

    case 1:		// Scelta del modo di funzionamento della linea selezionata al case 0
      lcd.print(F("    ^^^      "));
      if (tasto == tinc)
        SoglieCiclo(Appoggio[linea].Funzionamento, 0, 2, 1);

      if (tasto == tdec)
        SoglieCiclo(Appoggio[linea].Funzionamento, 0, 2, 0);
      StampaFunzionamento();
      break;

    case 2:		// Impostazione della luminosità massima della linea impostata al case 0
      lcd.print(F("          ^^^"));
      if (tasto == tinc)
        SoglieCiclo(Appoggio[linea].MaxFading, 0, 255, 1);

      if (tasto == tdec)
        SoglieCiclo(Appoggio[linea].MaxFading, 0, 255, 0);

      StampaValFading();
      break;

    case 3:
      lcd.setCursor(0, 3);
      //lcd.print(F("    *CONFERMARE*    "));
      lcd.print(TXT_CONFERMARE);
      
      if (tasto == tok)
      { 
        for (byte i = 0; i <= 2; i++)	//Salvataggio dei dati solo se cambiati e caricamento degli stessi nella struct principale
        { 
          IndBase = (i * 12) + 11;
          if (Appoggio[i].MaxFading != Plafo[i].MaxFading)
          { 
            EEPROM.write(IndBase + 1, Appoggio[i].MaxFading);
            Plafo[i].MaxFading = Appoggio[i].MaxFading;
            Plafo[linea].Tempoprec = millis();
          }
          if (Appoggio[i].Funzionamento != Plafo[i].Funzionamento)
          { 
            EEPROM.write(IndBase, Appoggio[i].Funzionamento);
            Plafo[i].Funzionamento = Appoggio[i].Funzionamento;
            if (Plafo[i].Funzionamento == 2)
              Statoluci(i);
          }
        }
        menu=tHome;
        initfunc = true;
      }
      if (tasto == tdx || tasto == tsx || tasto == tinc || tasto == tdec)
      { 
        linea = 0;
        conferma = true;
      }
      break;
  }
}

int LucePiena (byte oretotaliftp, byte minutitotaliftp, byte oretotalifad, byte minutitotalifad)
{
  return int (oretotaliftp * 60 + minutitotaliftp) - int ((oretotalifad * 60 + minutitotalifad) * 2);
}

unsigned long OrarioInSecondi (byte h, byte m) //Converte l'orario in secondi (da 0 a 86399 sec)
{
  return ((unsigned long)h * 60 + (unsigned long)m) * 60;
}

// Questa funzione viene eseguita in caso di riavvio di arduino per reset, per mancanza di luce o in caso di variazione degli orari dei
// fotoperiodi, in pratica carica nella struct i dati e calcola in caso di funzionamento automatico i dati necessari all'esecuzione del
// del fading in particolare lo stato del fading al momento del ripristino, l'intevrello tra un passaggio di rampa ed un'altro, lo stato
// delle linee di alimentazione, se è in corso l'alba il tramonto, se c'è luce piena e se è tutto spento.
// Viene eseguita una sola volta ed i dati calcolati o settati vengono poi usati dalla funzione GestioneLuci () per eseguire
// semplici confronti con i millis alleggerendo così il lavoro di Arduino.
void Statoluci(	byte linea)	
{
  IndBase = linea * 12;
  Plafo[linea].Funzionamento = EEPROM.read(IndBase + 11);
  if (Plafo[linea].Funzionamento == 2)
  { 
    Plafo[linea].OraOn = EEPROM.read(IndBase + 1);
    Plafo[linea].MinOn = EEPROM.read(IndBase + 2);
    Plafo[linea].OraOff = EEPROM.read(IndBase + 3);
    Plafo[linea].MinOff = EEPROM.read(IndBase + 4);
    Plafo[linea].OreFad = EEPROM.read(IndBase + 5);
    Plafo[linea].MinFad = EEPROM.read(IndBase + 6);
    Plafo[linea].OraFA = EEPROM.read(IndBase + 7);
    Plafo[linea].MinFA = EEPROM.read(IndBase + 8);
    Plafo[linea].OraIT = EEPROM.read(IndBase + 9);
    Plafo[linea].MinIT = EEPROM.read(IndBase + 10);
    Plafo[linea].MaxFading = EEPROM.read(IndBase + 12);

    // Calcolo degli orari di avvenimento degli eventi del fotoperiodo, in funzione dgli orari impostati
    //ed in relazione al momento in cui avvengo nel corso delle 24 ore

    Temporeale = OrarioInSecondi(mjAcquariumController.now.Hour(), mjAcquariumController.now.Minute()) + (unsigned long) mjAcquariumController.now.Second();
    Accensione = OrarioInSecondi(Plafo[linea].OraOn, Plafo[linea].MinOn);
    Spegnimento = OrarioInSecondi(Plafo[linea].OraOff, Plafo[linea].MinOff);
    Finealba = OrarioInSecondi(Plafo[linea].OreFad, Plafo[linea].MinFad);
    Iniziotramonto = OrarioInSecondi(Plafo[linea].OraIT, Plafo[linea].MinIT);

    /* Trasformazione degli orari calcolati sopra, in forma di tappe orarie in cui avvengono gli eventi, ripetto al momento di accensione
    	p.e. se alba inizia alle 10,00, il fading dura 2,00 ore e lo spegnimento avviene alle 22,00 le variabili diventeranno così:
    	Accensione = 10,00			Rimane uguale
    	Finealba = 2,00				Avviene a 2,00 ore dall'accensione e corrisponde esattamente alla durata del fading ;-)
    	Iniziotramonto = 10,00		Avviene a 8,00 ore dall'accensione
    	Spegnimento = 12,00 		Avviene a 10,00 ore dall'accensione */

    if (Temporeale < Accensione)
      Temporeale = 86400 + Temporeale - Accensione;
    else
      Temporeale = Temporeale - Accensione;

    if (Spegnimento < Accensione)
      Spegnimento = 86400 + Spegnimento - Accensione;
    else
      Spegnimento = Spegnimento - Accensione;

    Iniziotramonto = Spegnimento - Finealba; // Finealba corrisponde alla durata del Fading

    // Calcolo dell'intervallo temporale tra l'incremento/decremento delle rampe durante alba/tramonto, espresso in millesimi di secondo
    Plafo[linea].DeltaFading = (OrarioInSecondi(Plafo[linea].OreFad, Plafo[linea].MinFad) * 1000) / Plafo[linea].MaxFading;

    // Inizio dei controlli per scoprire in base all'ora attuale in quale momento del fotoperiodo della linea in questione mi trovo...


    if (Temporeale > Spegnimento) // Se le luci sono spente
    { 
      Plafo[linea].Alba = false;
      Plafo[linea].Tramonto = false;
      Plafo[linea].StatoPower = false;
      analogWrite(Plafo[linea].NrPin, 0);				// Azzero il fading
      setpinpcf(schrele, Plafo[linea].Powerline, 1);	// Spengo l'alimentatore della linea
    }
    else
    { // Se l'ora attuale è all'interno del periodo di alba
      if (Temporeale < Finealba)
      { 
        Plafo[linea].Alba = true;
        Plafo[linea].Tramonto = false;
        Plafo[linea].Fading = (Temporeale * 1000) / Plafo[linea].DeltaFading; // Calcolo il valore di fading in base a Temporeale
      }
      else	// Se Temporeale è all'interno del periodo di luce piena
      { 
        if (Temporeale < Iniziotramonto)
        { 
          Plafo[linea].Alba = false;
          Plafo[linea].Tramonto = false;
          Plafo[linea].Fading = Plafo[linea].MaxFading;
        }
        else // Se temporeale è all'interno del periodo di tramonto
        { 
          Plafo[linea].Alba = false;
          Plafo[linea].Tramonto = true;
          Plafo[linea].Fading = Plafo[linea].MaxFading - (((Temporeale - Iniziotramonto) * 1000) / Plafo[linea].DeltaFading); // Calcolo il valore di fading in base a Temporeale
        }
      }
      // Comandi fissi se le luci non sono spente
      setpinpcf(schrele, Plafo[linea].Powerline, 0);			//Alimento la linea led
      analogWrite(Plafo[linea].NrPin, Plafo[linea].Fading);	//Setto la luminosità della linea in base al fading calcolato
      Plafo[linea].Tempoprec = millis();						//Valore necessario in gestione luci per scandire il fading
      Plafo[linea].StatoPower = true;							//Flag per la verifica dello stato di alimentazione della linea
    }
  }
}

void GestioneLuci (byte linea)
{ 
  switch (Plafo[linea].Funzionamento)
  { 
    case 0:								// Se è stato impostato OFF manuale
      if (Plafo[linea].Fading == 0)		// Se sono arrivato al fading minimo e sono in OFF manuale disattivo lo switch
      { 
        Plafo[linea].Funzionamento = 3;
        setpinpcf(schrele, Plafo[linea].Powerline, 1);
        Plafo[linea].StatoPower = false;
      }else{ 
        if (((millis() - Plafo[linea].Tempoprec) >= 110) && (Plafo[linea].Fading > 0))
        { 
          Plafo[linea].Tempoprec = Plafo[linea].Tempoprec + 110;
          Plafo[linea].Fading --;
          analogWrite(Plafo[linea].NrPin, Plafo[linea].Fading);
        }
      }
      break;

    case 1:								// Se è stato impostato ON manuale
      if (Plafo[linea].Fading == Plafo[linea].MaxFading)		// Se sono arrivato al fading massimo e sono in ON manuale disattivo lo switch
        Plafo[linea].Funzionamento = 3;
      else{ 
        setpinpcf(schrele, Plafo[linea].Powerline, 0);					// come sopra, ma accendo.
        Plafo[linea].StatoPower = true;
        if (((millis() - Plafo[linea].Tempoprec) >= 110) && (Plafo[linea].Fading < Plafo[linea].MaxFading))
        { 
          Plafo[linea].Tempoprec = Plafo[linea].Tempoprec + 110;
          Plafo[linea].Fading ++;
          analogWrite(Plafo[linea].NrPin, Plafo[linea].Fading);
        }
      }
      break;

    case 2:
      if (Plafo[linea].Alba == true)
      { 
        if ((millis() - Plafo[linea].Tempoprec) >= Plafo[linea].DeltaFading)
        { 
          Plafo[linea].Tempoprec = Plafo[linea].Tempoprec + Plafo[linea].DeltaFading;
          if (Plafo[linea].Fading < Plafo[linea].MaxFading)
          { 
            Plafo[linea].Fading += 1;
            analogWrite(Plafo[linea].NrPin, Plafo[linea].Fading);
          }else
            Plafo[linea].Alba = false;
        }
      }else{ 
        if (Plafo[linea].Tramonto == true)
        { 
          if ((millis() - Plafo[linea].Tempoprec) >= Plafo[linea].DeltaFading)
          { 
            Plafo[linea].Tempoprec = Plafo[linea].Tempoprec + Plafo[linea].DeltaFading;
            if (Plafo[linea].Fading > 0)
            { 
              Plafo[linea].Fading -= 1;
              analogWrite(Plafo[linea].NrPin, Plafo[linea].Fading);
            }else{ 
              Plafo[linea].Tramonto = false;
              setpinpcf(schrele, Plafo[linea].Powerline, 1);
              Plafo[linea].StatoPower = false;
            }
          }
        }else{ 
          if ((Plafo[linea].OraOn == mjAcquariumController.now.Hour()) && (Plafo[linea].MinOn == mjAcquariumController.now.Minute()) && (Plafo[linea].Alba == false))
          { 
            Plafo[linea].Alba = true;
            Plafo[linea].Tempoprec = millis();
            Plafo[linea].Fading = 0;
            setpinpcf(schrele, Plafo[linea].Powerline, 0);
            Plafo[linea].StatoPower = true;
          }else{ 
            if ((Plafo[linea].OraIT == mjAcquariumController.now.Hour()) && (Plafo[linea].MinIT == mjAcquariumController.now.Minute()) && (Plafo[linea].Tramonto == false))
            { 
              Plafo[linea].Tramonto = true;
              Plafo[linea].Tempoprec = millis();
              Plafo[linea].Fading = Plafo[linea].MaxFading;
            }
          }
        }
      }
      break;
  }
}

void InfoLuci()
{ 
  lcd.setCursor(0, 0);
  lcd.print(TXT_INFOLUCI);
  for (byte linea = 0; linea <= 2; linea++)
  { 
    lcd.setCursor(0, linea + 1);
    lcd.print(F("L"));
    lcd.print(linea + 1);
    lcd.print(TXT_DUEPUNTI);

    lcd.print( ( Plafo[linea].StatoPower == false)? TXT_OFF : TXT_ON );

    lcd.print(F(" Fad:"));
    int perc = (Plafo[linea].Fading * 100) / Plafo[linea].MaxFading;
    lcd.print(perc);
    lcd.print(TXT_PERCENTUALE);
    lcd.setCursor(17, linea + 1);

    lcd.print( (Plafo[linea].Funzionamento == 2 ? F("A") : F("M")) );
  }
  if (tasto == tesc)
  { 
    initfunc = true;
    menu=tHome;
  }
}

void TotaliFtp (byte OraOn, byte MinOn, byte OraOff, byte MinOff, byte OreFad, byte MinFad)  //Creata per controllare che tutti i dati del fotoperiodo siano congrui
{ 
  if (OraOff <= OraOn)
  { 
    OreTot = (((((24 - OraOn) + OraOff) * 60) + MinOff) - MinOn) / 60;
    if (OreTot == 24)
      OreTot = 0;
  }else
    OreTot = (((OraOff * 60) + MinOff) - ((OraOn * 60) + MinOn)) / 60;

  if (MinOn <= MinOff)
    MinTot = MinOff - MinOn;
  else
    MinTot = ((60 - MinOn) + MinOff);

  Lucegiorno = int (OreTot * 60 + MinTot) - int ((OreFad * 60 + MinFad) * 2);
}

void ImpDatiFotoperiodo (byte linea)
{ 
  if (initfunc == true)						// Con questa if salvo le variabili interessate solo la prima volta che entro nell'impostazione
  { 
    IndBase = linea * 12;				// in modo da poter poi salvare nella eprom solo i dati variati con le if di conferma
    OraOnOld = Plafo[linea].OraOn;
    MinOnOld = Plafo[linea].MinOn;
    OraOffOld = Plafo[linea].OraOff;
    MinOffOld = Plafo[linea].MinOff;
    OreFadOld = Plafo[linea].OreFad;
    MinFadOld = Plafo[linea].MinFad;

    initfunc = false;
    Titoloimpostazionefotoperiodo = 1; // il valore di questa variabile determina il titolo della schermata
    LimitecaseInf = 1;                 // Valore iniziale del case, si inizia con impostare l'accesione delle luci che conincide con l'inizio dell'alba: OraIA
    LimitecaseSup = 4;                 // si finisce inizialmente con l'impostazione della variabile dei minuti di spegnimento del fotoperiodo che coincide con i minuti di fine tramonto: MinFT
    DatoFotoperiodo = 1;
    Parteimpostazione = 5;             // la variabile parte impostazione forza il valore del case al punto in cui si chiede di confermare i dati immessi se si preme ok
    conferma = true;                   // inizialmente è 5: siamo alla conferma dell'immisione dei dati di accensioe e spegniemtno delle luci.
    lcd.clear();
  }

  lcd.setCursor(0, 0);

  if (Titoloimpostazionefotoperiodo == 1)
  { 
    lcd.print(F("IMP. FOTOPERIODO L"));
    lcd.print(linea + 1);
    lcd.setCursor(0, 1);
    lcd.print(F("Acc.   Lung.  Spg.  "));
    lcd.setCursor(0, 2);
    stampa0(Plafo[linea].OraOn);
    lcd.print(F(":"));
    stampa0(Plafo[linea].MinOn);
    lcd.print(F("  "));
    stampa0(OreTot);
    lcd.print(F(":"));
    stampa0(MinTot);
    lcd.print(F("  "));
    stampa0(Plafo[linea].OraOff);
    lcd.print(F(":"));
    stampa0(Plafo[linea].MinOff);
  }

  if (Titoloimpostazionefotoperiodo == 2)
  { 
    lcd.print(F("Imp. Dur. Fading L"));
    lcd.print(linea + 1);
    lcd.setCursor(0, 1);
    lcd.print(F("Dur.Fad.    Luce Max"));
    lcd.setCursor(0, 2);
    stampa0(Plafo[linea].OreFad);
    lcd.print(F(":"));
    stampa0(Plafo[linea].MinFad);
    lcd.print(F("        "));
    stampa0(OreLuceMax);
    lcd.print(F(":"));
    stampa0(MinLuceMax);
    lcd.print(F("  "));
  }

  if (conferma == true)
  { 
    if (tasto == tok)
    { 
      DatoFotoperiodo = Parteimpostazione;
      conferma = false;
      tasto = tNull;
    }

    if (tasto == tdx)
      SoglieCiclo(DatoFotoperiodo, LimitecaseInf, LimitecaseSup, 1);

    if (tasto == tsx)
      SoglieCiclo(DatoFotoperiodo, LimitecaseInf, LimitecaseSup, 0);
  }


  if (tasto == tesc)
    menu=tHome;

  lcd.setCursor(0, 3);

  switch (DatoFotoperiodo)
  { 
    case 1:        // immissione dell'ora di accensione, coincide con Ora Inizio Alba OraIA
      stampafrecce(0);
      lcd.print(F("                  "));

      OraOnPrec = Plafo[linea].OraOn;          // salvo OraOn iniziale
      OreTotPrec = OreTot;         // e ora totale fotoperiodo

      if (tasto == tinc)
        SoglieCiclo(Plafo[linea].OraOn, 0, 23, 1);

      if (tasto == tdec)
        SoglieCiclo(Plafo[linea].OraOn, 0, 23, 0);

      TotaliFtp (Plafo[linea].OraOn, Plafo[linea].MinOn, Plafo[linea].OraOff, Plafo[linea].MinOff, Plafo[linea].OreFad, Plafo[linea].MinFad);

      if (Lucegiorno < 0) // Se è minore di 0 i valori si sono accavallati,
      { 
        beep = 3;
        Plafo[linea].OraOn = OraOnPrec; // i periodi si sono accavallati quindi riaggiorno le variabili ai vecchi valori
        OreTot = OreTotPrec;
      }
      break;		// Chiude case 1 di DatoFotoperiodo per impostazione OraOn

    case 2:	// immissione dei minuti dell'ora di accensione, coincide con Ora Inizio Alba OraIA
      lcd.print(F("   "));
      stampafrecce(0);
      lcd.print(F("               "));

      MinOnPrec = Plafo[linea].MinOn;
      MinTotPrec = MinTot;
      OreTotPrec = OreTot;

      if (tasto == tinc)
        SoglieCiclo(Plafo[linea].MinOn, 0, 59, 1);

      if (tasto == tdec)
        SoglieCiclo(Plafo[linea].MinOn, 0, 59, 0);

      TotaliFtp (Plafo[linea].OraOn, Plafo[linea].MinOn, Plafo[linea].OraOff, Plafo[linea].MinOff, Plafo[linea].OreFad, Plafo[linea].MinFad);

      if (Lucegiorno < 0)
      { 
        beep = 3;
        Plafo[linea].MinOn = MinOnPrec;
        OreTot = OreTotPrec;
        MinTot = MinTotPrec;
      }
      break; // Chiude case 2 di DatoFotoperiodo per impostazione MinIA

    case 3:
      lcd.print(F("              "));
      stampafrecce(0);
      lcd.print(F("    "));

      OraOffPrec = Plafo[linea].OraOff;
      OreTotPrec = OreTot;

      if (tasto == tinc)
        SoglieCiclo(Plafo[linea].OraOff, 0, 23, 1);

      if (tasto == tdec)
        SoglieCiclo(Plafo[linea].OraOff, 0, 23, 0);

      TotaliFtp (Plafo[linea].OraOn, Plafo[linea].MinOn, Plafo[linea].OraOff, Plafo[linea].MinOff, Plafo[linea].OreFad, Plafo[linea].MinFad);

      if (Lucegiorno < 0)
      { 
        beep = 3;
        Plafo[linea].OraOff = OraOffPrec;
        OreTot = OreTotPrec;
      }
      break; // Chiude case 3 di DatoFotoperiodo per impostazione OraTFtp

    case 4:
      lcd.print(F("                 "));
      stampafrecce(0);

      MinOffPrec = Plafo[linea].MinOff;
      MinTotPrec = MinTot;
      OreTotPrec = OreTot;

      if (tasto == tinc)
        SoglieCiclo(Plafo[linea].MinOff, 0, 59, 1);

      if (tasto == tdec)
        SoglieCiclo(Plafo[linea].MinOff, 0, 59, 0);

      TotaliFtp (Plafo[linea].OraOn, Plafo[linea].MinOn, Plafo[linea].OraOff, Plafo[linea].MinOff, Plafo[linea].OreFad, Plafo[linea].MinFad);

      if (Lucegiorno < 0)
      { 
        beep = 3;
        Plafo[linea].MinOff = MinOffPrec;
        OreTot = OreTotPrec;
        MinTot = MinTotPrec;
      }
      break; // Chiude case 4 di DatoFotoperiodo per impostazione MinTFtp

    case 5:
      //lcd.print(F("    *CONFERMARE*    "));
      lcd.print(TXT_CONFERMARE);
      if (conferma == false)
      { 
        if (tasto == tok)		// alla seconda pressione del tasto ok aggiorno le variabili in memoria, ma solo quelle affettivamente cambiate
        { // La variazione di questi valori comporta anche il ricalcolo a secondo dei casi delle variabili di inizio
          // alba e tramonto quindi anche queste vanno ricalcolate e memorizzate. vedi ultime due if
          if (Plafo[linea].OraOn != OraOnOld)
            EEPROM.write((IndBase + 1), Plafo[linea].OraOn);

          if (Plafo[linea].MinOn != MinOnOld)
            EEPROM.write((IndBase + 2), Plafo[linea].MinOn);

          if (Plafo[linea].OraOff != OraOffOld)
            EEPROM.write((IndBase + 3), Plafo[linea].OraOff);

          if (Plafo[linea].MinOff != MinOffOld)
            EEPROM.write((IndBase + 4), Plafo[linea].MinOff);

          Statoluci(linea);
          // agggiorno le variabili che mi servono per spostare l'immisione dati alla parte riguardante l'mmissione della durata dell'alba

          Titoloimpostazionefotoperiodo = 2;
          LimitecaseInf = 6;
          LimitecaseSup = 7;
          Parteimpostazione = 8;
          DatoFotoperiodo = LimitecaseInf;
          conferma = true;
        }
        else  // se non premo ok ma uno qualsiasi dei tasti torno alla modifica dei dati
        { 
          if (tasto == tdx || tasto == tsx || tasto == tinc || tasto == tdec)
          { 
            conferma = true;
            DatoFotoperiodo = LimitecaseInf;
          }
        }
      }
      break; // Chiude case 5 di DatoFotoperiodo per registrazione dati in memoria

    case 6:
      stampafrecce(0);
      lcd.print(F("                  "));

      OreFadPrec = Plafo[linea].OreFad;

      if (tasto == tinc)
        SoglieCiclo(Plafo[linea].OreFad, 0, 23, 1);

      if (tasto == tdec)
        SoglieCiclo(Plafo[linea].OreFad, 0, 23, 0);

      Lucegiorno = LucePiena (OreTot, MinTot, Plafo[linea].OreFad, Plafo[linea].MinFad); // Calcolo l'intervallo di tempo di luce piena in minuti

      if (Lucegiorno < 0)
      { 
        beep = 3;
        Plafo[linea].OreFad = OreFadPrec;
      }else{ 
        OreLuceMax = Lucegiorno / 60;
        MinLuceMax = Lucegiorno - (OreLuceMax * 60);
      }
      break; // Chiude case 6 per impostazione Ore fading

    case 7:
      lcd.print(F("   "));
      stampafrecce(0);
      lcd.print(F("               "));

      MinFadPrec = Plafo[linea].MinFad;

      if (tasto == tinc)
        SoglieCiclo(Plafo[linea].MinFad, 0, 59, 1);

      if (tasto == tdec)
        SoglieCiclo(Plafo[linea].MinFad, 0, 59, 0);

      Lucegiorno = LucePiena (OreTot, MinTot, Plafo[linea].OreFad, Plafo[linea].MinFad); // Calcolo l'intervallo di tempo di luce piena in minuti

      if (Lucegiorno < 0)
      { 
        beep = 3;
        Plafo[linea].MinFad = MinFadPrec;
      }else{ 
        OreLuceMax = Lucegiorno / 60;
        MinLuceMax = Lucegiorno - (OreLuceMax * 60);
      }
      break; // Chiude case 7 per impostazione di minuti di fading

    case 8:
      //lcd.print(F("    *CONFERMARE*    "));
      lcd.print(TXT_CONFERMARE);
      if (conferma == false)
      { 
        if (tasto == tok)
        {
          if (Plafo[linea].OreFad != OreFadOld)
            EEPROM.write((IndBase + 5), Plafo[linea].OreFad);

          if (Plafo[linea].MinFad != MinFadOld)
            EEPROM.write((IndBase + 6), Plafo[linea].MinFad);

          Oninmin = (Plafo[linea].OraOn * 60) + Plafo[linea].MinOn;       //Calcolo l'orario di accensione in minuti
          Offinmin = (Plafo[linea].OraOff * 60) + Plafo[linea].MinOff;    //Calcolo l'orario di spegnimento in minuti
          Fadeinmin = (Plafo[linea].OreFad * 60) + Plafo[linea].MinFad;   //Calcolo la durata del fading in minuti

          //  Calcolo ora e minuti di inizio tramonto
          ITinmin = Offinmin - Fadeinmin;

          if (ITinmin < 0)					//Per calcolare l'orario di inizio tramonto, eseguo la differenza tra durata di fading e spegnimento entrambe in minuti
            ITinmin = ITinmin + 1440;		//ma bisogna tener conto degli orari dopo la mezzanotte, quindi eseguire i calcoli in base ai casi che si possono presentare

          Plafo[linea].OraIT = ITinmin / 60;
          Plafo[linea].MinIT = ITinmin - (Plafo[linea].OraIT * 60);

          //  Calcolo ora e minuti di inizio alba
          FAinmin = Oninmin + Fadeinmin;

          if (FAinmin > 1440)
            FAinmin = FAinmin - 1440;

          Plafo[linea].OraFA = FAinmin / 60;
          Plafo[linea].MinFA = FAinmin - (Plafo[linea].OraFA * 60);

          EEPROM.write((IndBase + 7), Plafo[linea].OraFA);
          EEPROM.write((IndBase + 8), Plafo[linea].MinFA);
          EEPROM.write((IndBase + 9), Plafo[linea].OraIT);
          EEPROM.write((IndBase + 10), Plafo[linea].MinIT);

          Statoluci(linea);
          conferma = true;
          menu=tHome;
          initfunc = true;
        }
      }else{ 
        if (tasto == tdx || tasto == tsx || tasto == tinc || tasto == tdec)
        { 
          conferma = true;
          DatoFotoperiodo = LimitecaseInf;
        }
      }
      break; // Chiude case 8 di DatoFotoperiodo per registrazione dati in memoria
  }
}


