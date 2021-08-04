// Variables

// Pins
const int ledPin = 10;
const int ledPin2 = 11;
const int ledPin3 = 12;
const int ledPin4 = 13;

void setup()
{
  Serial.begin(9600);
  
  // Pin modes
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  
  // Timer and Pin Change interrupt
  startPCINT_Timer();
}

void loop()
{	
  Serial.println("Do Something");
  delay(5000);
}

// Function for setting up the PCINT and timer interrupts
void startPCINT_Timer()
{
  noInterrupts();

  // Setting up the PCINT
  PCICR |= B00000100;	// Enable PCIE2 which controls the group from D0 to D7
  PCMSK2 |= B00011100;	// Enable pin D2, D3, and D4 to trigger interrupt
  
  
  // Setting up the timer interrupts
  
  TCCR1A = 0;	// Reset Timer1 Control Register A
  TCCR1B = 0;	// Do the same for Control Register B  
  TCNT1  = 0;	// Initialize counter value to 0, max for timer1 is 65535
  
  
  // Set compare match register to the desired timer count
  OCR1A = (16000000 / (1024 * 1) -1);	
  	
  // Compare match register for blinking every 2 second 
  // is 31250 = (16*10^6 * 2) / (1*1024) - 1 (for 2 second)
  
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Set the prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  interrupts();
}

// ISR for the timer interrupt
ISR(TIMER1_COMPA_vect){
   digitalWrite(ledPin4, digitalRead(ledPin4) ^ 1);
}

// ISR for the Pin Change Interrupt
ISR (PCINT2_vect) 
{
  if (digitalRead(4) == HIGH)
  {
    Serial.println("PIN D4 Triggered");
    digitalWrite(ledPin3, HIGH);
  }
  
  else
  {
    digitalWrite(ledPin3, LOW);
  }
  
  if (digitalRead(3) == HIGH)
  {
    Serial.println("Pin D3 Triggered");
    digitalWrite(ledPin2, HIGH);
  }
  
  else
  {
    digitalWrite(ledPin2, LOW);
  }
  
  if (digitalRead(2) == HIGH)
  {
    Serial.println("Pin D2 Triggered");
    digitalWrite(ledPin, HIGH);
  }
  
  else
  {
    digitalWrite(ledPin, LOW);
  }
  
}
