/*byte read_pcf(int indirizzo)
{ 
  /*byte statoIOpcf;
  Wire.beginTransmission(indirizzo); // prima lettura per resettare il pcf ed attivare il piedino int
  Wire.requestFrom(indirizzo, 1);
  if (Wire.available())
    statoIOpcf = Wire.read();
  Wire.endTransmission();
  return statoIOpcf;*/
  //return pcfRele.read(indirizzo);
/*}*/

/*void setpinpcf(int indirizzo, byte pin, byte statopin)
{
	byte statoschrele = read_pcf(indirizzo);

	if (statopin == HIGH) statoschrele = statoschrele | pin;
	else statoschrele = statoschrele & ~pin;
	//pcfRele.write(pin, statoschrele);

	/*Wire.beginTransmission(indirizzo);
	Wire.write(statoschrele);
	Wire.endTransmission();*/
/*}*/


/*void statotasto()
{
  counter = !counter;
}*/


/*void leggitasto()
{ 
  if (counter)
  { 
    if (millis() - oldtasto >= 250)
    { 
      appoggio = read_pcf(keyboard);
      rstpcf = true;
      if (appoggio == sx || appoggio == dx || appoggio == dec || appoggio == inc || appoggio == ok || appoggio == esc)
      { 
        beep = 1;
        tasto = appoggio;
      }else{ counter = false;
        read_pcf(keyboard);
        tasto = 0;
        rstpcf = false;
      }
      /*switch (appoggio)
      	{	case sx:
      			Cicalino(240);
      			tasto = appoggio;
      		break;

      		case dx:
      			Cicalino(240);
      			tasto = appoggio;
      		break;

      		case dec:
      			Cicalino(220);
      			tasto = appoggio;
      		break;

      		case inc:
      			Cicalino(220);
      			tasto = appoggio;
      		break;

      		case ok:
      			Cicalino(200);
      			tasto = appoggio;
      		break;

      		case esc:
      			Cicalino(180);
      			tasto = appoggio;
      		break;

      		default:
      			counter = false;
      			read_pcf(keyboard);
      			tasto = 0;
      			rstpcf = false;
      		break;
      	}*/
 /*     oldtasto = millis();
    }
  }
  else {
    if (rstpcf)
    { 
      read_pcf(keyboard);
      tasto = 0;
      counter = false;
      rstpcf = false;
    }
  }

}
*/
