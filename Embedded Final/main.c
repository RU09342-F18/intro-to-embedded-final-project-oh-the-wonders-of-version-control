#include <msp430.h>

int voltage = 0;
int resistance =0;

int value;
int previous = 0;

int position = 0;

char temp;

const int arrayLength = 0;
char data[arrayLength];

void configurePWM() {
    P1DIR |= BIT4;                              // Sets P1.2 to the output direction
    P1SEL |= BIT4;                              // Connects P1.2 to TA CCR0 Capture
    TA0CTL = TASSEL_1 + ID_0+ MC_1 + TACLR;    // Configures TA0 to utilize ACLK, an internal divider
                                                // of 1, sets the clock to up mode, and initially clears
                                                // the clock
    TA0CCR0 = 656;                              // Sets the period of the PWM cycle
    TA0CCR3 =72;                                // Sets the duty cycle
    TA0CCTL3 = OUTMOD_7;                        // Sets TA0 to set/reset
}

void configureADC12() {
    ADC12CTL0 = ADC12SHT02 + ADC12ON;           // Turns on ADC12
    ADC12CTL1 = ADC12SHP;                       // Sources the sampcon signal from the sampling timer
    ADC12IE = BIT0;                             // Enables ADC12 interrupts
    ADC12CTL0 |= ADC12ENC;                      // Enales ADC12 conversion
    P6SEL |= BIT0;                              // P6.0 is configures as the ADC12 input
    P1DIR |= BIT0;                              // P1.0 is set to the output direction
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    configurePWM();
    configureADC12();

     while (1) {
         ADC12CTL0 |= ADC12SC;                       // Start sampling and conversion of ADC12
         __bis_SR_register(LPM0_bits + GIE);         // LPM0, ADC12_ISR will force exit
     }
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
  switch(ADC12IV) {
  case  6:                                       // Vector 6:  ADC12IFG0
      value = ADC12MEM0;
      position = (value / 4096.0) * 100;
      TA0CCR3 = position * 0.28 +23;

      __bic_SR_register_on_exit(LPM0_bits);      // Exit active CPU

  default: break;
  }
}

