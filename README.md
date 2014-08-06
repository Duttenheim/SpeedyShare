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
  - Running in the background would be nice since the window isn't really necessary to have if one is receiving files.
