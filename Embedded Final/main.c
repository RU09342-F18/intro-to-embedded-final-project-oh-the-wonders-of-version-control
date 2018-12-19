#include <msp430.h>
#include <stdio.h>

// Variables for ADC12 to control servo
int val = 0;
int pos = 0;

//
char input = 'a';

int topic = 0;
int value = 0;
int counter = 0;

char topicArray[8];
char valueArray[3];

int valueInt = 0;

//
int position = 0;
int toggle = 0;
int light = 0;
char positionArray[] = {'P','o','s','i','t','i','o','n'};
int toggleArray[] = {'T','o','g','g','l','e'};
int lightArray[] = {'L','i','g','h','t'};

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

void configureUART0() {
    P3SEL |= BIT3 | BIT4;                       // P3.4 is connected to RX and P3.3 is connected to TX
    UCA0CTL1 = UCSWRST;                         // Enables software reset
    UCA0CTL1 = UCSSEL_2;                        // Sets SMCLK as the clock source
    UCA0BR0 = 8;                                // Sets the baud rate to 115200
    UCA0BR1 = 0;                                // Sets the baud rate to 115200
    UCA0MCTL |= UCBRS_6 | UCBRF_0;              //
    UCA0CTL1 &= ~UCSWRST;                       // Disables software reset
    UCA0IE |= UCRXIE;                           // Enables RX based interrupts
    UCA0IE |= UCTXIE;                           // Enables TX based interrupts;
}

void configureUART1() {
    P4SEL |= BIT4 | BIT5;                       // P4.5 is connected to RX and P4.4 is connected to TX
    UCA1CTL1 = UCSWRST;                         // Enables software reset
    UCA1CTL1 = UCSSEL_2;                        // Sets SMCLK as the clock source
    UCA1BR0 = 8;                                // Sets the baud rate to 115200
    UCA1BR1 = 0;                                // Sets the baud rate to 115200
    UCA1MCTL |= UCBRS_6 | UCBRF_0;              //
    UCA1CTL1 &= ~UCSWRST;                       // Disables software reset
    UCA1IE |= UCRXIE;                           // Enables RX based interrupts
    UCA1IE |= UCTXIE;                           // Enables TX based interrupts
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    //configurePWM();
    //configureADC12();
    configureUART0();
    //configureUART1();

     while (1) {
         ADC12CTL0 |= ADC12SC;                       // Start sampling and conversion of ADC12
         __bis_SR_register(LPM0_bits + GIE);         // LPM0, ADC12_ISR will force exit
     }
}
/*
#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void){
  switch(ADC12IV) {
  case  6:                                       // Vector 6:  ADC12IFG0
      val = ADC12MEM0;
      pos = (val / 4096.0) * 100;
      TA0CCR3 = pos * 0.28 +23;

      __bic_SR_register_on_exit(LPM0_bits);      // Exit active CPU

  default: break;
  }
}
*/

#pragma vector=USCI_A0_VECTOR
__interrupt void UART0(void) {
    P1OUT |= BIT0;                              // Turns on the P1.0 LED
    if (UCA0IFG & UCTXIFG) {                    // If the TX interrupt flag is triggered
        UCA0IFG &= ~UCTXIFG;                    // Clears the TX interrupt flag
        ADC12CTL0 |= ADC12SC;                   // Start sampling and conversion of ADC12
    } if (UCA0IFG & UCRXIFG){                   // If the RX interrupt flag is triggered
        input = UCA0RXBUF;
        if (input == 10 || input == 13){
            value = 1;
            counter = 0;

            sscanf(valueArray, "%d", &valueInt);
        } else if (input == ':' || input == ' ') {
            topic = 1;
            counter = 0;
        } else if (input == '!') {
            topic = 0;
            value = 0;
            counter = 0;
            /*
            int i = 0;

            for (i = 0; i < 8; i++) {
                topicArray[i] = 0;
            }
            for (i = 0; i < 3; i++) {
                valueArray[i] = 0;
            }
            */
        } else if (topic == 1) {
            valueArray[counter] = input;
            counter ++;
        } else if (topic == 0) {
            topicArray[counter] = input;
            counter++;
        }
    }
}

/*
// UART1 interrupt vector
#pragma vector=USCI_A1_VECTOR
__interrupt void UART1(void) {
    if (UCA1IFG & UCTXIFG) {                    // If the TX interrupt flag is triggered
        UCA1IFG &= ~UCTXIFG;                    // Clears the TX interrupt flag
        ADC12CTL0 |= ADC12SC;                   // Start sampling and conversion of ADC12
    } if (UCA1IFG & UCRXIFG){                   // If the RX interrupt flag is triggered
        input = UCA1RXBUF;

        if (input == 10 || input == 13){
            value = 1;
            counter = 0;
        } else if (input == ':' || input == ' ') {
            topic = 1;
            value = 0;
            counter = 0;
        } else if (input == '!') {
            topic = 0;
            value = 0;
            counter = 0;
        } else if (topic == 1 && value == 0) {
            valueArray[counter] = input;
            counter ++;
        } else if (topic == 0 && value == 0) {
            topicArray[counter] = input;
            counter ++;
        } else {
            topic = 0;
            value = 0;
            counter = 0;
        }

    }
}
*/
