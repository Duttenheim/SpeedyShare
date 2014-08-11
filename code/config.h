#pragma once

// defined values
#define MAXPACKAGESIZE 65535
#define MAINPORT 1467
#define SERVICEPORT 1468

// service announcement messages
#define ARE_YOU_SPEEDYSHARE 'IASP'
#define PEER_IS_SPEEDYSHARE 'PISP'

// data transfer messages
#define REQUEST_FILE 'REQF'
#define ACCEPT_FILE 'ACPF'
#define PACKAGE 'PACK'
#define CHUNK 'CHNK'
#define ABORT 'ABRT'

#define KEEP_ALIVE 'KEEP'