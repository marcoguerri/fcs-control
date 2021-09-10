### What is fcs-control?
fcs-control sends raw Ethernet frames with zero payload with either a corrupted
or correct frame check sequence.

Flags:

* `-m`: destination MAC address
* `-i`: interface to send the frame from
* `-c`: corrupt the FCS


### Usage
Clone and build:
```
git clone https://github.com/marcoguerri/fcs-control.git
cd fcs-control
git submodule update --init
make
```

Example of frame with correct FCS:

```
sudo ./corrupt -m de:ad:be:ef:00:00 -i enp0s31f6
```

Packet capture:
```
20:05:54.253539 aa:bb:cc:dd:ee:ff (oui Unknown) > de:ad:be:ef:00:00 (oui Unknown), ethertype Unknown (0x1213), length 64: 
        0x0000:  0000 0000 0000 0000 0000 0000 0000 0000  ................
        0x0010:  0000 0000 0000 0000 0000 0000 0000 0000  ................
        0x0020:  0000 0000 0000 0000 0000 0000 0000 7a00  ..............z.
        0x0030:  137b
```

Example of frame with corrupted FCS:
```
sudo ./corrupt -m de:ad:be:ef:00:00 -i enp0s31f6 -c
```

Packet capture:
```
20:38:11.657410 aa:bb:cc:dd:ee:ff (oui Unknown) > de:ad:be:ef:00:00 (oui Unknown), ethertype Unknown (0x1213), length 64: 
        0x0000:  0000 0000 0000 0000 0000 0000 0000 0000  ................
        0x0010:  0000 0000 0000 0000 0000 0000 0000 0000  ................
        0x0020:  0000 0000 0000 0000 0000 0000 0000 efbe  ................
        0x0030:  adde                                     ..
```
