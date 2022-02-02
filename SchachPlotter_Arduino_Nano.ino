//https://arduino-projekte.webnode.at/arduino-debugger/
//gute Alternative zum Serial.print

//#define DEBUG_ON .... Debugger ist aktiviert
//#define DEBUG_ON ... Debugger ist deaktiviert

//#define DEBUG_ON //Debugger ist aktiviert (Definition muss vor der include-Anweisung stehen!)
//#include <MyDebug.h>

//Kommunikation mit dem PC
const int ANZAHL_FELDER = 5; //Übergabe vom PC an den Arduino (hier 5 float-Variablen).
float values[ANZAHL_FELDER]; //Sie werden in ein Array gepackt.
int feldindex = 0;

//Motorposition X/Y
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

float maxGeschwindigkeit = maxSpeed;
float minGeschwindigkeit = minSpeed;

float spanne       = minSpeed - maxSpeed;
float schrittweite = PI/2.0/rampenLaenge;
float winkel       = 0.0;

float gradKoeffizient;  //für Sinus-Rampe (obsolet)
float rampenKoeffizient;//(obsolet)
int zaehler; 			   //für die Rampenberechnung
int wegZaehler = 1; 	   //Für die Bremsrampe
//END Rampenberechnung Variablen -------

int stepCount;          //für Motorsteuerung
float VerteilerKoeff_X; //für Motorsteuerung
float VerteilerKoeff_Y; //für Motorsteuerung

// D1 mini ESP8266
// #define STEPPER_X_STEP_PIN 12 // D6
// #define STEPPER_X_DIR_PIN  13 // D7

// #define STEPPER_Y_STEP_PIN 16 // D0
// #define STEPPER_Y_DIR_PIN  14 // D5

// #define MOTORS_ENABLED     15 // D8
// #define SERVO               0 // D3
// #define MAGNET              4 // D2

// Arduino Nano
#define STEPPER_X_STEP_PIN 6 
#define STEPPER_X_DIR_PIN  7 

#define STEPPER_Y_STEP_PIN 4
#define STEPPER_Y_DIR_PIN  5 
 
#define MOTORS_ENABLED     8
#define MAGNET             9
#define SERVO              11
/*
int beschleunigenParabel[rampenLaenge];
int bremsenParabel[rampenLaenge];
*/


int bremsenSinus[rampenLaenge];
int beschleunigenSinus[rampenLaenge];


//---------------------------------------------------------------------------
float StepsProMM = 40.0;//1600 Steps/Umdrehung 40 Steps/mm (1/8 Schrittmodus)
//---------------------------------------------------------------------------

//bei M0=L M1=H M2=L,  800 Steps/Umdrehung 20 Steps/mm (1/4 Schrittmodus)
//bei M0=H M1=H M2=L, 1600 Steps/Umdrehung 40 Steps/mm (1/8 Schrittmodus)

int Park_X[] = {20, 20, 20, 20, 20, 20, 20, 20, 40, 40, 40, 40, 40, 40, 40, 40, 220, 220,
220, 220, 220, 220, 220, 220, 240, 240, 240, 240, 240, 240, 240, 240}; // Stellplätze X

int Park_Y[] = {140, 120, 100, 80, 60, 40, 20, 0, 140, 120, 100, 80, 60, 40, 20, 0, 140, 
120, 100, 80, 60, 40, 20, 0, 140, 120, 100, 80, 60, 40, 20, 0}; // Stellplätze Y

int Park_Index = 0; //Park_Index 0 bis 31
byte Modulo;
	int ServoWinkelMax = 170; //Servo Winkel Max
	int ServoWinkelMin = 10;  //Servo Winkel Min
