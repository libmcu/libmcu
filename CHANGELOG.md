# Changelog

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
