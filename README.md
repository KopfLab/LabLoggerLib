![blink](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-blink.yaml/badge.svg?branch=main) 
![publish](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-publish.yaml/badge.svg?branch=main)

# LabLoggerLibs

This repository holds a collection of code libraries for ***LabLogger*** devices. All content of this repository is free to use for noncommercial research and development efforts as outlined in the terms of the included [LICENSE file](LICENSE.md).

To cite the use of ***LabLogger*** libraries and devices in publications, use:

> Kopf S, 2025. _LabLogger: modular data logging for research labs_. <span>https://github.com/KopfLab/LabLoggerLibs<span>

A BibTeX entry for LaTeX users is

```
@Manual{
  microLogger,
  title = {LabLogger: modular data logging for research labs},
  author = {Sebastian Kopf},
  year = {2025},
  url = {https://github.com/KopfLab/LabLoggerLibs}
}
```

## Firmware

The following firmware is included in the repository to provide frequently used stand-alone tests for ***LabLogger*** devices and individual ***LabLogger*** components. The build status is based on automated cloud compile in the `Main` and `dev` branches.

| Program  | main | dev  |
| :------- | ---: | ---: |
| blink    | ![blink](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-blink.yaml/badge.svg?branch=main) | ![blink-dev](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-blink.yaml/badge.svg?branch=dev) |
| publish  | ![publish](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-publish.yaml/badge.svg?branch=main) | ![publish-dev](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-publish.yaml/badge.svg?branch=dev) |

To generate binaries for these programs:

 - clone this repository and all submodules (`git clone --recurse-submodules https://github.com/KopfLab/LabLoggerLibs`, see [dependencies](DEPENDENCIES.md) for details)
 - install the [Particle Cloud command line interface (CLI)](https://github.com/spark/particle-cli)
 - log into your account with `particle login`
 
In your main directory, the following `make` shortcuts are now available:

- to compile: `make PROGRAM` (e.g. `make blink` and `make publish`)
- to flash latest compile via USB: `make flash`
- to flash latest compile via cloud: `make flash device=DEVICE`
- to start serial monitor: `make monitor`

For additional options and make targets, see the documentation in the [makefile](makefile).

## Libraries

### LoggerCore

The `LoggerCore` library provides the following functionality.

- easily extensible framework for implementing cloud-connected LabLoggers based on the secure and well-established Particle [Photon 2](https://docs.particle.io/reference/datasheets/wi-fi/photon-2-datasheet/) platform
- constructs JSON-formatted data logs for flexible recording in spreadsheets or databases via cloud webhooks
- build-in data averaging and error calculation
- built-in support for remote control via cloud commands
- built-in support for device state management (logging behavior, data read and log frequency, etc.)
- built-in connectivity management with data caching during offline periods - Photon2 memory typically allows caching of 100 logs in permanent flash memory (protected even against power outtages) and an additional 500-1000 logs in volatile memory (transmitted if device goes online before a power out but lost during a power out)