//---------------------------------------------------------------------------
void setup() {
	pinMode(STEPPER_X_DIR_PIN, OUTPUT);
	pinMode(STEPPER_X_STEP_PIN,OUTPUT);
	pinMode(STEPPER_Y_DIR_PIN, OUTPUT);
	pinMode(STEPPER_Y_STEP_PIN,OUTPUT);
	
	pinMode(MOTORS_ENABLED,    OUTPUT);
	pinMode(SERVO,             OUTPUT);	
	pinMode(MAGNET,            OUTPUT);	
	
	digitalWrite(MOTORS_ENABLED, HIGH); //Motoren unter Spannung
	
	
	
/*
	//Mein mühsam erarbeiteter Algorithmus mit Hammer, Nagel und Kneifzange...
	rampenKoeffizient = (minSpeed - maxSpeed) / rampenLaenge;
	gradKoeffizient = 180.0 / rampenLaenge;
	
	//Sinus-Beschleunigungs- und Bremsrampe
   float x;
   int   y;
	int zaehler = 0;
	for(float i = 270; i > 90; i = i - gradKoeffizient){	 
		yield(); // Do (almost) nothing --
		x = (sin(i*PI/180) * 100);
		y = int (minSpeed - (minSpeed / 100 * x))/ 2 + maxSpeed;	
		Serial.println(y);
		beschleunigenSinus[zaehler] = y;  
		zaehler++;			
	}
	zaehler = 0;
	//Sinus-Bremsrampe
	zaehler = 0;
	for(float i = 90; i < 270; i = i + gradKoeffizient){		
		yield(); // Do (almost) nothing --  		
		x = (sin(i*PI/180)  * 100);
		y = int (minSpeed - (minSpeed / 100 * x))/ 2 + maxSpeed;	
		bremsenSinus[zaehler] = y;			
		zaehler++;
	}
	zaehler = 0;
*/	
	
	
	

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
	
	
/*
	//BEGIN Parabel-Berechnung--------------------
	rampenKoeffizient = 1.0 / rampenLaenge; //entweder 1.0 oder 2.0
	float multiplikator = 1.0;
	
	for (unsigned int i = 0; i < rampenLaenge; ++i) 
	{
		double wert = pow((rampenKoeffizient * multiplikator),2);
		//double wert = (rampenKoeffizient * multiplikator) * (rampenKoeffizient * multiplikator);
		//double wert = rampenKoeffizient * multiplikator * rampenKoeffizient * multiplikator;
		wert = minSpeed - (spanne * wert);
		int wert2 = int(wert);
		beschleunigenParabel[i] = wert2;
		bremsenParabel[rampenLaenge -i -1] = wert2;		
		multiplikator ++;
		
	}
*/	
Serial.begin(115200);
}
//---------------------------------------------------------------------------
void loop() 
{
	if (Serial.available() > 0) { //geändert. Vorher: if (Serial.available())
		for (feldindex = 0; feldindex < ANZAHL_FELDER; feldindex++) 
		{
			values[feldindex] = Serial.parseFloat();
		}//-----END for (feldindex = 0
		
		switch (int(values[0]))
		{
			case 1:			
			 FahrtXYZ(float(values[1]), float(values[2]));
			break;
		  
			case 2:
				Reset();
			break;
			
			case 3:
				ServoPickUpFigure();
			break;
			
			case 4:
				ServoPutDownFigure();
			break;
			
			case 5:
				ServoUP();
			break;
				
			case 6:
				ServoDown();
			break;
			
			case 7:
				MagnetON(); 
			break;
			
			case 8:
				MagnetOFF(); 
			break;

			case 9:
				ServoTitsch();
			break;

			case 10:
				Schachzug_1(int(values[1]), int(values[2]));
			break;
			
			case 11:
				Schachzug_2(int(values[1]), int(values[2]), int(values[3]), int(values[4]));
			break;

			case 12:
				Schlagzug(int(values[1]), int(values[2]), int(values[3]), int(values[4]));
			break;

			case 13:
				MapBerechnung(int(values[1])); 
			break;
			
			case 14:
				TestTextbox();
			break;
			
			case 15:
				Kaufmann(float(values[1]));
			break;
			
			case 16:
				ServoPrintActualAngle();
			break;
			
			case 17:
				ServoUpdateAngles(int(values[1]), int(values[2])); 
				//values[1] = MinAngle, values[2] = MaxAngle
			break;
		}//END switch
	}//END if (Serial.available())

   feldindex = 0;
	 
   values[0] = 0;
   values[1] = 0;
   values[2] = 0;
	values[3] = 0;
   values[4] = 0;
}//END void loop()
//-------------------------------------------------------------------
//###################################################################
int runden(float number) // ---- BEGIN runden() ---------------------
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
} // ----------------------------- END runden() ---------------------
//###################################################################
void Reset()  //--------------- BEGIN Reset() -----------------------
{ 
	int speedReset_Y = 2000;
	int speedReset_X = 2000;
	int wegLaengeX;
	int wegLaengeY; 
	
	//wegLaengeX = 980; //ausgemessen
	//wegLaengeY = 1180;//ausgemessen
	
	wegLaengeX = runden(StepsProMM * 22.5); //ausgemessen
	wegLaengeY = runden(StepsProMM * 29.5); //ausgemessen
	
	digitalWrite(MOTORS_ENABLED,LOW); //Treiber unter Spannung

	// X_RESET Begin ----
	StepperX_DIR_PLUS(); //Richtung nach rechts
	for(int i = 0; i <= wegLaengeX; i++)
	{
		yield(); // Do (almost) nothing 
		StepperX_run(speedReset_X);  
	}
	delay(1000);
	// X_RESET End ----
	
	//Y_RESET Begin ----
	StepperY_DIR_PLUS(); //Fahrt nach vorn
	for(int i = 0; i <= wegLaengeY; i++)
	{ 
		yield(); // Do (almost) nothing 
		StepperY_run(speedReset_Y);
	}  
	// Y_RESET End ----

	digitalWrite(MOTORS_ENABLED,HIGH); //Treiber wird entlastet
	istPosition_X = 0; //Nullpunkt definieren
	istPosition_Y = 0;
	Park_Index = 0; //Park_Index 0 bis 7
	
	Serial.println("ok 2: Reset");	
}//------------------------- END Reset() ----------------------------
//###################################################################
void FahrtXYZ(float XSteps, float YSteps) //Eingabe: Steps
{
float sollPosition_X;
float sollPosition_Y;

float stepsToGo_X = 0;
float stepsToGo_Y = 0;

float SummeXYZ;
int multiplikator_X = 1;
int multiplikator_Y = 1;

int counter_X = 0;
int counter_Y = 0;
int stepCount;

	digitalWrite(MOTORS_ENABLED,LOW); //Treiber unter Spannung	
	
	sollPosition_X = XSteps;
	sollPosition_Y = YSteps;
			
// DEBUG_PRINTLN_TXT_VAL("sollPosition_X ", XSteps);
// DEBUG_PRINTLN_TXT_VAL("sollPosition_Y ", YSteps);
// DEBUG_PRINTLN;

// Richtung bestimmen, abhängig von IstPosition
//Ich weiß: die abs()-Funktion muss hier natürlich her.
//stepsToGo_X = abs(sollPosition_X - istPosition_X);
//Ich lasse es jetzt aber so...
	//--------------------------------------------
   if (sollPosition_X > istPosition_X){
		StepperX_DIR_PLUS();
		stepsToGo_X = sollPosition_X - istPosition_X;
   }
   else if(sollPosition_X < istPosition_X){
		StepperX_DIR_MINUS();
		stepsToGo_X = istPosition_X - sollPosition_X;  // X  
   }
	else if (sollPosition_X == istPosition_X){
		stepsToGo_X = 0;
		VerteilerKoeff_X = 0;		
	}
	//--------------------------------------------
   if (sollPosition_Y > istPosition_Y){
		StepperY_DIR_PLUS();
		stepsToGo_Y = sollPosition_Y - istPosition_Y;
   }
   else if(sollPosition_Y < istPosition_Y){
		StepperY_DIR_MINUS();
		stepsToGo_Y = istPosition_Y - sollPosition_Y;  // Y
   }
	else if (sollPosition_Y == istPosition_Y){
		stepsToGo_Y = 0;
		VerteilerKoeff_Y = 0;
	}
   //--------------------------------------------	
 
	SummeXYZ = (stepsToGo_X + stepsToGo_Y);
	
// DEBUG_PRINTLN_TXT_VAL("stepsToGo_X = ", stepsToGo_X);
// DEBUG_PRINTLN_TXT_VAL("stepsToGo_Y = ", stepsToGo_Y);
// DEBUG_PRINTLN;

   //--------------------------------------------	
	if(stepsToGo_X == 0)
	{
		VerteilerKoeff_X = 0;
	}
	else
	{
		VerteilerKoeff_X = SummeXYZ / stepsToGo_X;	
	}
   //--------------------------------------------	
	if(stepsToGo_Y == 0)
	{
		VerteilerKoeff_Y = 0;
	}
	else
	{
		VerteilerKoeff_Y = SummeXYZ / stepsToGo_Y;	
	}
   //--------------------------------------------

// DEBUG_PRINTLN_TXT_VAL("VerteilerKoeff_X = ", VerteilerKoeff_X);	
// DEBUG_PRINTLN_TXT_VAL("VerteilerKoeff_Y = ", VerteilerKoeff_Y);		

	istPosition_X = sollPosition_X;
	istPosition_Y = sollPosition_Y;
	//--------------------------------------------	
	stepCount = 0;
	multiplikator_X = 1;
	multiplikator_Y = 1;
   wegZaehler      = 1;
	int IntSummeXYZ = int(SummeXYZ);
	//********************************************	
	for( stepCount = 0; stepCount <= IntSummeXYZ; stepCount++)
	{ 
		yield(); // Do (almost) nothing
		
		if (stepCount <= rampenLaenge)
		{
			//actualSpeed = beschleunigenParabel[stepCount]; //für Parabel
			actualSpeed = beschleunigenSinus[stepCount]; //für Sinus 
			//actualSpeed = maxSpeed;
		}
		if (stepCount > rampenLaenge && stepCount < (SummeXYZ - rampenLaenge))
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
		if (stepCount >= (SummeXYZ - rampenLaenge))
		{
			//actualSpeed = bremsenParabel[wegZaehler];  //für Parabel
			actualSpeed = bremsenSinus[wegZaehler];  //für Sinus
			wegZaehler++;
			//actualSpeed = maxSpeed;
		}
		// if ((stepsToGo_X < 850) || (stepsToGo_Y < 850))
		// {
			// actualSpeed = 700;
		// }
		//-----------------------------------------
		if (stepCount == runden(VerteilerKoeff_X * multiplikator_X))
		{		  
			StepperX_run(actualSpeed); 
			multiplikator_X++;
			counter_X++;
		}
		//-----------------------------------------
		if (stepCount == runden(VerteilerKoeff_Y * multiplikator_Y))
		{		  
			StepperY_run(actualSpeed); 
			multiplikator_Y++;
			counter_Y++;			
		}
		//-----------------------------------------	
	}//END for( stepCount = 0; stepCount <= SummeXYZ; stepCount++)
		
// DEBUG_PRINTLN_TXT_VAL("counter_X = ", counter_X);	
// DEBUG_PRINTLN_TXT_VAL("counter_Y = ", counter_Y);	
// DEBUG_PRINTLN_TXT("#################################");
	multiplikator_X = 1;
	multiplikator_Y = 1;
   wegZaehler      = 1;
	counter_X = 0;	
	counter_Y = 0;
//***********************************************
	digitalWrite(MOTORS_ENABLED,HIGH); //Treiber aktiv (HIGH = inaktiv)	   
	Serial.println("ok"); 
}//-------- END FahrtXYZ(int XSteps, int YSteps) --------------------

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

