Network-Device-Driver
=====================
This is an example of kernel programming as a part of the Operaring Systems project. In this project I created
a network device driver which has the following properties:
  1. Two network interfaces named os1 and os2 are created.
  2. These interfaces can be brought up and shut down by ifconfig.
  3. Addresses will be hard wired: 192.168.0.1 for os1 and 192.168.1.1 for os2.
  4. Transmission of packets from one to the other will be observed using tcpdump or ethereal (a.k.a wireshark).
