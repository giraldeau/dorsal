# perf python script
import os
import sys

sys.path.append(os.environ['PERF_EXEC_PATH'] + \
    '/scripts/python/Perf-Trace-Util/lib/Perf/Trace')

from perf_trace_context import *
from Core import *
from cStringIO import StringIO

g = {} # global state

outfile = None

def trace_begin():
    # FIXME: open a file by TID
    global outfile
    outfile = open("/tmp/flame.txt", "w")

def sched__sched_switch(**args):
    pass
    #print "sched_switch"

"""
process_event
{ 'attr': binary,
  'symbol': 'pt_event_del',
  'sample': {
    'ip': 18446744071579081744L,
    'pid': 4754,
    'period': 2L,
    'time': 320590626721L,
    'tid': 4754,
    'cpu': 0
  },
  'dso': '[kernel.kallsyms]',
  'comm': 'sleep',
  'ev_name': 'instructions',
  'raw_buf': '',
  'callchain': [] // defined with --itrace=ig
}
"""

"""
 'callchain': [{'dso': '[kernel.kallsyms]',
                'ip': 3988579L,
                'sym': {'binding': 1,
                        'end': 3988581L,
                        'name': '__this_cpu_preempt_check',
                        'start': 3988560L}},
"""

import pprint
state = {}
prev_stack = None
prev_time = 0
cnt = 0

def get_time(args, default):
    return args.get("sample", {}).get("time", default)
    
def process_event(args):
    
    global prev_stack
    global prev_time
    global outfile
    global cnt
    
    #pprint.pprint(args)
    
    callchain = args.get("callchain", [])
    
    top = []
    sym = "do_nanosleep"
    for i, item in enumerate(callchain):
        if item.get('sym', {}).get('name', "") == sym:
            top = callchain[i:]
            break;
    stack = []
    for item in top:
        stack.append(item['sym']['name'])

    if len(stack) > 0:
        # init
        if prev_stack == None:
            prev_stack = stack
            prev_time = get_time(args, prev_time + 1)
        if len(stack) != len(prev_stack):
            now = get_time(args, prev_time + 1)
            delta = now - prev_time + 1
            line = "{};{} {}\n".format(cnt, ";".join(prev_stack), delta)
            outfile.write(line)
            prev_stack = stack
            prev_time = now
            #cnt += 1

def trace_end():
    global outfile
    outfile.close()

def trace_unhandled(event_name, context, event_fields_dict):
    pass
    #print ' '.join(['%s=%s'%(k,str(v))for k,v in sorted(event_fields_dict.items())])
