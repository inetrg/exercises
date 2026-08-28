# BLE Advertisements!

This exercise uses the NimBLE Bluetooth stack to send custom advertisements.

To change to this directory from a different exercise, use the following command in the terminal.

```sh
$ cd ../12-bluetooth
```

## Task 1
To make our node receive BLE advertisements we configure the NimBLE scanner to listen for other BLE devices.

**1. Initialize the nimble scanner:**
```C
/* [TASK 1.1: initialize the nimble scanner ] */
nimble_scanner_cfg_t params = {
    .itvl_ms = SCAN_INTERVAL_MS,
    .win_ms = SCAN_WINDOW_MS,
    .flags = NIMBLE_SCANNER_PHY_1M,
};

/* initialize the scanner and set up our own callback */
nimble_scanner_init(&params, nimble_scan_evt_cb);
```

Besides the scanner configuration parameters the `nimble_scanner_init()` function takes a user defined callback as an argument.
This callbaclk function get executed on every discovered advertisement packet.
Implement the callback.

**2. Implement a custom callback for scan events:**
```C
void nimble_scan_evt_cb(uint8_t type, const ble_addr_t *addr,
                        const nimble_scanner_info_t *info,
                        const uint8_t *ad, size_t len)
{
    ...
}
```

Lets add some logic to the callback so it outputs information about discovered devices.

**3. Use the bluetil utility features to parse the name of advertised devices:**
```C
bluetil_ad_t rec_ad;

/* drop const of ad with cast. Ensure read-only access! */
uint8_t *ad_ro = (uint8_t*)ad;
bluetil_ad_init(&rec_ad, ad_ro, len, len);

char name[BLE_ADV_PDU_LEN + 1] = {0};
int res = bluetil_ad_find_str(&rec_ad, BLE_GAP_AD_NAME,
                              name, sizeof(name));
```

**4. Output the name, address, and the raw data of the advertisement packet:**
```C
if (res == BLUETIL_AD_OK) {
    printf("\n\"%s\" @", name);
}
nimble_addr_print(addr);
printf("sent %d bytes:\n", len);
_print_hex_arr(ad, len);
```

**5. Now, add a simple shell command to start and stop the scanning procedure via the RIOT shell:**
```C
int cmd_scan(int argc, char **argv)
{
    if (argc == 2) {
        if (strcmp("start", argv[1]) == 0) {
            nimble_scanner_start();
            return 0;
        } else if (strcmp("stop", argv[1]) == 0) {
            nimble_scanner_stop();
            return 0;
        }
    }

    printf("usage: %s start|stop\n", argv[0]);
    return 0;
}
SHELL_COMMAND(scan,"start/stop BLE scanning",cmd_scan);
```

Finally, compile, flash and run the application and start the scanner.

```sh
$ make all flash term
```

```sh
> scan start
```

If you are not living under a rock and, for example, test this in a university lab, there is a good chance the node will already pick up quite a few advertisements of devices closeby.


## Task 2
We now want to enable our node to not only receive advertisements but also send out advertisements with our own data.

**1. First, add a function for configuring advertisements with a custom payload:**
```C
static void start_adv(uint8_t *payload, unsigned payload_len)
{
...
}
```

**2. Initialize the required data structures and configure the advertisement parameters:**
```C
/* buffer for the advertisement */
static uint8_t adv_buf[ADV_PKT_BUFFER_SIZE];
struct os_mbuf *data;
int rc;
struct ble_gap_ext_adv_params params;

/* advertising data struct */
static bluetil_ad_t ad;

/* use defaults for non-set params */
memset (&params, 0, sizeof(params));

/* advertise using ID addr */
params.own_addr_type = id_addr_type;

params.primary_phy = BLE_HCI_LE_PHY_1M;
params.secondary_phy = BLE_HCI_LE_PHY_1M;
params.tx_power = TX_POWER_UNDEF;
params.sid = 0;
/* min/max advertising interval converted from ms to 0.625ms units */
params.itvl_min = BLE_GAP_ADV_ITVL_MS(600);
params.itvl_max = BLE_GAP_ADV_ITVL_MS(800);

/* configure the nimble instance */
rc = ble_gap_ext_adv_configure(NIMBLE_INSTANCE, &params, NULL, NULL, NULL);
assert (rc == 0);
```