void MagnetON()
{ 
	digitalWrite(MAGNET, HIGH);  	// case 7:
	Serial.println("ok 7: MagnetON");	
}
void MagnetOFF()
{
	digitalWrite(MAGNET, LOW);	 	// case 8:
	Serial.println("ok 8: MagnetOFF");
} //----------------------- END Magnet ON/OFF -----------------------

void ServoUP()                   // case 5:
{
	 for(int pos = ServoWinkelMin; pos < ServoWinkelMax; pos++) 
	 { // Servo Up, Grundeinstellung: ServoWinkelMin = 10, ServoWinkelMax = 170
      yield;
		ServoMove(pos);
	 }
	 Serial.println("ok 5: ServoUP");
}
//------
void ServoDown()                 // case 6:
{
	for(int pos = ServoWinkelMax; pos > ServoWinkelMin; pos--) 
	{ // Servo Down, Grundeinstellung: ServoWinkelMax = 170, ServoWinkelMin = 10
		yield;
		ServoMove(pos);
	}
	Serial.println("ok 6: ServoDown");

}
//------
void ServoTitsch()                  //case 9: 
{                    
	ServoDown();
	delay(3000);
	ServoUP();
	Serial.println("ok 9: ServoTitsch");
}
//------
void ServoPickUpFigure()            //Case 3:
{          
	ServoDown();
	delay(400); //Verweilzeit auf der Figur
	digitalWrite(MAGNET, HIGH);
	delay(20);
	ServoUP();	
	Serial.println("ok 3: ServoPickUpFigure");	
}
//------
void ServoPutDownFigure()           //Case 4:
{          
	ServoDown();
	delay(400); //Verweilzeit auf der Figur
	digitalWrite(MAGNET, LOW);
	delay(20);
	ServoUP();	
	Serial.println("ok 4: ServoPutDownFigure");	
}
//------
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
//------
void Schachzug_1(int Buchstabe, int Zahl)                     //Case 10:
{	//Eingabe ASCII-Werte: z.B. 'a1' -> 'a'=97, '1'=49
	//Dann Umrechnung der ASCII-Werte in die Koordinaten des Schachbretts
	char myBuchChar = Buchstabe;
	char myZahlChar = Zahl;
	int Steps_X;
	int Steps_Y;

// DEBUG_PRINTLN_TXT_VAL("ASCII Buchstabe = ", Buchstabe);
// DEBUG_PRINTLN_TXT_VAL("ASCII Zahl      = ", Zahl);
// DEBUG_PRINTLN_TXT_VAL("Buchstabe       = ", myBuchChar);
// DEBUG_PRINTLN_TXT_VAL("Zahl            = ", myZahlChar);
	Buchstabe = Buchstabe - (94 - (Buchstabe - 94) * 19);
	Zahl = Zahl - (48 - (Zahl - 48) * 19) - 20;
// DEBUG_PRINTLN_TXT_VAL("Wert Buchstabe  = ", Buchstabe);
// DEBUG_PRINTLN_TXT_VAL("Wert Zahl       = ", Zahl);

	Steps_X = runden(Buchstabe * StepsProMM);
	Steps_Y = runden(Zahl * StepsProMM);
// DEBUG_PRINTLN_TXT_VAL("Steps_X         = ", Steps_X);
// DEBUG_PRINTLN_TXT_VAL("Steps_Y         = ", Steps_Y);

	FahrtXYZ(Steps_X,Steps_Y);

	Serial.println("ok 10: Schachzug_1");
}
//------
void Schachzug_2(int X_Begin, int Y_Begin, int X_End, int Y_End)   //Case 11:
{
	X_Begin = runden(X_Begin * StepsProMM);
	Y_Begin = runden(Y_Begin * StepsProMM);
	X_End = runden(X_End * StepsProMM);
	Y_End = runden(Y_End * StepsProMM);

	FahrtXYZ(X_Begin,Y_Begin);
	ServoPickUpFigure();
	FahrtXYZ(X_End,Y_End);
	ServoPutDownFigure();
	FahrtXYZ(0,0);
	

	Serial.println("ok 11: Schachzug_2");
}//- END Schachzug_2(int X_Begin, int Y_Begin, int X_End, int Y_End)-
//------
void Schlagzug(int X_Begin, int Y_Begin, int X_End, int Y_End)     //Case 12:
{
	X_Begin = runden(X_Begin * StepsProMM);
	Y_Begin = runden(Y_Begin * StepsProMM);
	X_End = runden(X_End * StepsProMM);
	Y_End = runden(Y_End * StepsProMM);
	Park_X[Park_Index] = runden(Park_X[Park_Index] * StepsProMM);
	Park_Y[Park_Index] = runden(Park_Y[Park_Index] * StepsProMM);
	
	FahrtXYZ(X_End,Y_End);
	ServoPickUpFigure();
	FahrtXYZ(Park_X[Park_Index],Park_Y[Park_Index]);
	ServoPutDownFigure();
	
	FahrtXYZ(X_Begin,Y_Begin);
	ServoPickUpFigure();
	FahrtXYZ(X_End,Y_End);
	ServoPutDownFigure();
	FahrtXYZ(0,0);
	
	Park_Index++;
	Serial.println("ok 12: Schlagzug");
}//- END Schlagzug_2(int X_Begin, int Y_Begin, int X_End, int Y_End)-
//------
void MapBerechnung(byte Fig_Adress) 						             //case 13:
{
	byte EinerStelle;
	byte ZehnerStelle;
	int Koeffizient;
	byte FeldQuadrat = 20; //mm
	int X_Achse, Y_Achse;
	int X_Begin, Y_Begin, X_End, Y_End;
	//Restwertberechnung (Modulo)

	//---------------------------------------------------------
	EinerStelle = Fig_Adress % 10;
	
	if (EinerStelle == 0)			 				    //1. Reihe
	{
		Koeffizient  = -11;
		ZehnerStelle = (Fig_Adress / 10) % 10;
		X_Achse = ZehnerStelle * FeldQuadrat;
		X_Begin = runden(ZehnerStelle * FeldQuadrat * StepsProMM);
		X_End   = 20 * StepsProMM;
		
		if (Fig_Adress >= 100)
		{
			X_Achse = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat;	
			X_Begin = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat * StepsProMM; 
		}
		
		Y_Achse = EinerStelle * FeldQuadrat;
		
		Y_Begin = runden(EinerStelle * FeldQuadrat * StepsProMM);
				
		Serial.print(" 1.Reihe "); Serial.print(EinerStelle);
		//Serial.print(" Fig_Adress "); Serial.print(Fig_Adress);
		Serial.print(" X_Achse "); Serial.print(X_Achse);
		Serial.print(" Y_Achse "); Serial.print(Y_Achse);	
		Serial.print(" X_Begin "); Serial.println(X_Begin);
		//Serial.print(" Y_Achse "); Serial.println(Y_Achse);		
	}
//---------------------------------------------------------	
	if (EinerStelle == 1)								 //2. Reihe
	{
		Koeffizient  = -2;
		ZehnerStelle = (Fig_Adress / 10) % 10;
		X_Achse = ZehnerStelle * FeldQuadrat;
		X_Begin = runden(ZehnerStelle * FeldQuadrat * StepsProMM);
		X_End   = 20 * StepsProMM;
		
		if (Fig_Adress >= 100)
		{
			X_Achse = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat;	
			X_Begin = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat * StepsProMM; 
		}
		
		Y_Achse = EinerStelle * FeldQuadrat;		
		
		Y_Begin = runden(EinerStelle * FeldQuadrat * StepsProMM);
		Serial.print(" 2.Reihe "); Serial.print(EinerStelle);
		//Serial.print(" Fig_Adress "); Serial.print(Fig_Adress);
		Serial.print(" X_Achse "); Serial.print(X_Achse);
		Serial.print(" Y_Achse "); Serial.print(Y_Achse);
		Serial.print(" X_Begin "); Serial.println(X_Begin);
	}
//---------------------------------------------------------		
	if (EinerStelle == 6)								 //7. Reihe
	{
		Koeffizient  = 83;
		ZehnerStelle = (Fig_Adress / 10) % 10;
		X_Achse = ZehnerStelle * FeldQuadrat;
		X_Begin = runden(ZehnerStelle * FeldQuadrat * StepsProMM);
		X_End   = 20 * StepsProMM;
		
		if (Fig_Adress >= 100)
		{
			X_Achse = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat;	
			X_Begin = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat * StepsProMM; 
		}
		
		Y_Achse = EinerStelle * FeldQuadrat;
		
		Y_Begin = runden(EinerStelle * FeldQuadrat * StepsProMM);
		Serial.print(" 7.Reihe "); Serial.print(EinerStelle);
		//Serial.print(" Fig_Adress "); Serial.print(Fig_Adress);
		Serial.print(" X_Achse "); Serial.print(X_Achse);
		Serial.print(" Y_Achse "); Serial.print(Y_Achse);
		Serial.print(" X_Begin "); Serial.println(X_Begin);

	}
//---------------------------------------------------------		
	if (EinerStelle == 7)								 //8. Reihe
	{
		Koeffizient  = 92;
		ZehnerStelle = (Fig_Adress / 10) % 10;
		X_Achse = ZehnerStelle * FeldQuadrat;
		X_Begin = runden(ZehnerStelle * FeldQuadrat * StepsProMM);
		X_End   = 20 * StepsProMM;
		
		if (Fig_Adress >= 100)
		{
			X_Achse = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat;	
			X_Begin = (Fig_Adress - EinerStelle) / 10 * FeldQuadrat * StepsProMM; 
		}
		
		Y_Achse = EinerStelle * FeldQuadrat;
		
		Y_Begin = runden(EinerStelle * FeldQuadrat * StepsProMM);
		Serial.print(" 8.Reihe "); Serial.print(EinerStelle);
		//Serial.print(" Fig_Adress "); Serial.print(Fig_Adress);
		Serial.print(" X_Achse "); Serial.print(X_Achse);
		Serial.print(" Y_Achse "); Serial.print(Y_Achse);
		Serial.print(" X_Begin "); Serial.println(X_Begin);

	}
}
//---------- END void MapBerechnung(byte Fig_Adress) ------

void TestTextbox()										                   //case 14:
{	
	Serial.println("Zeile1");
	Serial.println("Zeile2");
	Serial.println("Zeile3");
	Serial.println("Zeile4");
}
//---------------------------------------------------------
void Kaufmann(float dezimalZahl)                                    //case 15:
{	
	int gerundeteZahl = runden(dezimalZahl);
	Serial.print("Dezimalzahl    = "); Serial.println(dezimalZahl);
	Serial.print("gerundete Zahl = "); Serial.println(gerundeteZahl);
	Serial.println();	
}
//---------------------------------------------------------
void ServoPrintActualAngle()										            //case 16:
{	
	Serial.println(ServoWinkelMin);
	Serial.println(ServoWinkelMax);
}
//---------------------------------------------------------
void ServoUpdateAngles(int MinAgle, int MaxAngle)                     //case 17:
{
	ServoWinkelMin = MinAgle;
	ServoWinkelMax = MaxAngle;	
}
//---------------------------------------------------------