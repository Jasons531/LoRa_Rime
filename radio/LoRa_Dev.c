
#include "LoRa_Dev.h"
#include "sx1276.h"

#include "contiki.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/netstack.h"

#ifndef LoRa_CONF_AUTOACK
#define LoRa_CONF_AUTOACK 0
#endif /* LoRa_CONF_AUTOACK */

#define WITH_SEND_CCA 1

#define FOOTER_LEN 2

#define FOOTER1_CRC_OK      0x80
#define FOOTER1_CORRELATION 0x7f

#if defined( USE_BAND_433 )

#define RF_FREQUENCY                                433000000 // Hz 

#elif defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             20        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#endif

#include <stdio.h>
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

void LoRa_arch_init(void);

/* XXX hack: these will be made as Chameleon packet attributes */
rtimer_clock_t LoRa_time_of_arrival, LoRa_time_of_departure;

int LoRa_authority_level_of_sender;

int LoRa_packets_seen, LoRa_packets_read;

#define BUSYWAIT_UNTIL(cond, max_time)                                  \
  do {                                                                  \
    rtimer_clock_t t0;                                                  \
    t0 = RTIMER_NOW();                                                  \
    while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
  } while(0)

volatile uint8_t LoRa_sfd_counter;
volatile uint16_t LoRa_sfd_start_time;
volatile uint16_t LoRa_sfd_end_time;

static volatile uint16_t last_packet_timestamp;
/*---------------------------------------------------------------------------*/
PROCESS(LoRa_process, "LORA driver");
/*---------------------------------------------------------------------------*/

int LoRa_on(void);
int LoRa_off(void);

static int LoRa_read(void *buf, unsigned short bufsize);
static int LoRa_transmit(unsigned short len);
static int LoRa_send(const void *data, unsigned short len);

static int LoRa_receiving_packet(void);
static int pending_packet(void);
/* static int detected_energy(void); */

signed char LoRa_last_rssi;
uint8_t LoRa_last_correlation;

static uint8_t receive_on;
static int channel;

