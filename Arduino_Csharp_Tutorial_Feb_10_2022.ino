
//Kommunikation mit dem PC
const int ANZAHL_FELDER = 5; //Übergabe vom PC an den Arduino (hier 5 float-Variablen).
float values[ANZAHL_FELDER]; //Sie werden in ein Array gepackt.
int feldindex = 0;

int ServoWinkelMax = 170; //Servo Winkel Max
int ServoWinkelMin =  10; //Servo Winkel Min

#define LED_BLINK 9
#define SERVO    11

//###################################################################
// Arduino Nano
#define STEPPER_X_STEP_PIN 6 
#define STEPPER_X_DIR_PIN  7 

#define STEPPER_Y_STEP_PIN 4
#define STEPPER_Y_DIR_PIN  5 
 
#define MOTORS_ENABLED     8

float istPosition_X = 0;
float istPosition_Y = 0;

//BEGIN Rampenberechnung Variablen -------
int stepPause          = 8; 		 //Microseconds
const int rampenLaenge = 150.0;	 //Steps
float maxSpeed         = 150.0;	 //Microseconds Pause zwischen den Steps
float minSpeed         = 2500.0;	 //Microseconds Pause zwischen den Steps
							//Die Spanne ist hier: Bremsrampe 150µs --> 2500µs
							             //Beschleunigungsrampe 2500µs --> 150µs
int actualSpeed        = maxSpeed;//Microseconds Pause zwischen den Steps

//für miq19 Sinus-Rampe
float maxGeschwindigkeit = maxSpeed;
float minGeschwindigkeit = minSpeed;
float spanne       = minSpeed - maxSpeed;
float schrittweite = PI/2.0/rampenLaenge;
float winkel       = 0.0;

int wegZaehler; 	   	//Für die Bremsrampe

int bremsenSinus[rampenLaenge];
int beschleunigenSinus[rampenLaenge];
//END Rampenberechnung Variablen -------

int stepCount;          //für Motorsteuerung
float VerteilerKoeff_X; //für Motorsteuerung
float VerteilerKoeff_Y; //für Motorsteuerung

//---------------------------------------------------------------------------
float StepsProMM = 40.0;//1600 Steps/Umdrehung 40 Steps/mm (1/8 Schrittmodus)
//---------------------------------------------------------------------------

//bei M0=L M1=H M2=L,  800 Steps/Umdrehung 20 Steps/mm (1/4 Schrittmodus)
//bei M0=H M1=H M2=L, 1600 Steps/Umdrehung 40 Steps/mm (1/8 Schrittmodus)
//###################################################################

//----------------------- Beginn void setup() -----------------------
void setup() {
	pinMode(LED_BLINK, OUTPUT);
	pinMode(SERVO,     OUTPUT);
	
//###################################################################	
	pinMode(STEPPER_X_DIR_PIN, OUTPUT);
	pinMode(STEPPER_X_STEP_PIN,OUTPUT);
	pinMode(STEPPER_Y_DIR_PIN, OUTPUT);
	pinMode(STEPPER_Y_STEP_PIN,OUTPUT);
	
	pinMode(MOTORS_ENABLED,    OUTPUT);
	digitalWrite(MOTORS_ENABLED, HIGH); //Motoren nicht unter Spannung
	
	//Elegante, schlanke Sinus-Rampenberechnung...
	//Danke miq19 vom Arduino Forum!	
	for (unsigned int i = 0; i < rampenLaenge; ++i) 
	{
	  float wert = maxGeschwindigkeit + spanne * sin(winkel);
	  yield(); // Do (almost) nothing - für ESP D1 mini
	  bremsenSinus[i] = wert;
	  beschleunigenSinus[rampenLaenge - i - 1] = wert;
	  winkel += schrittweite;
	}	
//###################################################################

	Serial.begin(115200);
}
//------------------------- Ende void setup() -----------------------

