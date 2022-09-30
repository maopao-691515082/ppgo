#coding=utf8

import os, sys, time
from StringIO import StringIO

MN_BUILTINS = "__builtins"

_vmode = False
_vmode_indent_count = 0

def enable_vmode():
    global _vmode
    _vmode = True

def inc_vmode_indent():
    global _vmode_indent_count
    assert _vmode_indent_count >= 0
    _vmode_indent_count += 1

def dec_vmode_indent():
    global _vmode_indent_count
    _vmode_indent_count -= 1
    assert _vmode_indent_count >= 0

def vlog(msg):
    if _vmode:
        print time.strftime("ppgoc: [%H:%M:%S]") + "  " * _vmode_indent_count, msg

def exit(msg):
    print >> sys.stderr, "错误：" + msg
    print >> sys.stderr
    sys.exit(1)

#warning信息不实时输出，而是记录在set中（顺便去重），在编译之后统一输出
_warning_set = set()
def warning(msg):
    f = StringIO()
    print >> f, "警告：" + msg
    _warning_set.add(f.getvalue())

def output_all_warnings():
    for w in _warning_set:
        print >> sys.stderr, w

def raise_bug():
    raise Exception("BUG")

class OrderedDict:
    def __init__(self):
        self.l = []
        self.d = {}

    def __iter__(self):
        return iter(self.l)

    def __len__(self):
        return len(self.l)

    def __nonzero__(self):
        return len(self) > 0

    def __getitem__(self, k):
        return self.d[k]

    def __setitem__(self, k, v):
        if k not in self.d:
            self.l.append(k)
        self.d[k] = v

    def itervalues(self):
        for k in self.l:
            yield self.d[k]

    def iteritems(self):
        for k in self.l:
            yield k, self.d[k]

    def key_at(self, idx):
        return self.l[idx]

    def value_at(self, idx):
        return self.d[self.l[idx]]

    def copy(self):
        od = OrderedDict()
        for name in self:
            od[name] = self[name]
        return od

class OrderedSet:
    def __init__(self):
        self.d = OrderedDict()

    def __iter__(self):
        return iter(self.d)

    def __len__(self):
        return len(self.d)

    def __nonzero__(self):
        return len(self) > 0

    def add(self, k):
        self.d[k] = None

    def key_at(self, idx):
        return self.d.key_at(idx)

    def value_at(self, idx):
        return self.d.value_at(idx)

    def copy(self):
        os = OrderedSet()
        os.d = self.d.copy()
        return os

_id = 0
def new_id():
    global _id
    _id += 1
    return _id

def open_src_file(fn):
    f = open(fn)
    f.seek(0, os.SEEK_END)
    if f.tell() > 1024 ** 2:
        exit("源代码文件[%s]过大" % fn)
    f.seek(0, os.SEEK_SET)
    f_cont = f.read()
    try:
        f_cont.decode("utf8")
    except UnicodeDecodeError:
        exit("源代码文件[%s]不是utf8编码" % fn)
    if "\r" in f_cont:
        warning("源代码文件[%s]含有回车符‘\\r’" % fn)
    f.seek(0, os.SEEK_SET)
    return f

def abs_path(path):
    return os.path.abspath(os.path.expanduser(path))
