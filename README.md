# Elevator Simulation in 8th

This is a simple simulation of an elevator, written in the [8th][8th] programming
language. [8th][8th] is a dialect of [Forth][forth] which is cross-platform and
allows building of applications for Windows, macOS, Linux, iOS and Android.

## Running It

### Running with the 8th interpreter

1. Download [8th][8th] and install it.
2. Make sure the paths at the start of the [Makefile](./Makefile) are set correctly.
   You can check them with the command:

   ```shell
   $ make config
   ```
3. At your shell prompt, type:

    ```shell
    $ make run
    ```

### Compiling the executable

1. Ensure you've installed [8th][8th] and set the paths correctly in the Makefile.
2. Make sure the paths at the start of the [Makefile](./Makefile) are set correctly.
   You can check them with the command:

   ```shell
   $ make config
   ```
3. At your shell prompt, type:

    ```shell
    $ make build
    ```

The compiled executables will be in the [out/](./out/) directory. You can change which
platforms are built by editing the project file [8th.proj](./8th.proj).

## Author

Tammy Cravit <tammy@tammymakesthings.com>

## Version History

###  v1.01 - 2026-11-15
- Added Makefile and associated tooling.

###  v1.00 - 2026-11-15
- Initial Revision

[8th]:   <https://8th-dev.com/>
[forth]: <https://en.wikipedia.org/wiki/Forth_(programming_language)>