//----------------------- Beginn void loop() ------------------------
void loop() 
{
 if (Serial.available() > 0) 
	{ 
		for (feldindex = 0; feldindex < ANZAHL_FELDER; feldindex++) 
		{
			values[feldindex] = Serial.parseFloat();
		}//-----END for (feldindex = 0
		
		switch (int(values[0]))
		{
			case 1:
				FahrtXY(float(values[1]), float(values[2]));
			break;
			
			case 3:
				blinkende_LED(int(values[1]));
			break;

			case 4:
				blinkLED_mitPause(int(values[1]), int(values[2]));
			break;
			
			case 5:
				ServoUP();
			break;
			
			case 6:
				ServoDown();
			break;
			
			case 7:
				//LED_Dimmer(int(values[1]));
			break;
			
			case 8:
				ServoPrintActualAngle();
			break;
			
			case 9:
				ServoUpdateAngles(int(values[1]), int(values[2])); 
				//values[1] = MinAngle, values[2] = MaxAngle
			break;
			
			case 10:
				printSchraegstrich();
			break;
		}//END switch
		
	}//END if (Serial.available()

   feldindex = 0;
	 
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
	values[3] = 0;
   values[4] = 0; 
}
//-------------------------- Ende void loop() -----------------------

// ------------------------- BEGIN runden() -------------------------
int runden(float number) 
{
  int x;
  x = (int) number;
  if (number- x >= 0.5){
		x += 1;
  }	
  else{
		x = (int) number;  
  }
return x;  
} // ------------------------- END runden() -------------------------

//------------------- Beginn void blinkende_LED(int x_Mal) ----------
void blinkende_LED(int x_Mal)                               //case 3:
{
	for(int i = 1; i <= x_Mal; i++)
	{
		digitalWrite(LED_BLINK, HIGH); 
		delay(500);
		digitalWrite(LED_BLINK, LOW); 
		delay(500);
	}
	Serial.print("Ich habe ");	Serial.print(x_Mal); Serial.println(" mal geblinkt mit einer"); 
	Serial.println("konstanten Pause von 500 ms.");
	Serial.println();
}
//------------------- Ende void blinkende_LED(int x_Mal) ------------

//------------------- Beginn void blinkLED_mitPause() ---------------
void blinkLED_mitPause(int x_Mal, int pausenZeit)           //case 4:
{
	for(int i = 1; i <= x_Mal; i++)
	{
		digitalWrite(LED_BLINK, HIGH); 
		delay(pausenZeit);
		digitalWrite(LED_BLINK, LOW); 
		delay(pausenZeit);
	}
	// Serial.print("Ich habe ");	Serial.print(x_Mal); Serial.println(" mal geblinkt");
	// Serial.print("bei einer Pausenzeit von ");Serial.print(pausenZeit); 
	// Serial.println(" ms."); Serial.println();
	Serial.print(";");
}
//------------------- Ende void blinkLED_mitPause() -----------------

//----------------------- Beginn ServoUP() --------------------------
void ServoUP()                                             // case 5:
{
	 for(int pos = ServoWinkelMin; pos < ServoWinkelMax; pos++) 
	 { // Servo Up, Grundeinstellung: ServoWinkelMin = 10, ServoWinkelMax = 170
		ServoMove(pos);
	 }
	 Serial.println("ok 5: ServoUP");
}
//----------------------- Ende ServoUP() ----------------------------

//----------------------- Beginn ServoDown() ------------------------
void ServoDown()                                           // case 6:
{
	for(int pos = ServoWinkelMax; pos > ServoWinkelMin; pos--) 
	{ // Servo Down, Grundeinstellung: ServoWinkelMax = 170, ServoWinkelMin = 10
		ServoMove(pos);
	}
	Serial.println("ok 6: ServoDown");
}
//----------------------- Ende ServoDown() --------------------------

