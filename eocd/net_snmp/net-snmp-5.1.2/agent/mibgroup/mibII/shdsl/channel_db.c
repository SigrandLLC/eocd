#include "channel_db.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>

#include <net-snmp/net-snmp-config.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

//#include "struct.h"
//#include "util_funcs.h"
//#include "../sysORTable.h"
//#include "../interfaces.h"

#include "shdsl.h"
#include "channel_db.h"



init_comm()