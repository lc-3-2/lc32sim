# LC-3.2 Simulator
A user-mode simulator of the LC-3.2 ISA, written in C++23. It can be used to run LC-3.2 binaries in either a graphical or a headless mode. To create such binaries, take a look at [lc-3-2/llvm](https://github.com/lc-3-2/llvm);

## Build and run:
Build with CMake and Ninja:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=DEBUG -G Ninja ..
ninja
```

The following build configs are supported: `[DEBUG, RELEASE, RELEASE_DBGINFO]`

Run with:
```bash
./lc32sim <path-to-lc3-binary>
```

## Configuration
The simulator can be configured using a JSON config file. The default config file is `lc32sim.json` in the current working directory. The config file can be changed using the `-c` command line option.

### Example configuration file
```json
{
    "log_level": "INFO",
    "display": {
        "width": 640,
        "height": 480,
        "vblank_length": 68,
        "instructions_per_scanline": 400,
        "frames_per_second": 60.0,
        "accelerated_rendering": true
    },
    "memory": {
        "size": 4294967296,
        "simulator_page_size": 4096
    },
    "keybinds": {
        "a": "A",
        "b": "B",
        "select": "Backspace",
        "start": "Return",
        "right": "Right",
        "left": "Left",
        "up": "Up",
        "down": "Down",
        "r": "R",
        "l": "L"
    }
}
```
For the latest list of configuration options as well as their default values, refer to `src/config.hpp` ([link](src/config.hpp)).

### Command line options
Certain configuration options can be overridden using command line options. The following command line options are supported:
```
-v, --version              Print version information and exit
-h, --help                 Print help message and exit
-c, --config <path>        Path to JSON-formatted config file [default: ./lc32sim.json]
-s, --software-rendering   Disable hardware-accelerated rendering, even if enabled in config
-l, --log-level <level>    Set minimum log level to be displayed; lower levels are suppressed
-H, --headless             Run simulator without a display
```

For a guaranteed up-to-date summary of command line options, execute `./lc32sim --help`.
