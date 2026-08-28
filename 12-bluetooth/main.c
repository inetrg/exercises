/*
 * Copyright (C) 2025 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 */
#include <stdio.h>
#include <stdlib.h>
#include "host/util/util.h"
#include "host/ble_hs.h"
#include "nimble_scanner.h"
#include "nimble_addr.h"
#include "nimble/ble.h"
#include "net/bluetil/ad.h"
#include "shell.h"
#include "timex.h"
#include "ztimer.h"

#define TX_POWER_UNDEF (127)

#define GAP_NAME_BUF_SIZE (300)
#define ADV_PKT_BUFFER_SIZE (300)
#define NIMBLE_INSTANCE (0)

/* The scan window defines how long to listen on the currently
 * configured advertisement channel. */
#define SCAN_WINDOW_MS    30

/* The scan interval defines how long to wait before starting the next
 * scan window an the next advertisement channel. Setting the
 * SCAN_INTERVAL_MS = SCAN_WINDOW_MS results in continuous scanning.*/
#define SCAN_INTERVAL_MS    30

static uint8_t id_addr_type;

const char adv_name[] = "RIOT-ext-adv";

/* The first two bytes of the manufacturer specific data type contain
 * a company ID code which for a final product must be requested from
 * the Bluetooth SIG. 
 * For testing purposes we use 0xFFFF as an unassigned identifier code.
 */
const uint8_t _company_id_code[] = { 0xFF, 0xFF };

/* We use the manufacturer specific data type to transmit arbitrary data.
 * After the company ID we always put the same marker pattern to be able
 * to easily filter for these packets.
 */
const uint8_t _custom_msd_marker_pattern[] = {
        0xf0, 0x9f, 0x93, 0x9f
};

/* payload offset within the custom marked  manufacturer specific data field */
#define MSD_PAYLOAD_OFFS (sizeof(_company_id_code) + \
                          sizeof(_custom_msd_marker_pattern))

/* buffer to store the advertised data in */
uint8_t _payload_buf[ADV_PKT_BUFFER_SIZE];
unsigned _pl_len = 0;

void _ad_append(bluetil_ad_t *ad, const uint8_t *data, unsigned len)
{
    if ((ad->pos + len) <= ad->size) {
        memcpy(&ad->buf[ad->pos], data, len);
        ad->pos += len;
    }
}

/* [TASK 2.6: define how to build manufacturer specific payload] */

/* [TASK 2.1: add function to configure advertisements with a custom payload] */
void start_adv(uint8_t *payload, unsigned payload_len)
{
    (void)payload;
    (void)payload_len;

    /* [TASK 2.2: Initialize data structures and configure advertisement parameters] */

    /* [TASK 2.3: Create a new advertisement packet] */
 
    /* [TASK 2.4: Append a manufacturer specific data type] */

    /* [TASK 2.5: Start advertising] */
}

/* [TASK 2.7: Add another shell comand for changing the advertised data] */

void _print_hex_arr(const uint8_t *data, unsigned len)
{
    printf("{");
    for (unsigned i = 0; i < len; i++) {
        printf(" 0x%02x%c", data[i], (i == len-1) ? ' ' : ',');
    }
    printf("}\n");
}

/* [TASK 1.2: Implement a custom callback for scan events] */
void nimble_scan_evt_cb(uint8_t type, const ble_addr_t *addr,
                        const nimble_scanner_info_t *info,
                        const uint8_t *ad, size_t len)
{
    assert(addr);
    (void)type;
    (void)info;
    (void)ad;
    (void)len;
    
    /* [TASK 3.1: filter for extended advertisements only] */
    
    /* [TASK 1.3: parse the name of advertised devices] */

    /* [TASK 1.4: Output name, address, and data of the advertisement] */
    
    /* [TASK 3.2: output our payload marked by our custom byte pattern] */
}

/* [TASK 1.5: add a shell command to start and stop scanning] */


int main(void)
{
    /* Sleep so that we do not miss this message while connecting */
    ztimer_sleep(ZTIMER_SEC, 3);
    puts("BLE extended advertisement app!");
    
    int rc;
    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    /* configure global address */
    rc = ble_hs_id_infer_auto(0, &id_addr_type);
    assert(rc == 0);

    /* [TASK 1.1: initialize the nimble scanner ] */

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
/** @} */
