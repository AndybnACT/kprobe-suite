## Probe and Play
Here comes a linker hack that help reduce the effort to play with kprobes on Linux. The `struct kprobe` are self-maintained. So you don't have to mess around with a bunch of kprobe's info, their names, and keeping them into an array before submitting them to the kprobe subsystem. 

A simple example is listed in `kprobe.c`.

## TODO
 - `kretprobe`
 - provide macro to disable/enable kprobes

