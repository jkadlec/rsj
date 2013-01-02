#ifndef DEBUG_H
#define DEBUG_H

//#define TEST_DEBUG
//#define CALC_DEBUG
//efine THREADING_DEBUG
//#define RING_DEBUG

#ifdef TEST_DEBUG
#define dbg_test(msg...) printf(msg)
#define dbg_test_hex(data, len)  hex_log(LOG_ZONE, (data), (len))
#define dbg_test_exec(cmds) do { cmds } while (0)
#else
#define dbg_test(msg...)
#define dbg_test_hex(data, len)
#define dbg_test_exec(cmds)
#endif

#ifdef CALC_DEBUG
#define dbg_calc(msg...) printf(msg)
#define dbg_calc_hex(data, len)  hex_log(LOG_ZONE, (data), (len))
#define dbg_calc_exec(cmds) do { cmds } while (0)
#else
#define dbg_calc(msg...)
#define dbg_calc_hex(data, len)
#define dbg_calc_exec(cmds)
#endif

#ifdef THREADING_DEBUG
#define dbg_threading(msg...) printf(msg)
#define dbg_threading_hex(data, len)  hex_log(LOG_ZONE, (data), (len))
#define dbg_threading_exec(cmds) do { cmds } while (0)
#else
#define dbg_threading(msg...)
#define dbg_threading_hex(data, len)
#define dbg_threading_exec(cmds)
#endif

#ifdef RING_DEBUG
#define dbg_ring(msg...) printf(msg)
#define dbg_ring_hex(data, len)  hex_log(LOG_ZONE, (data), (len))
#define dbg_ring_exec(cmds) do { cmds } while (0)
#else
#define dbg_ring(msg...)
#define dbg_ring_hex(data, len)
#define dbg_ring_exec(cmds)
#endif

#endif // DEBUG_H
