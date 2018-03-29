# CALSAT (STAC CubeSat) Userspace

[![Build Status](https://travis-ci.org/space-technologies-at-california/calsat-userspace.svg?branch=master)](https://travis-ci.org/space-technologies-at-california/calsat-userspace)
[![codecov](https://codecov.io/gh/space-technologies-at-california/calsat-userspace/branch/master/graph/badge.svg)](https://codecov.io/gh/space-technologies-at-california/calsat-userspace)
[![CodeFactor](https://www.codefactor.io/repository/github/space-technologies-at-california/calsat-userspace/badge)](https://www.codefactor.io/repository/github/space-technologies-at-california/calsat-userspace)

This repo contains all the software for our CubeSat. It is organized into
subdirectory for each application.

To build for your host computer, run:
```shell
make native-config all
```

See [build-process](docs/build-process.md) for more information about building
this project.

See [project-organization](docs/project-organization.md) for how this project is
organized.

## Useful Links

This page has useful information specific to our main computer, including how to
load software onto the NAND: [Digi
Documentation](https://www.digi.com/resources/documentation/digidocs/90001547/#landing_pages/yocto_index.htm%3FTocPath%3DDigi%2520Embedded%2520Yocto%7C_____0)

This is the IRVINE01 software repository, which has example implementations for
sensor and comms software: [IRVINE01
repo](https://github.com/irvinecubesat/irvine-01-sw)
