# Computer Information

## 3/23/2018

The main computer is connected to `drfcx.com` via serial and Ethernet. The
serial port on `drfcx.com` is `/dev/ttyUSB0` and it's local IP address is
`10.0.2.56`. Username: `root`, password: none

## Serial Access

The serial interface is useful because it provides output during boot and does
not require networking to work properly, which allows you to debug issues
preventing boot or remote access.

Anyone with access to `drfcx.com` has been added to the `dialout` UNIX group. If
you don't have access to `/dev/ttyUSB0` and `groups` does not show `dialout`,
contact Mark Hill for access.

To interact with the computer over serial, ssh into `drfcx.com` and run `minicom
-D /dev/ttyUSB0`. Then press `Ctrl-A` then `O` (not zero) to configure minicom.
From here, disable `Hardware Flow Control` by selecting `Serial port setup` and
then pressing `F`. You can now exit the current menu by pressing `enter`. I
recommend saving this setting as your default so that you don't have to
configure minicom each time you start it.

Press enter and you should get either a shell or login prompt. The default login
is user: `root` with no password. If that has been changed, contact Mark Hill to
find out the current login information.

To exit minicom, press `Ctrl-A` then `X`, then press enter to select `Yes` when
prompted.

## SSH

While the serial port is very useful for debugging, it makes file transfer
difficult because binary characters can be intercepted as terminal control
sequences. For this reason, the computer is also connected to `drfcx.com` via
Ethernet. I use a USB-Ethernet adapter managed by ifupdown and have dnsmasq
running as a DHCP server on that USB-Ethernet interface.

If `ssh root@10.0.2.56` fails to connect to the computer, first check the serial
connection to make sure the board is on and working properly. If it is, you can
check it's current IP lease using `cat /var/lib/misc/dnsmasq.leases` on
`drfcx.com`, or by running `ip a` on the computer over serial. If the computer's
`eth0` interface does not have an IP address or is listed as DOWN, run `ifup
eth0` to bring the Ethernet back up.
