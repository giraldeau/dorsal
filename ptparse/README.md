# PT Parse

Intel PT is now [integrated with perf](http://lxr.free-electrons.com/source/tools/perf/Documentation/intel-pt.txt) and exposed as a PMU for usage. As an example, on supported CPUs, `intel_pt` event can be enabled for recording hardware trace (userspace) while `ls` executes:

```
perf record -e intel_pt//u ls
```

The hardware trace is written in a auxilliary perf buffer the size of which is configurable. The buffer is copied to disk and becomes part of the  `perf.data` file. The `ptparse` tool allows the raw hardware trace data to be extracted from that file.

_The `ptparse` tool is a totally untested and hastily written piece of software, meant just to learn internals of Perf and its data file format._

## Usage
```
$ perf record -e intel_pt//u ls
$ ./ptparse [path to perf.data file] [extracted file.pt]
```
Without any arguments, `ptparse` expects a `perf.data` file containing AUX buff data and generates an `extracted.pt` file.

You can decode the `.pt` file using [`fastdecode`](https://github.com/andikleen/simple-pt/blob/master/fastdecode.c) written by Andi Kleen. You can also do `perf report -D` to dump raw data from the perf.data file and check raw bytes.

## Extra Fun
We basically learnt about the `perf.data` file from opening it directly in a hex editor such as `bless` as well as using `perf report -D` on the `perf.data` file. We analysed how Perf builds the file and how events are written. The userspace perf conterpart is what one might call -- _quite tight_ in terms of accesibility from usersapce. Using GDB is highly recommended to uncover the Perf magic :D In our trials, we mmap the file and iterate over the events till we get all `PERF_RECORD_AUXTRACE` events which are per-CPU. Then, based on each `aux->cpu`, just dump the `auxtrace_event->size` number of bytes after removing the event header and save it to a file. As more `PERF_RECORD_AUXTRACE` events are encountered, we just find aux->cpu and append the binary data to the corresponding file. This _may be_ a highly volatile way but the courageous among you can use this as a template and try to actually go over all events till you find events of your interest. If you have some suggestions or a better way, let us know. :)

_Suchakra Sharma <suchakrapani.sharma@polymtl.ca>_
_Francis Giraldeau <francis.giraldeau@gmail.com>_
