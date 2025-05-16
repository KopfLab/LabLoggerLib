![blink](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-blink.yaml/badge.svg?branch=main) 
![publish](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-publish.yaml/badge.svg?branch=main)

# LabLoggerLibs

This repository holds a collection of code libraries for ***LabLogger*** devices. To cite the use of ***LabLogger*** libraries and devices in scientific publications, please use:

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

The following firmware is included in the repository to provide frequently used stand-alone tests for ***LabLogger*** devices and individual ***LabLogger*** components. The build status is based on automated cloud compile in the `main` and `dev` branches.

| Program  | *main* branch | *dev* branch  |
| :------- | :--- | :--- |
| blink    | ![blink](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-blink.yaml/badge.svg?branch=main) | ![blink-dev](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-blink.yaml/badge.svg?branch=dev) |
| i2c_scanner    | ![blink](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-i2c_scanner.yaml/badge.svg?branch=main) | ![blink-dev](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-i2c_scanner.yaml/badge.svg?branch=dev) |
| publish  | ![publish](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-publish.yaml/badge.svg?branch=main) | ![publish-dev](https://github.com/KopfLab/LabLoggerLibs/actions/workflows/compile-publish.yaml/badge.svg?branch=dev) |

### Compile

To generate binaries for these programs:

 - clone this repository and all submodules (`git clone --recurse-submodules https://github.com/KopfLab/LabLoggerLibs`, see [dependencies](DEPENDENCIES.md) for details)
 - install the [Particle Cloud command line interface (CLI)](https://github.com/spark/particle-cli)
 - log into your account with `particle login`
 - install ruby gems with `bundle install`
 
In your main directory, the following `rake` shortcuts are now available:

- to list all programs: `rake programs` (or `rake help` for all available tasks)
- to compile a program: `rake PROGRAM` (e.g. `rake blink` and `rake publish`)
- to flash latest compile via USB: `rake flash`
- to flash latest compile via cloud: `rake flash DEVICE=name`
- to start serial monitor: `rake monitor`

For additional options and rake tasks, see the documentation in the [Rakefile](Rakefile).

### Development

The configuration for individual programs is managed via their compile workflow YAML (e.g. `.github/workflows/compile-blink.yaml`), which defines a GitHub action to automatically compile the program firmware upon pushes to the repo. The workflow YAML defines all necessary source code, libraries, and auxiliary files, as well as what platform and version to compile for, in the job `matrix` settings (`program` and `platform`). The [Rakefile](Rakefile) `compile` task actually just parses these settings to determine which files to include to compile a specific program. 

The workflow YAML additionally specifies which folders to watch for changes to trigger the automatic rebuild on GitHub in the `push` -> `paths` setting. This information is also used by the [Guardfile](Guardfile) to figure out which files should trigger an automatic rebuild during development. Use `rake PROGRAM` to compile a program for the first time and then activate automatic re-compiles by running `bundle exec guard`. It will figure out which program was last build, pull the folders to watch out of the workflow YAML and trigger re-compile if anything changes. 

To add a new program (`myprog`):

 - work in a development git branch (e.g. `dev-myprog`)
 - create a sub folder `src/myprog` that includes a `project.properties` file with `name=myprog` and a list of commented out dependencies
 - if there are any new dependencies, add them to the table at the end of the `README.md`, and as git submodules in the lib/ folder via `cd lib` + `git submodule add https://github.com/...`
 - add a YAML workflow for github actions in `.github/workflows/compile-myprog.yaml` (see e.g. `i2c_scanner` as example) that lists the `src`, `lib` and `aux` needed to compile the program
 - add a task in the `Rakefile` under the `### PROGRAM ###` subheading that's simply `task :myprog => :compile`
 - test compilation with `rake myprog`, fix issues in the sources (`src/myprog/`) and with libraries as needed until it compiles successfully
 - use `bundle exec guard` to continue development with auto compilation
 - once the program works as intended and compiles correctly via GitHub actions (https://github.com/kopflab/LabLoggerLibs/actions), add it to the list of firmware in the `README.md` with the github actions badges to `main` and `dev` (whichever dev branch is the correct one, e.g. `dev-myprog`)

## Libraries

### LoggerCore

The `LoggerCore` library provides the following functionality.

- easily extensible framework for implementing cloud-connected LabLoggers based on the secure and well-established Particle [Photon 2](https://docs.particle.io/reference/datasheets/wi-fi/photon-2-datasheet/) platform
- constructs JSON-formatted data logs for flexible recording in spreadsheets or databases via cloud webhooks
- build-in data averaging and error calculation
- built-in support for remote control via cloud commands
- built-in support for device state management (logging behavior, data read and log frequency, etc.)
- built-in connectivity management with data caching during offline periods - Photon2 memory typically allows caching of 100 logs in permanent flash memory (protected even against power outtages) and an additional 500-1000 logs in volatile memory (transmitted if device goes online before a power out but lost during a power out)

## Dependencies

The following third-party software is used in the ***LabLogger*** libraries:

| **Library** | **Dependency**                         | **Website**                                                        | **License** |
|-------------|----------------------------------------|--------------------------------------------------------------------|-------------|
| *all*       | Particle firmware                      | https://github.com/particle-iot/device-os                          | LGPL3.0     |
| LoggerCore  | DeviceNameHelperRK                     | https://github.com/rickkas7/DeviceNameHelperRK                     | MIT         |
| LoggerCore  | FileHelperRK                           | https://github.com/rickkas7/FileHelperRK                           | MIT         |
| LoggerCore  | SequentialFileRK                       | https://github.com/rickkas7/SequentialFileRK                       | MIT         |
| LoggerCore  | PublishQueueExtRK                      | https://github.com/rickkas7/PublishQueueExtRK                      | MIT         |
| LoggerCore  | SparkFun_Qwiic_OpenLog_Arduino_Library | https://github.com/sparkfun/SparkFun_Qwiic_OpenLog_Arduino_Library | MIT         |
| LoggerOled  | Adafruit_SSD1306                       | https://github.com/adafruit/Adafruit_SSD1306                       | BSD         |
| LoggerOled  | Adafruit-GFX-Library                   | https://github.com/adafruit/Adafruit-GFX-Library                   | BSD         |
| LoggerOled  | Adafruit_BusIO                         | https://github.com/adafruit/Adafruit_BusIO                         | MIT         |


