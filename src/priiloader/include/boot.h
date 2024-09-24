#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <ogc/isfs.h>
#include <ogc/machine/processor.h>
#include <malloc.h>
#include <unistd.h>
#include "ChannelHandler.hpp"
#include "identify.h"
#include "fs.h"
//#include "wdvd.h"



int bootChannel(u64 title, bool boot4by3 = false);