This is modified u-boot source for selected Netgear routers based on Atheros SoC and Wifi chips.

I have collected GPL sources from Netgear website, extracted u-boot part from them and compiled into one tree.
I also did a cleanup: there are no compilation errors, warnings, ambigous assignments, unnecessary redundancy.

Following ar71xx/ar724x devices are supported:

* WNR2000v3
* WNR612v2
* WNR1000v2 (v2h2)
* WNR2200
* WNDR3700v1 (u)
* WNDR3700v2 (v1h2)

Compilation requires MIPS toolchain when cross-building on Linux running Intel CPU. Personally I use QEMU i386 Virtual Machine with Slackware 13.37 32-bit and old u-boot recommended MIPS GCC compiler/linker.

This source supports building images for 4, 8 and 16 MB flash sizes so all above devices can be modded and equipped with more flash storage than in original model. Please note that this requires access to SPI NOR flash programmer, soldering skills and knowledge what to do more (ART backup!).

See README.netgear for build instructions.

Loader flashing from u-boot prompt (console access required) is described in README.flash.
