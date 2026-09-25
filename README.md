## Electra One base firmware

## Building
The following steps will compile ElectraOne base firmware with the Demo application compiled in:

1. Download the ARM GNU Toolchain. You can get it at [GNU Arm Embedded Toolchain Downloads](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads).
1. Install it or make symlinks so that the `bin`, `lib`, `share`, `arm-none-eabi` directories are located in the `/usr/local/arm` directory. If the location is not suitable, provide Makefile with alternative `COMPILERPATH` variable.
1. Switch to the project root directory and run `./scripts/install-tools`. On windows, you will most likely encounter compilation errors. I am working on it. Ignore it for now.
1. Build the firmware with `make all`
1. Make sure your ElectraOne is connected to the computer with a USB cable and run `make upload`.

If you got compilation errors when running `./scripts/install-tools`, the `make upload` will fail. In that case you can upload the firmware image using the ElectraOneConsole app. You can find the `firmware.img` file in the `build` directory.


## Building the MIDI Controller app
The MIDI Controller application lives in `apps/controller` and is built from this
same repository:

1. Run `make APPPATH=apps/controller all` to build the firmware image.
1. Run `make APPPATH=apps/controller upload` to build it and upload it to the unit.

`apps/demo` is still the default, so a plain `make all` builds the Demo app.


## Building custom app
If you were able to compile and run the firmware with the default Demo app, you can compile a custom app by taking following steps. Minimal HelloWorld application will be used to demonstrate that.

1. Download or clone the [Demo applications](https://github.com/electraone/demos) from the GitHub.
1. Place the `demos` directory next to the `firmware` directory.
1. Switch to the `firmware` directory and create a symlink to the app, `ln -s ../demos/helloworld`
1. Run `make APPPATH=helloworld upload`. This will build the firmware with the HelloWorld compiled in and the firmware will be uploaded to the ElectraOne controller.
