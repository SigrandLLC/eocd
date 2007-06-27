#ifndef EOCD_UNIX_IFACE
#define EOCD_UNIX_IFACE

//---- interface settings ----//
#define SOCKET_PATH "/var/sigrand-eocd/"
#define SOCKET_NAME "eocd-socket"
#define MAX_CONNECTIONS 5


//---- interface structures -----//

typedef enum { REQUEST=0,RESPONSE } Eocd_msg_status;
typedef enum { EOCD_SERVER=0,EOCD_CLIENT } Eocd_if_type;

typedef struct {
    int fd[MAX_CONNECTIONS +1];
    int fd_num;
    int hisock;        
    unsigned char fd_act[MAX_CONNECTIONS +1];
    fd_set socks;
    Eocd_if_type type;
} Eocd_desc;

typedef struct {
    unsigned int len;
    unsigned short id;
    unsigned short sub_id:15;
    unsigned short status:1;
    unsigned short span;
    unsigned char dev;
} Eocd_msg;


//---- Message functions ----//
Eocd_msg *eocd_alloc_msg(int len);
int eocd_setup_msg(Eocd_msg *msg,int id,int sub_id,int span,char dev);
char *eocd_get_dataptr(Eocd_msg *msg);
inline void eocd_set_msg_status(Eocd_msg *msg,Eocd_msg_status status);
Eocd_msg_status eocd_get_msg_status(Eocd_msg *msg);
int eocd_send_msg(int fd,Eocd_msg *msg);
int eocd_recv_msg(int fd,Eocd_msg **msg);

//---- Descriptor functions ----//
inline Eocd_desc *eocd_alloc_desc();
inline void eocd_free_desc(Eocd_desc *d);
int eocd_init_client(Eocd_desc *desc);
int eocd_init_server(Eocd_desc *desc);
inline int eocd_wait(Eocd_desc *desc);
inline int eocd_nex_fd(Eocd_desc *d);


//---- Message IDs -----//

enum msg_ID{
// Service
    GET_SPAN_CONF,
// SNMP related
    SNMP_SPAN_CONF,
    SNMP_ENDP_CONF,
    SNMP_ENDP_MAINT,
    SNMP_UNIT_MAINT,
    SNMP_SPAN_CONF_PROF,
    SNMP_ENDP_ALARM_PROF
};

//---- subsystems ----//

// SNMP_SPAN_CONF
enum snmp_span_conf{
    NUM_REP,
    CONF_PROF,
    ALARM_PROF
};

// SNMP_ENDP_CONF
enum snmp_endp_conf{
    ENDP_ALARM_PROF
};

// SNMP_ENDP_MAINT
enum snmp_endp_maint{
    LO_CONF,
    PWR_BACKOFF,
    SOFT_RST
};

// SNMP_UNIT_MAINT
enum snmp_unit_maint{
    LO_TOUT
};

// SNMP_SPAN_CONF_PROF
enum snmp_span_conf_prof{
    WIRE_NUM=0,
    MIN_LRATE,
    MAX_LRATE,
    PSD,
    TRNSM_MODE,
    REMOTE,
    PWR_FEED,
    CUR_MARG_DOWN,
    WORST_MARG_DOWN,
    CUR_MARG_UP,
    WORST_MARG_UP,
    USE_MARG,
    CLOCK_REF,
    LPROBE,
    ROW_STAT
};


// SNMP_ENDP_ALARM_PROF

#endif


