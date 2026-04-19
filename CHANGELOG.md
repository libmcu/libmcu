# Changelog
## v0.3.0 - March 30, 2026
- mcumgr: add management interface and ESP-IDF port
- pki: add certificate chain verification support
- metrics: add typed metric macros
- metrics: add schema support and in-band encoding
- metrics: add validation on initialization
- metrics: add `metrics_unset()`
- ringbuf: add dynamic buffer resizing
- ringbuf: add `ringbuf_available()`
- wdt: add utility functions
- wdt: add threaded mode and overflow fix
- dfu: add self-test and pending verification support
- dfu: improve image validation and tag version parsing
- board: add switch-to-factory API
- board: add reboot reasons for diagnostics
- wifi: add IP information to wifi interface
- logging: improve tag handling
- base64: add buffer size checks
- cli: add DEL key support and improve buffer handling
- Apple: add semaphore implementation
- OpenSSL: add PKI utility functions for key and certificate management
- ESP-IDF: map NVS errors to errno
### **Breaking Changes**
- interfaces: apply prefix to all public interface symbols
- logging: rename `__FILENAME__` to `FILENAME_TAG`
- wdt: add `lm_` prefix to avoid symbol conflicts
- flash: add `lm_` prefix to avoid symbol conflicts

## v0.2.6 - March 9, 2025
- crc32: add CRC-32 computation and table generation
- ble: add BLE interface
- fsm: add FSM module
- runner: add runner module and bypass option
- dfu: add DFU module
- xmodem: add XMODEM protocol implementation
- buzzer: add buzzer module with melody support
- cleanup: add priority-based cleanup module
- ratelim: add rate limiting module
- msgq: add message queue module
- metricfs: add metric storage module
- board: add CPU load API and idle hook
- board: add `board_name()` and `board_get_time_since_boot_us()`
- cli: add pause functionality and environment pointer
- wdt: add watchdog timer interface and hooks
- metrics: add `is_set`, min/max helpers, and ISO8601-related test support
- strext: add `strupper()`, `strlower()`, and `strchunk()`
- timext: add ISO 8601 conversion helpers
- ports: add PKI interface for mbedtls v3
- ci: add version consistency workflow
### **Breaking Changes**
- i2c: separate bus and device APIs
- pwm: add channel and pin parameters to interface APIs
- spi: refactor for multi-device support and add pin parameter
- timer: rename `timer` module to `apptmr`
- fsm: rename `fsm_state()` to `fsm_get_state()`

## v0.2.5 - January 10, 2024
### **Breaking Changes**
- interfaces: replace port interface with vtable

## v0.2.4 - January 9, 2024
- port: freertos: add stack overflow hook
- button: change time data type from int to long
- logging: change stringify() to return length instead of pointer
- cli: add history functionality
- port: freertos: fix task stack size calculation
- pm: add power management interface
- i2c: add i2c interface
- uart: add uart interface
- adc: add adc interface
- wifi: add wifi interface
- spi: add spi interface
- gpio: add gpio interface
- timer: add timer interface
- pwm: add pwm interface
- metrics: change metric key type to uint16_t
- logging: fix deadlock
- add ARM Cortex-M fault handler
- actor: add actor model
### **Breaking Changes**
- button: change handler interface
- kvstore: rename api of `struct kvstore` to `struct kvstore_api`
- common: replace string-based reboot reason with code-based

## v0.2.3 - January 4, 2023
### **Breaking Changes**
- metrics: change to not designate key ID when defining a metric
- cli: add convenient macros to define/register commands

## v0.2.2 - December 31, 2022
- button: replace `pthread_mutex` with `xxx_lock/unlock` wrapper
- ports: freertos: remove `sem_post_nointr()`
- ao: add `ao_post_if_unique()`

## v0.2.1 - December 10, 2022
- Add trace module
- cli: trim leading spaces
- kvstore: add clear functionality
- kvstore: remove nvs initialization in esp-idf port
- Add active object module

## v0.2.0 - October 18, 2022
- Add the ring buffer module
- Add an exponential back-off module
- Add base64
- Add debouncing module
- Add metrics module
- Add CMake scaffolding
- Add hexdump
- Add COBS
- Refactor build system
- Rename `components` to `modules`
- Rename `shell` to `cli`
- Rename `system` to `board`
### cli
- Remove `strpbrk()` dependancy
- Fix not to split quote string with spaces
### compiler
- Add `CONST_CAST` macro
### logging
- Add tag functionality
- Place temporal buffer in bss instead of stack
- Change message length data type from `uint8_t` to `uint16_t`
- Fix buffer overflow when stringifying
### ports
- Add freertos posix pthread port 
- Fix freertos tick and time conversion
- Fix division by zero
- Fix header path
### pubsub
- Add wildcard support
- Support zero length message publish

## v0.1.0 - December 13, 2020
The initial release.