**3. Create a new advertisement packet:**
```C
/* get mbuf for adv data */
data = os_msys_get_pkthdr(ADV_PKT_BUFFER_SIZE, 0);
assert(data);

/* build advertising data with flags to specifiy that:
 * - the device is a BLE device (instead of BR/EDR a.k.a. bluetooth classic)
 * - the device is not discoverable */
rc = bluetil_ad_init_with_flags(&ad, adv_buf, sizeof(adv_buf),
        BLE_GAP_FLAG_BREDR_NOTSUP);
assert(rc == BLUETIL_AD_OK);

/* give the device a name that is included in the advertisements */
rc = bluetil_ad_add_name(&ad, adv_name);
assert(rc == BLUETIL_AD_OK);
```

**4. Append a manufacturer specific data type to the advertisement:**
```C
/* Add a manufacturer specific data entry with custom marker. */
_ad_append_marked_msd_payload(&ad, payload, payload_len);

/* fill mbuf with adv data */
rc = os_mbuf_append(data, ad.buf, ad.pos);
assert(rc == 0);

rc = ble_gap_ext_adv_set_data(NIMBLE_INSTANCE, data);
assert (rc == 0);
```

**5. Start advertising:**
```C
rc = ble_gap_ext_adv_start(NIMBLE_INSTANCE, 0, 0);
assert (rc == 0);

printf("Now advertising \"%s\"\n", payload);
```

**6. Define how to build our own manufacturer specific payload:**
```C
/* hand-craft a manufacturer specific data type with a custom marker at the start of the data */
static void _ad_append_marked_msd_payload(bluetil_ad_t *ad, const uint8_t *payload, unsigned len)
{
    uint8_t msd_len = sizeof(_company_id_code) + 1 +
                      sizeof(_custom_msd_marker_pattern) + len;
    uint8_t data_type = BLE_GAP_AD_VENDOR;

    /* set the size field */
    _ad_append(ad, &msd_len, sizeof(msd_len));

    /* set the data type */
    _ad_append(ad, &data_type, sizeof(data_type));

    /* set the company id code */
    _ad_append(ad, _company_id_code, sizeof(_company_id_code));

    /* set the marker */
    _ad_append(ad, _custom_msd_marker_pattern, sizeof(_custom_msd_marker_pattern));

    /* set the payload */
    _ad_append(ad, payload, len);
}
```

**7. Add another shell comand which allows changing the advertised data:**

```C
int cmd_adv(int argc, char **argv)
{
    /* check that the command is called correctly */
    if (argc != 2) {
        puts("usage: adv <message>");
        return 1;
    }

    /* if advertising is already active stop it before updating
     * the advertised content */
    if (ble_gap_ext_adv_active(NIMBLE_INSTANCE)) {
        ble_gap_ext_adv_stop(NIMBLE_INSTANCE);
    }

    _pl_len = strlen(argv[1]);

    /* update the payload with the given message */
    memcpy(_payload_buf, argv[1], _pl_len);

    start_adv(_payload_buf, _pl_len);

    return 0;
}
SHELL_COMMAND(adv,"set advertised message",cmd_adv);
```

## Task 3

To ease testing our application with our custom extended advertisements we first add some filtering logic to our scan callback.
We want to make sure to only display advertisements which we are interested in.

**1. Filter for extended advertisements only:**
```C
/* ignore legacy advertisements */
if (!(type & NIMBLE_SCANNER_EXT_ADV)) {
    return;
}
```

**2. Additionally filter based on our custom marker in the manfacturer specific data:**
```C
bluetil_ad_data_t msd;
res = bluetil_ad_find(&rec_ad, BLE_GAP_AD_VENDOR, &msd);
if (res == BLUETIL_AD_OK) {
    uint8_t *marker = &msd.data[sizeof(_company_id_code)];
    if (memcmp(marker, _custom_msd_marker_pattern,
               sizeof(_custom_msd_marker_pattern)) == 0) {
        uint8_t *payload = &msd.data[MSD_PAYLOAD_OFFS];
        /* length of the payload without the marker */
        int pl = msd.len - MSD_PAYLOAD_OFFS;
        printf("%.*s\n", pl, payload);
    }
}
```

Now we can test the advertisement application with two node. First flash both of them.

```sh
$ make all flash term
```

Now configure one to advertise a custom message and the other to scan for advertisements.

**On node 1:**
```sh
> adv "some fancy advertisement message"
```

**On node 2:**
```sh
> scan start
```
