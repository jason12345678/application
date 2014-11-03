#ifndef     _SMART_NET_H_
#define     _SMART_NET_H_
#if _MSC_VER > 1000
#pragma once
#endif

#include "common_struct.h"

LPCTSTR get_error_msg(DWORD code);
class proc_prio;

class smart_net{
public:
    smart_net();
    ~smart_net();
public:
    /*Initialize the atheros SmartNET engine. Return 0(ERROR_SUCCESS) if it
    succeeds. If it returns some error code, we can use get_error_msg(errCode)
    to retrieve the text error message. And we can use get_error_msg to
    retrieve error message for any other apis which return error code, too.*/
    DWORD   init();
    DWORD   check_gbt_ssid();
    DWORD   check_acer_ssid();
    BOOL    is_smartnet_running();
    void    set_avail_bandwidth(UINT32 rx_avail_kbps);
    /*set_avail_bandwidth() and auto_detect_bw_enabled(), the latest call will
    be valid. set_avail_bandwidth() will disable auto-detecting.*/
    DWORD   auto_detect_bw_enabled(BOOL enable);
    /*The priority index must be unique, that means different application owns same
    priority isn't allowed. priority 1 the highest priority, 2 the second 
    priority and so on. <=0 means removing this proc from high priority.*/
    DWORD   set_proc_prio(const TCHAR *proc_name, int prio_idx);
    /*We should call apply_prio_list() after all set_proc_prio() calling
    finished to active these priority lists.*/
    void    apply_prio_list();
    /*Clear all priority of applications which have been set. So after this
    call, we should call set_proc_prio again to set priority for application.*/
    void    clearall_prio();
    /*check_proc_change should be called every about 2s, to check whether the
    running applications have changed. And this function will adjust the
    filters per the priority of applications. Return TRUE if running
    applications changed.*/
    BOOL    check_proc_change(BOOL *pid_changed = NULL,
                              BOOL *net_changed = NULL);

    /*In multi-thread application, we should call lock() first before we call
    the following interfaces. And call unlock() after we call that.*/
    /*put m_curr_pid_entry to the head of pid lists.*/
    void    proc_seek_head();
    /*get_next_proc() will retrieve proc name and pid. rx_prio and tx_prio
    will retrieve its priority. If you wanna query the bandwidth used by this
    application, you should supply rx_Bps and tx_Bps.*/
    DWORD   get_next_proc(TCHAR  *proc_name,
                          INT32  proc_name_len,
                          DWORD  *pid,
                          UINT32 *rx_prio,
                          UINT32 *tx_prio,
                          UINT32 *rx_Bps = NULL,
                          UINT32 *tx_Bps = NULL);
    /*put m_curr_netstat_entry to the head of NETSTAT_ENTRY of current pid.*/
    void    netentry_seek_head();
    /*This will traverse all the NETSTAT_ENTRYs of the the application
    we get from get_next_proc(). That means you can traverse the udp/tcp
    connections created by the application located by get_next_proc().*/
    DWORD   get_next_netentry(NETSTAT_ENTRY *net_entry);

    /*bandwidth statistics per pid.*/
    /*Enable/disable statis: Should be called protecting with pid list lock.*/
    void    enable_bw_statis(BOOL enable);
    DWORD   get_bw_statis_by_pid(
        IN  DWORD  pid,
        OUT UINT32 *rx_Bps,
        OUT UINT32 *tx_Bps);

    /*If your application uses multi-thread, you should call lock() before 
    you call the following APIs because the following APIs may affect the 
    NETSTAT_ENTRY. And you should call unlock() before your thread exits.*/
    void    lock();
    void    unlock();
    DWORD get_rx_available_bw();

private:
    proc_prio   *m_proc_prio;

};
typedef class smart_net SMART_NET;

#endif /*_SMART_NET_H_*/