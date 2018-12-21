#include <msp430.h>
#include <stdio.h>

// Global variables
char input = 'a';                               // Character to store the received UART value
int topic = 0;                                  // Integer to check if the topic is received
int value = 0;                                  // Integer to check if the value is received
int counter = 0;                                // Counter to increment through the arrays
char topicArray[8];                             // Array to store the topic received
char valueArray[3];                             // Array to store the value received
int valueInt = 0;                               // Integer to store the value received as an integer
int position = 0;                               // Integer to check if position topic is received
int toggle = 0;                                 // Integer to check if toggle topic is received
int light = 0;                                  // Integer to check if light topic is received
char positionArray[] = {'P','o','s','i','t','i','o','n'};
                                                // Array to check against the position topic
int toggleArray[] = {'T','o','g','g','l','e'};  // Array to check ahainst the toggle topic
int lightArray[] = {'L','i','g','h','t'};       // Array to check against the light topic
int messageReceived = 0;                        // Variable to determine if a UART message was received
int lightNumber = 0;                            // Integer to store the light value

void configurePWM() {
    P1DIR |= BIT4;                              // Sets P1.4 to the output direction
    P1SEL |= BIT4;                              // Connects P1.4 to TA CCR0 Capture
    P1DIR |= BIT5;                              // Sets P1.5 to the output direction
    P1SEL |= BIT5;                              // Connects P1.5 to TA CCR0 Capture
    TA0CTL = TASSEL_1 + ID_0+ MC_1 + TACLR;     // Configures TA0 to utilize ACLK, an internal divider
                                                // of 1, sets the clock to up mode, and initially clears
                                                // the clock
    TA0CCR0 = 656;                              // Sets the period of the PWM cycle
    TA0CCR3 =48;                                // Sets the duty cycle
    TA0CCTL3 = OUTMOD_7;                        // Sets TA0 to set/reset
}

void configurePWM1 () {
    P2DIR |= BIT0;                              // Sets P1.4 to the output direction
    P2SEL |= BIT0;                              // Connects P1.4 to TA CCR0 Capture
    TA1CTL = TASSEL_1 + ID_0+ MC_1 + TACLR;     // Configures TA0 to utilize ACLK, an internal divider
                                                // of 1, sets the clock to up mode, and initially clears
                                                // the clock
    TA1CCR0 = 656;                              // Sets the period of the PWM cycle
    TA1CCR1 = 656;                              // Sets the duty cycle
    TA1CCTL1 = OUTMOD_7;                        // Sets TA0 to set/reset
}

void configureUART0() {
    P3SEL |= BIT3 | BIT4;                       // P3.4 is connected to RX and P3.3 is connected to TX
    UCA0CTL1 = UCSWRST;                         // Enables software reset
    UCA0CTL1 = UCSSEL_2;                        // Sets SMCLK as the clock source
    UCA0BR0 = 8;                                // Sets the baud rate to 115200
    UCA0BR1 = 0;                                // Sets the baud rate to 115200
    UCA0MCTL |= UCBRS_6 | UCBRF_0;              // Sets the baud rate to 115200
    UCA0CTL1 &= ~UCSWRST;                       // Disables software reset
    UCA0IE |= UCRXIE;                           // Enables RX based interrupts
    UCA0IE |= UCTXIE;                           // Enables TX based interrupts;
}

