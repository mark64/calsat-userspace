# CALSAT (STAC CubeSat) Userspace

This repo contains all the software for our CubeSat. It is organized into
subdirectory for each application. For example, code for the daemon maintaining
CubeSat orientation is in the `/orientation/` folder.

Header files are separated and placed in the `/include/`subdirectory, so headers
from the orientation daemon would be placed in `/include/orientation/`. This
must be followed for all code, and header files should be extensively
documented.

## Useful Links

This page has useful information specific to our main computer, including how to
load software onto the NAND. [Digi
Documentation](https://www.digi.com/resources/documentation/digidocs/90001547/#landing_pages/yocto_index.htm%3FTocPath%3DDigi%2520Embedded%2520Yocto%7C_____0)

This is the IRVINE01 software repository, which has example implementations for
sensor and comms software [IRVINE01
repo](https://github.com/irvinecubesat/irvine-01-sw)
