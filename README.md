SpeedyShare
===========

SpeedyShare provides a very simple way to send files between computers, no servers or FTP required!

The way SpeedyShare works is by transmitting bytes through good old simple TCP/IP. It requires no registration, 
and only uses an IP address to access the user.

It still requires some work:
  - Secure package sending, it would be nice to encrypt the traffic with a symmetric key.
  - Continuing a download which has been interrupted is currently not possible.
  - Running in the background would be nice since the window isn't really necessary if one is receiving files.
  - Being able to change the listening port.
  - Being able to queue (or in parallel) upload to more than one peer.
  - Being able to stop a file transfer as the receiver.
  - Notifications for when stuff happens behind the curtains.


Download pre-built binary for Windows at:
http://duttenheim.github.io/speedyshare
