# ESPIDFORTH Component

Embeddable Forth interpreter for ESP-IDF with FFI to ESP-IDF APIs.

## Adding to Your Project

### Option 1: Copy the component

Copy the entire `forth/` directory into your project's `components/` folder:

```
your_project/
  components/
    forth/
      CMakeLists.txt
      idf_component.yml
      forth_core.h
      forth_core.cpp
      forth_version.h
  main/
    main.c
  CMakeLists.txt
```

### Option 2: Git submodule

```bash
cd your_project
git submodule add <repo-url> components/forth
```

### Option 3: ESP Component Manager

Add to your project's `main/idf_component.yml`:

```yaml
dependencies:
  espidforth:
    path: ../path/to/ESPIDFORTH/components/forth
```

## Usage

### Minimal Example

```c
#include "forth_core.h"

/* Your I/O callbacks */
static int my_getchar(void) {
    // return character or -1 if none available
}

static void my_putchar(int c) {
    // output character
}

void app_main(void) {
    /* Initialize with 100 KB dictionary heap */
    forth_init(100 * 1024);

    /* Interactive REPL (blocks forever) */
    forth_repl(my_getchar, my_putchar);

    forth_deinit();
}
```

### Evaluate Forth Without REPL

```c
#include "forth_core.h"

void app_main(void) {
    forth_init(100 * 1024);

    /* Set up I/O first (needed for . and .s output) */
    forth_repl_set_io(my_getchar, my_putchar);  // or just start the REPL

    /* Evaluate Forth expressions programmatically */
    forth_eval("2 3 + .");          // prints "5 "
    forth_eval(": square dup * ;"); // define a word
    forth_eval("7 square .");       // prints "49 "

    forth_deinit();
}
```

## API Reference

```c
/* Initialize the Forth engine with given dictionary heap size in bytes */
int forth_init(int heap_size_bytes);

/* Run interactive REPL (blocking). Provide character I/O callbacks. */
void forth_repl(int (*get_char)(void), void (*put_char)(int));

/* Evaluate a Forth expression string. Returns 0 on success. */
int forth_eval(const char *text);

/* Query Forth dictionary memory usage */
int forth_heap_used(void);
int forth_heap_free(void);

/* Free all Forth engine resources */
void forth_deinit(void);
```

## Build Configuration

Set `ESPIDFORTH_ENABLE_TESTS=1` (default) to include built-in test suites (`test` and `test-ffi` words). Set to `0` to strip test code and save ~5.4 KB flash.

In PlatformIO `build_flags`:
```ini
build_flags = -DESPIDFORTH_ENABLE_TESTS=0
```

Or in ESP-IDF CMake:
```cmake
target_compile_definitions(your_target PRIVATE ESPIDFORTH_ENABLE_TESTS=0)
```

### Sizing the engine (no-PSRAM boards)

The dictionary, code space and stacks are static arrays. Their limits are
`#ifndef`-guarded so a consumer can size them from the build:

| Define | Default | Static cost |
|---|---|---|
| `MAX_WORDS` | 512 | `MAX_WORDS * (MAX_WORD_LEN + ~12)` bytes |
| `MAX_WORD_LEN` | 64 | (per dictionary entry) |
| `MAX_DICT_CODE` | 4096 cells | `* sizeof(cell_t)` (16 KB on 32-bit) |
| `MAX_STACK`, `MAX_RSTACK` | 256 cells each | 1 KB each |
| `MAX_INPUT` | 256 | one line buffer |

Defaults total about 57 KB. A consumer that must leave room for a TLS stack on
an ESP32-S3 without PSRAM builds with

```ini
build_flags = -DMAX_WORDS=160 -DMAX_WORD_LEN=24 -DMAX_DICT_CODE=1536
```

for about 14 KB (measured in an Arduino-esp32 host project, 2026-09). The
`forth_init()` heap size is separate and is malloc'd.

## Targets

Tested on ESP32, ESP32-S3, ESP32-C3, and ESP32-C6 with ESP-IDF >= 5.0.
