/* Feb 9 2013

dmt@saikoled.com
guest@openmusiclabs.com

*/

#define redPin 9    
#define greenPin 10 
#define bluePin 11 // Blue LED connected to digital pin 11
#define whitePin 13 // White LED connected to digital pin 13

#define steptime 1

#define SIZE 32 // max frame size, if you want to do it this way
volatile byte dmx_buffer[SIZE]; // allocate buffer space

volatile unsigned int dmx_ptr = 0;
volatile byte dmx_state = 0; // state tracker

void setup() {
  pinMode(whitePin, OUTPUT);  
  // Enable output on port to control direction of RS485
  DDRD |= 0x03;
  PORTD |= 0x01;
  
  // initialize uart for data transfer
  UBRR1H = ((F_CPU/250000/16) - 1) >> 8;
  UBRR1L = ((F_CPU/250000/16) - 1);
  UCSR1A = 1<<UDRE1;
  UCSR1C = 1<<USBS1 | 1<<UCSZ11 | 1<<UCSZ10; // 2 stop bits,8 data bitss
  UCSR1B = 1<<TXEN1; // turn it on

  sei();
  analogWrite(whitePin,10);
}

void loop() {
  // use the data in dmx_buffer[] here
  //analogWrite(redPin,dmx_buffer[0]);
  //analogWrite(greenPin,dmx_buffer[1]);
  //analogWrite(bluePin,dmx_buffer[2]);
  //analogWrite(whitePin,dmx_buffer[3]);
  
  while(dmx_state); // check if last buffer was sent out
  // fill buffer with some new data
  for (byte i = 0; i < SIZE; i++) {
    dmx_buffer[i] = i << 3;
  }
  // send out dmx data
  dmx_send();

}


ISR(USART1_TX_vect) {

  // handle data transfers
  // check for state 3 first as its most probable
  if (dmx_state == 3) {
    UDR1 = dmx_buffer[dmx_ptr];
    dmx_ptr++;
    if (dmx_ptr >= SIZE) {
      dmx_state = 4; // last byte sent
    }
  }
  else if (dmx_state == 1) {
    // reset baudrate generator
    UBRR1H = ((F_CPU/250000/16) - 1) >> 8;
    UBRR1L = ((F_CPU/250000/16) - 1);
    UDR1 = 0; // send slot0 blank space
    dmx_state = 2; // set to slot0 transmit mode
  }
  else if (dmx_state == 2) {
    UDR1 = dmx_buffer[0]; // send first data byte
    dmx_ptr = 1; // set pointer to next data byte
    dmx_state = 3; // set to data transfer mode
  }
  else {
    UCSR1B &= ~(1<<TXCIE1); // disable tx complete interrupt
    UCSR1A |= 1<<TXC1; // clear flag if set
    dmx_state = 0; // reset - bad condition or done

  }
}

void dmx_send() {
  dmx_state = 1; // set to BREAK transfer
  // set usart for really slow transfer for BREAK signal
  // this will produce a BREAK of 112.5us
  // and a MAB of 25us
  UBRR1H = ((F_CPU/80000/16) - 1) >> 8;
  UBRR1L = ((F_CPU/80000/16) - 1);
  UCSR1B |= 1<<TXCIE1; // enable tx complete interrupt
  UDR1 = 0;
  
}

