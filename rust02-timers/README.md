# Simple timer example

In this example we see two fundamental ways of using the timers of your hardware.
First, we configure a timer to trigger an event that gets executed in the future.
Second, we use the timer to sleep (block the thread) for a known amount of time.

## Adding modules to your RIOT application
RIOT is highly modular, meaning that functionalities are split into modules, which
are only built when required.

In order to interact with timers, we need to use the `ztimer` module. To make it
available to us, we add it to the list of used modules in the application's
Makefile: `USEMODULE += ztimer`. Depending on the granularity that we want when
handling time, we can specify different extra modules:

```Makefile
USEMODULE += ztimer_usec # microsecond precision
USEMODULE += ztimer_msec # millisecond precision
USEMODULE += ztimer_sec  # second precision
```

To learn more about the `ztimer` module, visit the [documentation page](https://doc.riot-os.org/group__sys__ztimer.html).

To change to this directory from a different exercise, use the following command in the terminal.

```sh
$ cd ../rust02-timers
```

## Task 1
**1. Build and flash the application. Connect to the serial port:**
```sh
$ make all flash term
```

**2. Reset the board. You should see a "Timeout!" string after 2 seconds.**

## Task 2
Modify the application so that the board blinks the LED only for 250 ms for 10 iterations.

**1. Adapt the loop to be true for 10 iterations:**
```rust
for _ in 0..10 {
```

**2. Sleep only for 250 ms for each iteration after setting the LED to "on":**
```rust
Clock::msec().sleep(Duration::from_millis(250));
```

**3. Build and flash the application. Connect to the serial port:**
```sh
$ make all flash term
```

## Task 3

Add a new timer to turn LED1 on after 1 second.

**1. Define LED 1:**
```rust
let mut led1 = riot_wrappers::led::LED::<1>::new();
```

**2. Around the loop, call the [`set_during`](https://rustdoc.etonomy.org/riot_wrappers/ztimer/struct.Clock.html#method.set_during) function, and in the callback, turn on LED1.**

```rust
Clock::msec().set_during(
    || led1.on().unwrap(),
    riot_wrappers::ztimer::Ticks::from_duration(
        Duration::from_secs(1)
        ).expect("1 second is expressible in millisecond ticks"),
    || {
```

```rust
});
```

Note that the LED 1 was sent from the main thread to the interrupt handler defined in the first closure.
Code that should execute inside an interrupt handler is limited
(for example, no blocking operations should occur).
The requirement that data sent to an interrupt handler is `Send` prevents many mistakes there.

**3. Recompile and build the application. Press the reset button.**
```sh
$ make all flash term
```
**You should see the LED 1 going on after a second; the LED 0 keeps blinking until "Done!" is be printed**
