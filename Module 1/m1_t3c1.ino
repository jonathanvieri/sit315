// Variables

// Pins
const int ledPin = 10;
const int motionPin1 = 2;
const int motionPin2 = 3;

volatile int ledState1 = LOW;
volatile int ledState2 = LOW;

void setup()
{
  Serial.begin(9600);
  
  // Pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Interrupt
  attachInterrupt(digitalPinToInterrupt(motionPin1), toggleLED , CHANGE);
  attachInterrupt(digitalPinToInterrupt(motionPin2), toggleBuiltinLED, CHANGE);
}

void loop()
{	
  Serial.println("Do Something");
  delay(3000);
}
	

void toggleLED()
{
  ledState1 = !ledState1;
  digitalWrite(ledPin, ledState1);
}

void toggleBuiltinLED()
{
  ledState2 = !ledState2;
  digitalWrite(LED_BUILTIN, ledState2);
}
