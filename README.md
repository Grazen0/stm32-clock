# STM32 Clock

A simple C program for the STM32F1 to generate a clock signal in both astable
(frequency-driven) and monostable (manual) modes.

## Usage

You'll need to have the GNU ARM Toolchain and [OpenOCD] installed on your
system. You can compile and flash the program to a connected STM32F1 with this
command:

```bash
make flash
```

This will listen for incoming GDB connections, but you can just send `^C` to
quit.

While OpenOCD is listening, you can connect via GDB with this command:

```bash
make debug
```

[openocd]: https://openocd.org/