static radio_result_t
get_value(radio_param_t param, radio_value_t *value)
{
  if(!value) {
    return RADIO_RESULT_INVALID_VALUE;
  }
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    *value = receive_on ? RADIO_POWER_MODE_ON : RADIO_POWER_MODE_OFF;
    return RADIO_RESULT_OK;
  case RADIO_PARAM_CHANNEL:
//    *value = cc2520_get_channel();
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MIN:
    *value = 11;
    return RADIO_RESULT_OK;
  case RADIO_CONST_CHANNEL_MAX:
    *value = 26;
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t
set_value(radio_param_t param, radio_value_t value)
{
  switch(param) {
  case RADIO_PARAM_POWER_MODE:
    if(value == RADIO_POWER_MODE_ON) {
      LoRa_on();
      return RADIO_RESULT_OK;
    }
    if(value == RADIO_POWER_MODE_OFF) {
      LoRa_off();
      return RADIO_RESULT_OK;
    }
    return RADIO_RESULT_INVALID_VALUE;
  case RADIO_PARAM_CHANNEL:
    if(value < 11 || value > 26) {
      return RADIO_RESULT_INVALID_VALUE;
    }
//    LoRa_set_channel(value);
    return RADIO_RESULT_OK;
  default:
    return RADIO_RESULT_NOT_SUPPORTED;
  }
}

static radio_result_t
get_object(radio_param_t param, void *dest, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t
set_object(radio_param_t param, const void *src, size_t size)
{
  return RADIO_RESULT_NOT_SUPPORTED;
}

const struct radio_driver cc2520_driver =
  {
    LoRa_init,
    LoRa_prepare,
    LoRa_transmit,
    LoRa_send,
    LoRa_read,
    /* cc2520_set_channel, */
    /* detected_energy, */
    LoRa_cca,
    LoRa_receiving_packet,
    pending_packet,
    LoRa_on,
    LoRa_off,
    get_value,
    set_value,
    get_object,
    set_object
  };

/*---------------------------------------------------------------------------*/
static unsigned int
status(void)
{
  uint8_t status;
//  LoRa_GET_STATUS(status);
  return status;
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
set_txpower(uint8_t power)
{

}
/*---------------------------------------------------------------------------*/
#define AUTOCRC (1 << 6)
#define AUTOACK (1 << 5)
#define FRAME_MAX_VERSION ((1 << 3) | (1 << 2))
#define FRAME_FILTER_ENABLE (1 << 0)
/*---------------------------------------------------------------------------*/
int
LoRa_init(void)
{
  Radio.Init( NULL );
	
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
							   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
							   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							   true, 0, 0, LORA_IQ_INVERSION_ON, 3000000 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
							   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
							   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  process_start(&LoRa_process, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
LoRa_transmit(unsigned short payload_len)
{
  int i, txpower;

  txpower = 0;
  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    /* Remember the current transmission power */
    txpower = LoRa_get_txpower();
    /* Set the specified transmission power */
    set_txpower(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) - 1);
  }

  /* The TX FIFO can only hold one packet. Make sure to not overrun
   * FIFO by waiting for transmission to start here and synchronizing
   * with the CC2420_TX_ACTIVE check in cc2420_send.
   *
   * Note that we may have to wait up to 320 us (20 symbols) before
   * transmission starts.
   */
#ifndef LORA_CONF_SYMBOL_LOOP_COUNT
PRINTF("LORA_CONF_SYMBOL_LOOP_COUNT needs to be set!!!\n");
#else
#define LOOP_20_SYMBOLS CC2420_CONF_SYMBOL_LOOP_COUNT
#endif

#if WITH_SEND_CCA
  BUSYWAIT_UNTIL(status() & BV(CC2520_RSSI_VALID) , RTIMER_SECOND / 10);  strobe(CC2520_INS_STXONCCA);
#endif /* WITH_SEND_CCA */
  for(i = LOOP_20_SYMBOLS; i > 0; i--) {
    if(CC2520_SFD_IS_1) {
      {
        rtimer_clock_t sfd_timestamp;
        sfd_timestamp = cc2520_sfd_start_time;
#if PACKETBUF_WITH_PACKET_TYPE
        if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
           PACKETBUF_ATTR_PACKET_TYPE_TIMESTAMP) {
          /* Write timestamp to last two bytes of packet in TXFIFO. */
          CC2520_WRITE_RAM(&sfd_timestamp, CC2520RAM_TXFIFO + payload_len - 1, 2);
        }
#endif
      }

      if(!(status() & BV(CC2520_TX_ACTIVE))) {
        /* SFD went high but we are not transmitting. This means that
           we just started receiving a packet, so we drop the
           transmission. */
        RELEASE_LOCK();
        return RADIO_TX_COLLISION;
      }
      if(receive_on) {
		ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
      }
      ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);
      /* We wait until transmission has ended so that we get an
	 accurate measurement of the transmission time.*/
     //BUSYWAIT_UNTIL(getreg(CC2520_EXCFLAG0) & TX_FRM_DONE , RTIMER_SECOND / 100);
      BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);

#ifdef ENERGEST_CONF_LEVELDEVICE_LEVELS
      ENERGEST_OFF_LEVEL(ENERGEST_TYPE_TRANSMIT,cc2520_get_txpower());
#endif
      ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
      if(receive_on) {
	ENERGEST_ON(ENERGEST_TYPE_LISTEN);
      } else {
	/* We need to explicitly turn off the radio,
	 * since STXON[CCA] -> TX_ACTIVE -> RX_ACTIVE */
	off();
      }

      if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
        /* Restore the transmission power */
        set_txpower(txpower & 0xff);
      }

      RELEASE_LOCK();

      return RADIO_TX_OK;
    }
  }

  /* If we are using WITH_SEND_CCA, we get here if the packet wasn't
     transmitted because of other channel activity. */
  RIMESTATS_ADD(contentiondrop);
  PRINTF("LoRa: do_send() transmission never started\n");

  if(packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER) > 0) {
    /* Restore the transmission power */
    set_txpower(txpower & 0xff);
  }

  RELEASE_LOCK();
  return RADIO_TX_COLLISION;
}
/*---------------------------------------------------------------------------*/
static int
LoRa_send(const void *payload, unsigned short payload_len)
{
  Radio.Send( (uint8_t *)payload, payload_len );
  return LoRa_transmit(payload_len);
}
/*---------------------------------------------------------------------------*/
int
LoRa_off(void)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
int
LoRa_on(void)
{
 
}
/*---------------------------------------------------------------------------*/
int
LoRa_get_channel(void)
{
  return channel;
}
/*---------------------------------------------------------------------------*/
int
LoRa_set_channel(int c)
{
  uint16_t f;
  /*
   * Subtract the base channel (11), multiply by 5, which is the
   * channel spacing. 357 is 2405-2048 and 0x4000 is LOCK_THR = 1.
   */
  channel = c;

//  f = MIN_CHANNEL + ((channel - MIN_CHANNEL) * CHANNEL_SPACING);
//  /*
//   * Writing RAM requires crystal oscillator to be stable.
//   */
//  BUSYWAIT_UNTIL((status() & (BV(CC2520_XOSC16M_STABLE))), RTIMER_SECOND / 10);

//  /* Wait for any transmission to end. */
//  BUSYWAIT_UNTIL(!(status() & BV(CC2520_TX_ACTIVE)), RTIMER_SECOND / 10);

//  /* Define radio channel (between 11 and 25) */
//  setreg(CC2520_FREQCTRL, f);

//  /* If we are in receive mode, we issue an SRXON command to ensure
//     that the VCO is calibrated. */
//  if(receive_on) {
//    strobe(CC2520_INS_SRXON);
//  }

  RELEASE_LOCK();
  return 1;
}
/*---------------------------------------------------------------------------*/
void
LoRa_set_pan_addr(unsigned pan,
                    unsigned addr,
                    const uint8_t *ieee_addr)
{
  uint8_t tmp[2];

  /*
   * Writing RAM requires crystal oscillator to be stable.
   */
//  BUSYWAIT_UNTIL(status() & (BV(CC2520_XOSC16M_STABLE)), RTIMER_SECOND / 10);

//  tmp[0] = pan & 0xff;
//  tmp[1] = pan >> 8;
//  CC2520_WRITE_RAM(&tmp, CC2520RAM_PANID, 2);


//  tmp[0] = addr & 0xff;
//  tmp[1] = addr >> 8;
//  CC2520_WRITE_RAM(&tmp, CC2520RAM_SHORTADDR, 2);
//  if(ieee_addr != NULL) {
//    int f;
//    uint8_t tmp_addr[8];
//    // LSB first, MSB last for 802.15.4 addresses in CC2520
//    for (f = 0; f < 8; f++) {
//      tmp_addr[7 - f] = ieee_addr[f];
//    }
//    CC2520_WRITE_RAM(tmp_addr, CC2520RAM_IEEEADDR, 8);
//  }
  RELEASE_LOCK();
}
/*---------------------------------------------------------------------------*/
/*
 * Interrupt leaves frame intact in FIFO.
 */
