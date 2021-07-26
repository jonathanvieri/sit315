// Variables

// Pins
const int ledPin = 10;
const int motionPin = 2;

volatile int ledState = LOW;

void setup()
{
  Serial.begin(9600);
  
  // Pin modes
  pinMode(ledPin, OUTPUT);

  // Interrupt
  attachInterrupt(digitalPinToInterrupt(motionPin), toggleLED , CHANGE);

}

void loop()
{	
  Serial.println("Do Something");
  delay(3000);
}


void toggleLED()
{
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
}
