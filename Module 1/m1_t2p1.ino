// Variables

int tempInput = 0;
double temp = 0;

// Pins
int ledPin = 10;
int buttonPin = 2;
int tempPin = A5;

// Global Variables
int buttonState = LOW;
int ledState = LOW;


void setup()
{
  Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  // Interrupt
  attachInterrupt(digitalPinToInterrupt(buttonPin), toggleLed, FALLING);
}

void loop()
{	
  tempInput = analogRead(tempPin);
  temp = calculateTemperature(tempInput);
  
  Serial.print("Current Temperature: ");
  Serial.println(temp);
  
  if (temp > 30)
  {
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
  }
  
  else
  {
    ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
  
  delay(1000); 
}

// Method to calculate the current temperature
double calculateTemperature(int tempInput)
{
  temp = (double)tempInput / 1024;
  temp = temp * 5;
  temp = temp - 0.5;
  temp = temp * 100;
  
  return temp;
}


void toggleLed()
{
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
}