int
LoRa_interrupt(void)
{
  process_poll(&LoRa_process);

  last_packet_timestamp = LoRa_sfd_start_time;
  LoRa_packets_seen++;
  return 1;
}

/*-------------------------------接收数据处理线程--------------------------------------------*/
PROCESS_THREAD(LoRa_process, ev, data)
{
  int len;
  PROCESS_BEGIN();

  PRINTF("LoRa_process: started\n");

  while(1) {
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

    PRINTF("LoRa_process: calling receiver callback\n");

    packetbuf_clear();
    packetbuf_set_attr(PACKETBUF_ATTR_TIMESTAMP, last_packet_timestamp);
    len = LoRa_read(packetbuf_dataptr(), PACKETBUF_SIZE);
    packetbuf_set_datalen(len);

    NETSTACK_RDC.input();
    /* flushrx(); */
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
LoRa_read(void *buf, unsigned short bufsize)
{
  uint8_t footer[2];
  uint8_t len;

  ///判断是否接收
//  if(!CC2520_FIFOP_IS_1) {
//    return 0;
//  }

//  cc2520_packets_read++;


  if(footer[1] & FOOTER1_CRC_OK) {
	LoRa_last_rssi = footer[0];
    LoRa_last_correlation = footer[1] & FOOTER1_CORRELATION;


    packetbuf_set_attr(PACKETBUF_ATTR_RSSI, LoRa_last_rssi);
    packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, LoRa_last_correlation);

    RIMESTATS_ADD(llrx);

  } else {
    RIMESTATS_ADD(badcrc);
    len = FOOTER_LEN;
  }

  /* Another packet has been received and needs attention. */
  process_poll(&LoRa_process);

  RELEASE_LOCK();

  if(len < FOOTER_LEN) {
    return 0;
  }

  return len - FOOTER_LEN;
}
/*---------------------------------------------------------------------------*/
void
LoRa_set_txpower(uint8_t power)
{
  set_txpower(power);
}
/*---------------------------------------------------------------------------*/
int
LoRa_get_txpower(void)
{
  uint8_t power;
//  power = getreg(CC2520_TXPOWER);
  return power;
}
/*---------------------------------------------------------------------------*/
int
LoRa_rssi(void)
{
  int rssi;
  int radio_was_off = 0;

//  BUSYWAIT_UNTIL(status() & BV(CC2520_RSSI_VALID), RTIMER_SECOND / 100);

//  rssi = (int)((signed char)getreg(CC2520_RSSI));
  return rssi;
}