//----------------------- Beginn ServoMove() ------------------------
void ServoMove(int pos)
{
	int pwm;
  //Winkel in Mikrosekunden umrechnen
  pwm = (pos * 11) + 500;
  // Servo Pin auf HIGH zum aktivieren des Servos
  digitalWrite(SERVO, HIGH);
  // Kurze Zeit warten
  delayMicroseconds(pwm);
  // Servo Pin auf LOW zum deaktivieren des servos
  digitalWrite(SERVO, LOW); 
  delay(8);            
}
//----------------------- Ende ServoMove() --------------------------

//----------------------- Beginn LED_Dimmer() -----------------------
// void LED_Dimmer(int dimmNumber)                                      //case 7:
// {
	// if (dimmNumber == 0)
	// {
		// digitalWrite(LED_BLINK, LOW);
		// goto bailout;
	// }	
	// analogWrite(LED_BLINK, dimmNumber);
	// bailout:	
	// int dummy = 0;
// }
//----------------------- Ende LED_Dimmer() -------------------------

//-------------------------------------------------------------------
void ServoPrintActualAngle()										            //case 8:
{	
	Serial.println(ServoWinkelMin);
   Serial.println(ServoWinkelMax);
	Serial.println();
}
//-----------
void ServoUpdateAngles(int MinAgle, int MaxAngle)                    //case 9:
{
	ServoWinkelMin = MinAgle;
	ServoWinkelMax = MaxAngle;	
   Serial.println(MinAgle);
   Serial.println(MaxAngle);
	Serial.println();
}
//-------------------------------------------------------------------
void printSchraegstrich()                                           //case 10:
{
	Serial.print("/");
}
//-------------------------------------------------------------------
//###################################################################
void FahrtXY(float XSteps, float YSteps) //Eingabe: Steps
{
	float	sollPosition_X = XSteps;
	float	sollPosition_Y = YSteps;

	float stepsToGo_X = 0; //die aktuellen Schritte für den X-Motor nach Positionsberechnung
	float stepsToGo_Y = 0; //die aktuellen Schritte für den Y-Motor nach Positionsberechnung

	float SummeXY; 			 //Summe von stepsToGo_X + stepsToGo_Y
	int multiplikator_X = 1; //für Verteilerkoeffizient X
	int multiplikator_Y = 1; //für Verteilerkoeffizient Y

	int counter_X  = 0; //Zählt die Steps des X-Motors
	int counter_Y  = 0; //Zählt die Steps des Y-Motors
	int wegZaehler = 1; //Zähler für die Bremsrampe
	int stepCount;		  // Zähler für die For-Schleife

	digitalWrite(MOTORS_ENABLED,LOW); //Treiber unter Spannung	

	// Richtung der Motoren bestimmen, abhängig von IstPosition und SollPosition
	//--------------------------------------------
	stepsToGo_X = abs(sollPosition_X - istPosition_X);
	 if (sollPosition_X > istPosition_X)
		 StepperX_DIR_PLUS();
	 else
		 StepperX_DIR_MINUS();
	//--------------------------------------------
   stepsToGo_Y = abs(sollPosition_Y - istPosition_Y);
	 if (sollPosition_Y > istPosition_Y)
		 StepperY_DIR_PLUS();
	 else
		 StepperY_DIR_MINUS();	
   //--------------------------------------------	
	SummeXY = (stepsToGo_X + stepsToGo_Y); //Die eigentlichen Steps, die zu fahren sind.
   //--------------------------------------------	
	if(stepsToGo_X == 0)
		VerteilerKoeff_X = 0;
	else
		VerteilerKoeff_X = SummeXY / stepsToGo_X;	
   //--------------------------------------------	
	if(stepsToGo_Y == 0)
		VerteilerKoeff_Y = 0;
	else
		VerteilerKoeff_Y = SummeXY / stepsToGo_Y;	
   //--------------------------------------------

	istPosition_X = sollPosition_X;//istPosition_X wird aktualisiert
	istPosition_Y = sollPosition_Y;//istPosition_Y wird aktualisiert
	//--------------------------------------------	
	stepCount = 0;
	multiplikator_X = 1;
	multiplikator_Y = 1;
		
	//***************** Beginn der eigentlichen Fahrt ****************	
	for( stepCount = 0; stepCount <= SummeXY; stepCount++)
	{ 
		//yield(); // Do (almost) nothing
		
		//Beginn der Beschleunigungsramperampe
		if (stepCount <= rampenLaenge)     
		{
			//actualSpeed = beschleunigenParabel[stepCount]; //für Parabel
			actualSpeed = beschleunigenSinus[stepCount]; //für Sinus 
			//actualSpeed = maxSpeed;
		}
		
		//HighSpeed Bereich
		if (stepCount > rampenLaenge && stepCount < (SummeXY - rampenLaenge))
		{  
			if (stepsToGo_X == 0 || stepsToGo_Y == 0) 
			{
				actualSpeed = maxSpeed;
			}
			
			if (stepsToGo_X != 0 && stepsToGo_Y != 0) 
			{
				actualSpeed = maxSpeed - 40;//Versuch einer Beschleunigung bei Querwegen
			}
		}
		
		//Beginn der Bremsrampe
		if (stepCount >= (SummeXY - rampenLaenge)) 
		{
			//actualSpeed = bremsenParabel[wegZaehler];  //für Parabel
			actualSpeed = bremsenSinus[wegZaehler];  //für Sinus
			wegZaehler++;
		}
		// if ((stepsToGo_X < 850) || (stepsToGo_Y < 850))
		// {
			// actualSpeed = 700;
		// }
		
		//X-Motor wird getaktet
		if (stepCount == runden(VerteilerKoeff_X * multiplikator_X))
		{		  
			StepperX_run(actualSpeed);                     
			multiplikator_X++;
			counter_X++;
		}
		
		//Y-Motor wird getaktet
		if (stepCount == runden(VerteilerKoeff_Y * multiplikator_Y))
		{		  
			StepperY_run(actualSpeed);                     
			multiplikator_Y++;
			counter_Y++;			
		}
		//-----------------------------------------	
	}//***************** Ende der eigentlichen Fahrt *****************	

	//zurücksetzen der Variablen
	multiplikator_X = 1;
	multiplikator_Y = 1;
   wegZaehler      = 1;
	counter_X = 0;	
	counter_Y = 0;

	digitalWrite(MOTORS_ENABLED,HIGH); //Treiber aktiv (HIGH = inaktiv)	   
	Serial.print(";"); 
}//-------- END FahrtXY(int XSteps, int YSteps) --------------------
//###################################################################

// ---------------------- BEGIN Stepper-Prozeduren ------------------
void StepperX_DIR_PLUS()
{
	digitalWrite(STEPPER_X_DIR_PIN, HIGH);
}
//------ 
void StepperX_DIR_MINUS()
{
	digitalWrite(STEPPER_X_DIR_PIN,  LOW);
}
//------
void StepperY_DIR_PLUS()
{
	digitalWrite(STEPPER_Y_DIR_PIN, HIGH);
}
//------
void StepperY_DIR_MINUS()
{
	digitalWrite(STEPPER_Y_DIR_PIN,  LOW);
}
//------
void StepperX_run(int Xmotorspeed)
{
	digitalWrite(STEPPER_X_STEP_PIN, HIGH);
   delayMicroseconds(stepPause);
   digitalWrite(STEPPER_X_STEP_PIN,  LOW);
   delayMicroseconds(Xmotorspeed);
}
//------
void StepperY_run(int Ymotorspeed)
{
	digitalWrite(STEPPER_Y_STEP_PIN, HIGH);
   delayMicroseconds(stepPause);
   digitalWrite(STEPPER_Y_STEP_PIN,  LOW);
   delayMicroseconds(Ymotorspeed);
}	
// ---------------------- END Stepper-Prozeduren --------------------