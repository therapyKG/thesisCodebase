#include "radio.h"

void rf_init_lora(int freq) {
    mcu_init(freq);
    //radio_events.TxDone = OnTxDone;
    //radio_events.RxDone = OnRxDone;
    //radio_events.TxTimeout = OnTxTimeout;
    //radio_events.RxTimeout = OnRxTimeout;
    //radio_events.RxError = OnRxError;

    sx1276_init(&radio_events);
    sx1276_set_channel(RF_FREQUENCY);


    sx1276_set_txconfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    sx1276_set_rxconfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

}
