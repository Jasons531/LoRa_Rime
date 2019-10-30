
#include "LoRa_Dev.h"
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

static radio_result_t get_value(radio_param_t param, radio_value_t *value)
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
    LoRa_send,
    LoRa_read,
    LoRa_on,
    LoRa_off,
    get_object,
    set_object
  };

/*---------------------------------------------------------------------------*/
static RadioState_t status(void)
{
  return Radio.GetStatus();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void set_txpower(uint8_t power)
{
	uint8_t paConfig = 0;
    uint8_t paDac = 0;
	
	paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );
    paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power > 14 )
        {
            power = 14;
        }
        paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
    }
    
    SX1276Write( REG_PACONFIG, paConfig );
}
/*---------------------------------------------------------------------------*/
#define AUTOCRC (1 << 6)
#define AUTOACK (1 << 5)
#define FRAME_MAX_VERSION ((1 << 3) | (1 << 2))
#define FRAME_FILTER_ENABLE (1 << 0)
/*---------------------------------------------------------------------------*/
int LoRa_init(void)
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
static int LoRa_send(const void *payload, unsigned short payload_len)
{
  Radio.Send( (uint8_t *)payload, payload_len );
}
/*---------------------------------------------------------------------------*/
int LoRa_off(void)
{
	Radio.Standby();
	return 1;
}
/*---------------------------------------------------------------------------*/
int LoRa_on(void)
{
	
}
/*---------------------------------------------------------------------------*/
int LoRa_get_channel(void)
{
  return channel;
}
/*---------------------------------------------------------------------------*/
int LoRa_set_channel(int c)
{
  Radio.SetChannel(c);
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
static int LoRa_read(void *buf, unsigned short bufsize)
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
void LoRa_set_txpower(uint8_t power)
{
  set_txpower(power);
}
/*---------------------------------------------------------------------------*/
int LoRa_get_txpower(void)
{  
  return SX1276Read( REG_PACONFIG );
}
/*---------------------------------------------------------------------------*/
int LoRa_rssi(void)
{
  return Radio.Rssi(MODEM_LORA);
}
