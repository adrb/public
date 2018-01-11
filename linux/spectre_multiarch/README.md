
This tool can detect whether your CPU is vulnerable to Spectre attack
and can be easily exploited. It's just slightly modified version
of original PoC published in [1].

Main diffrence is, that now it's architecture idependent and allows you
to test multiple architecures for eg. ARM (and of course x86).

One disadvantage is that, that you first need to find your threshold
value and table size, which guarantees stable results.
Second one is that, since now we need to "manually" flush L1 cache,
it's much slower than clflush on x86

Usage:

$ ./build
$ ./m $(( 16 * 1024 * 1024 ))
Cached average read time: 26
Non-cached average read time: 38
No clear prediction!

Now you see, that you need increase flush table size, so:

$ ./m $(( 32 * 1024 * 1024 ))
Cached average read time: 27
Non-cached average read time: 744
Suggested threshold: 54

Seems fine. Next, update accordingly CACHE_HIT_THRESHOLD and CACHE_MEM_FLUSH
in spectre.c and check your vulnerability:

$ ./build
$ ./s
Reading 40 bytes:
Reading at malicious_x = 0xffffffffffdff2c8... Success: 0x54=’T’ score=2
Reading at malicious_x = 0xffffffffffdff2c9... Success: 0x68=’h’ score=2
Reading at malicious_x = 0xffffffffffdff2ca... Success: 0x65=’e’ score=2
Reading at malicious_x = 0xffffffffffdff2cb... Success: 0x20=’ ’ score=2

Notes:

On newer Linux kernel versions, you may need to unlock perf counters first:

# echo 1 > /proc/sys/kernel/perf_event_paranoid

-- 
[1] https://spectreattack.com/spectre.pdf

