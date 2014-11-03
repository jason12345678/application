#ifndef _COMMON_STRUCT_H
#define _COMMON_STRUCT_H

#include <Winsock2.h>
//#include <ws2ipdef.h>
/*#include <windns.h>*/
#include <Iprtrmib.h>
#include <Iphlpapi.h>
#include <Tlhelp32.h>
#include <afxmt.h>
#include <winioctl.h>
#include <Psapi.h>

#include "list.h"

#define SDK_VERSION         _T("SDK ver1.0.0.24")

typedef struct _proc_prio_entry {
    TCHAR   proc_name[MAX_PATH];
    INT32   prio_idx; /*priority 1 the highest priority, 2 the second 
                  priority and so on. <=0 means low priority.*/

    dlink   link;
}PROC_PRIO_ENTRY, *PPROC_PRIO_ENTRY;

typedef enum _pid_prio {
    ALL_LOW_PRIO,
    PART_HIGH_PRIO,
    FULL_HIGH_PRIO,
    NORMAL_PRIO /*This application doesn't use our NIC*/
}PID_PRIO;

typedef struct _netstat_entry {
    BOOL        is_ipv4;    /* 1:ipv4, 0:ipv6.*/
    BOOL        is_tcp;     /* 1:tcp, 0:udp. */
    BOOL        is_high_priority;
    UINT32      txq_prio; /*TX_PxQUEUE_PRIORITY.*/

    UINT64      rx_highq_sw_fid; /*from FwpmFilterAdd0*/
    UINT64      rx_highq_nic_fid;/*the index of hw filters*/
    UINT64      tx_highq_fid;
    UINT64      tx_lowq_fid;
    ULONGLONG   entry_id;   /* App can relate this unique id to its DataPtr
                            of ListView to identify this entry. */
    UINT64      statis_entry_id;/*Returned from IM driver.*/
    UINT32     udp_ref_count;  /* count udp flow with same local interface, delete when reach 0*/
    BOOL        thru_our_nic; /*Whether this net entry goes thru our nic.*/
    union {
        MIB_TCPROW_OWNER_PID    tcprow;
        MIB_UDPROW_OWNER_PID    udprow;
        MIB_TCP6ROW_OWNER_PID   tcp6row;
        MIB_UDP6ROW_OWNER_PID   udp6row;
    };
    dlink       net_link;
}NETSTAT_ENTRY, *PNETSTAT_ENTRY;

#endif /*_COMMON_STRUCT_H*/