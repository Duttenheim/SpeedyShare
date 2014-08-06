SpeedyShare
===========

SpeedyShare provides a very simple way to send files between computers, no servers or FTP required!

The way SpeedyShare works is by transmitting bytes through good old simple TCP/IP. It requires no registration, 
and only uses an IP address to access the user.

It still requires some work:
  - Secure package sending, it would be nice to encrypt a package with a symmetric key. 
  - Continuing a download which has been interrupted is currently not possible.
  - Displaying the local network in a browser would be nice.
  - Aborting the retrieval of a file doesn't remove the progress bar from the sender.
  - Running in the background would be nice since the window isn't really necessary if one is receiving files.
  - Drag'n'drop.
  - Being able to change the listening port.
  - Being able to queue (or in parallel) upload to more than one peer.


Download pre-built binary for Windows at:
http://duttenheim.github.io/speedyshare
