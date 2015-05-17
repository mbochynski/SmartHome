#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"

void main()
{
        char addr[5];
        char buf[32];
        char status;

        WDTCTL = WDTHOLD | WDTPW;
        DCOCTL = CALDCO_1MHZ;
        BCSCTL1 = CALBC1_1MHZ;
        BCSCTL2 = DIVS_0;  // SMCLK = DCOCLK/1
        // SPI (USI) uses SMCLK, prefer SMCLK=DCO (no clock division)

        /* Initial values for nRF24L01+ library config variables */
        rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
        rf_addr_width      = 5;
        rf_speed_power     = RF24_SPEED_MIN | RF24_POWER_MAX;
        rf_channel         = 0x4c;
        msprf24_init();  // All RX pipes closed by default
        msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst enabled for receiving Auto-ACKs
        msprf24_set_pipe_packetsize(0, 0);  // Dynamic payload length enabled (size=0)

        // Transmit to 'rad01' (0x72 0x61 0x64 0x30 0x31)
        msprf24_standby();
        addr[0] = 'r'; addr[1] = 'a'; addr[2] = 'd'; addr[3] = '0'; addr[4] = '1';
        w_tx_addr(addr);
        w_rx_addr(0, addr);  // Pipe 0 receives auto-ack's, autoacks are sent back to the TX addr so the PTX node
                             // needs to listen to the TX addr on pipe#0 to receive them.
        buf[0] = '1';
        buf[1] = '2';
        buf[2] = '3';
        buf[3] = '\0';
        w_tx_payload(4, buf);
        msprf24_activate_tx();
        LPM4;

        if (rf_irq & RF24_IRQ_FLAGGED) {
                msprf24_get_irq_reason();  // this updates rf_irq
                if (rf_irq & RF24_IRQ_TX)
                        status = 1;
                if (rf_irq & RF24_IRQ_TXFAILED)
                        status = 0;
                msprf24_irq_clear(RF24_IRQ_MASK);  // Clear any/all of them
        }

        status += 1;
        // Do something cool with the 'status' variable
        // ???
        // Profit!
        // Go to sleep forever:
        _DINT();
        LPM4;
}
