#ifndef __SCHED__CREDIT__H__
#define __SCHED__CREDIT__H__

#include <xen/config.h>
#include <xen/init.h>
#include <xen/lib.h>
#include <xen/sched.h>
#include <xen/domain.h>
#include <xen/delay.h>
#include <xen/event.h>
#include <xen/time.h>
#include <xen/perfc.h>
#include <xen/sched-if.h>
#include <xen/softirq.h>
#include <asm/atomic.h>
#include <xen/errno.h>
#include <xen/keyhandler.h>

/*
 * Physical CPU
 */
struct csched_pcpu {
    struct list_head runq;
    uint32_t runq_sort_last;
    struct timer ticker;
    unsigned int tick;
    unsigned int idle_bias;
};

/*
 * Virtual CPU
 */
struct csched_vcpu {
    struct list_head runq_elem;
    struct list_head active_vcpu_elem;
    struct csched_dom *sdom;
    struct vcpu *vcpu;
    atomic_t credit;
    s_time_t start_time;   /* When we were scheduled (used for credit) */
    uint16_t flags;
    int16_t pri;
#ifdef CSCHED_STATS
    struct {
        int credit_last;
        uint32_t credit_incr;
        uint32_t state_active;
        uint32_t state_idle;
        uint32_t migrate_q;
        uint32_t migrate_r;
    } stats;
#endif
	/*add by wei*/
	struct {
		uint64_t over_schedule;
		uint64_t under_schedule;
		uint64_t boost_schedule;
		uint64_t batch_schedule;
	} num_pri_schedule;

	struct {
		uint64_t over;
		uint64_t under;
		uint64_t boost;
		uint64_t batch;
	} num_pri;
	
};


/*
 * Domain Type
 */
#define NORMAL 0
#define BATCH 1

//# define NOT_RUN_THRESHOLD_MS 500
# define NOT_RUN_THRESHOLD_NS 500000000

/*
 * Domain
 */
struct csched_dom {
    struct list_head active_vcpu;
    struct list_head active_sdom_elem;
    struct domain *dom;
    uint16_t active_vcpu_count;
    uint16_t weight;
    uint16_t cap;
	/*added by wei*/
	uint16_t batch_threshold_vcpu_count;
	uint16_t vm_type;
	/*lock for batch_threshold_vcpu_count*/
    spinlock_t lock;
};

/*
 * System-wide private data
 */
struct csched_private {
    /* lock for the whole pluggable scheduler, nests inside cpupool_lock */
    spinlock_t lock;
    struct list_head active_sdom;
    uint32_t ncpus;
    struct timer  master_ticker;
    unsigned int master;
    cpumask_var_t idlers;
    cpumask_var_t cpus;
    uint32_t weight;
    uint32_t credit;
    int credit_balance;
    uint32_t runq_sort;
    unsigned ratelimit_us;
    /* Period of master and tick in milliseconds */
    unsigned tslice_ms, tick_period_us, ticks_per_tslice;
    unsigned credits_per_tslice;
};

/*
 * Priorities
 */
#define CSCHED_PRI_TS_BOOST      0      /* time-share waking up */
#define CSCHED_PRI_TS_UNDER     -1      /* time-share w/ credits */
#define CSCHED_PRI_TS_OVER      -2      /* time-share w/o credits */
#define CSCHED_PRI_TS_BATCH     -3      /* time-share only no higher priority vcpu runnable */
#define CSCHED_PRI_IDLE         -64     /* idle */

#endif