void configureInput() {
    P1DIR &= ~BIT0;                             // Sets P1.0 in the input direction
    P1IE |= BIT0;                               // P1.0 interrupt enabled
    P1IES |= BIT0;                              // P1.0 interrupt flag is set with a high
                                                // to low transition
    P1IFG &= ~BIT0;                             // P1.0 interrupt flag is cleared
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    configurePWM();                             // Configured PWM to control servo
    configurePWM1();                            // Configured PWM to control digital to analog converter
    configureUART0();                           // Configured UART to receive messages from ESP8266
    configureInput();                           // Configures interrupt for input coming from comparator

    __bis_SR_register(GIE);                     // LPM0, ADC12_ISR will force exit
    while (1) {                                 // Infinite loop
        while (messageReceived == 1) {          // If the message is received
            // Integers utilized to determine the number of the value received
            int int1 = 0;
            int int2 = 0;
            int int3 = 0;

            // Function to store the value received as an integer
            if (valueArray[2] == 0) {
                if (valueArray[1] == 0) {
                    valueInt = valueArray[0] - '0';
                } else {
                    int1 = (valueArray[0] - '0') * 10;
                    int2 = (valueArray[1] - '0');
                    valueInt = int1 + int2;
                }
            } else {
                int1 = (valueArray[0] - '0') * 100;
                int2 = (valueArray[1] - '0') * 10;
                int3 = (valueArray[2] - '0');
                valueInt = int1 + int2 + int3;
            }

            // Function to determine the topic based on the array received
            int i = 0;
            for (i = 0; i < 8; i++) {
                if (topicArray[i] == positionArray[i] && toggle == 0 && light == 0) {
                    position = 1;
                } else if (topicArray[i] == toggleArray[i] && position == 0 && light == 0) {
                    toggle = 1;
                } else if (topicArray[i] == lightArray[i] && position == 0 && toggle == 0) {
                    light = 1;
                }
            }

            // Clears the topic array of all values
            for (i = 0; i < 8; i++) {
                topicArray[i] = 0;
            }
            // CLears the value array of all values
            for (i = 0; i < 3; i++) {
                valueArray[i] = 0;
            }

            // If the position topic is called
            if (position == 1) {
                if (valueInt > 100) {
                                valueInt = 100;
                } else if (valueInt < 0) {
                                valueInt = 0;
            }
            TA0CCR3 = valueInt * -0.18 + 48;    // Controls the servo value based on the position
            // If the toggle topic is called
            } else if (toggle == 1) {
                if (valueInt > 1) {
                    valueInt = 1;
                } else if (valueInt < 0) {
                    valueInt = 0;
                }
                valueInt = valueInt * 100;
                TA0CCR3 = valueInt * -0.18 + 48;
                                                // Toggles the servo open or closed
            // If the light topic is called, it controls the PWM value to the DAC
            } else if (light == 1){
                if ((valueInt > 0.0) && (valueInt <= 10.0)) {
                    TA1CCR1 = 600.0 - (15.0 * valueInt);
                } else if ((valueInt > 10.0) && (valueInt <= 20.0)) {
                    TA1CCR1 = 500.0 - (5.0 * valueInt);
                } else if ((valueInt > 20.0) && (valueInt <= 35.0)) {
                    TA1CCR1 = 460.0 - (3.0 * valueInt);
                } else if ((valueInt > 35.0) && (valueInt <= 100.0)) {
                    TA1CCR1 = 400.0 - (1.50 * valueInt);
                }
            }
            messageReceived = 0;                // Stops the while loop
        }
    }
}

#pragma vector=USCI_A0_VECTOR
__interrupt void UART0(void) {
    P1OUT |= BIT0;                              // Turns on the P1.0 LED
    if (UCA0IFG & UCTXIFG) {                    // If the TX interrupt flag is triggered
        UCA0IFG &= ~UCTXIFG;                    // Clears the TX interrupt flag
        ADC12CTL0 |= ADC12SC;                   // Start sampling and conversion of ADC12
    } if (UCA0IFG & UCRXIFG){                   // If the RX interrupt flag is triggered
        input = UCA0RXBUF;                      // Stores the received value in input
        if (input == 10 || input == 13){        // If statement for the end of the received message from the ESP8266
            value = 1;                          // Determines the value is received
            counter = 0;                        // Sets the counter to zero
            messageReceived = 1;                // Sets message received to run function in main
        } else if (input == ':' || input == ' ') {
                                                // If statement for after the topic has been received
            topic = 1;                          // Determines the topic is received
            counter = 0;                        // Sets the counter to zero
        } else if (input == '!') {              // If statement for the start of the message
            topic = 0;                          // Determines the topic is not yet received
            value = 0;                          // Determines the value is not yet received
            counter = 0;                        // Sets the counter to zero
            messageReceived = 0;                // Sets the message received to zero
            position = 0;                       // Sets position to zero
            toggle = 0;                         // Sets toggle to zero
            light = 0;                          // Sets light to zero
        } else if (topic == 1) {                // If statement to store the value
            valueArray[counter] = input;
            counter ++;
        } else if (topic == 0) {                // If statement to store the topic
            topicArray[counter] = input;
            counter++;
        }
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void button_interrupt(void){
    TA0CCR3 = 100 * -0.18 + 48;                 // Sets PWM duty cycle going to the DAC
    P1IFG &= ~BIT0;                             // Clears the interrupt flag
}

