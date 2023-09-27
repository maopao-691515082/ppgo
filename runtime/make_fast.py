#coding=utf8

import sys, os, hashlib, subprocess

def file_hash(fn):
    return hashlib.md5(open(fn).read()).hexdigest().upper()

HASHES_FN = "hashes"
HASHES_FILE_END_LINE = "<HASHES FILE END>"

CXX_PARALLEL_COUNT = 8

cxx, cxx_flags, ld_flags, out_fn, oc_dir = sys.argv[1 :]

h_file_hashes = {}
cpp_file_hashes = {}
for fn in os.listdir("."):
    if fn.endswith(".h"):
        h_file_hashes[fn] = file_hash(fn)
    elif fn.endswith(".cpp"):
        cpp_file_hashes[fn] = file_hash(fn)

oc_h_file_hashes = {}
oc_cpp_file_hashes = {}
oc_file_hash_fn = oc_dir + "/" + HASHES_FN
if os.path.isfile(oc_file_hash_fn):
    lines = list(open(oc_file_hash_fn))
    if lines and lines[-1] == HASHES_FILE_END_LINE + "\n":
        for line in lines[: -1]:
            fn, h = line.split()
            if fn.endswith(".h"):
                oc_h_file_hashes[fn] = h
            elif fn.endswith(".cpp"):
                oc_cpp_file_hashes[fn] = h
            else:
                raise Exception("unknown file `%s` in obj-cache file-hashes `%s`" % (fn, oc_file_hash_fn))

h_file_changed = h_file_hashes != oc_h_file_hashes

cxx_cmds = []
for fn, h in cpp_file_hashes.items():
    assert fn.endswith(".cpp")
    obj_fn = fn[: -4] + ".o"
    if h_file_changed or h != oc_cpp_file_hashes.get(fn):
        cxx_cmds.append("%s -c -o %s %s %s" % (cxx, obj_fn, cxx_flags, fn))
    else:
        cmd = "cp %s/%s ./" % (oc_dir, obj_fn)
        print cmd
        ret = os.system(cmd)
        if ret != 0:
            sys.exit(1)

for i in range(0, len(cxx_cmds), CXX_PARALLEL_COUNT):
    ccs = cxx_cmds[i : i + CXX_PARALLEL_COUNT]
    ps = []
    for cc in ccs:
        sys.stdout.write(cc + "\n")
        p = subprocess.Popen(cc, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        ps.append(p)
    ok = True
    for p in ps:
        rc = p.wait()
        if rc != 0:
            sys.stderr.write(p.stderr.read().decode("utf-8"))
            ok = False
    if not ok:
        sys.exit(1)

cmd = "%s -o %s *.o %s" % (cxx, out_fn, ld_flags)
print cmd
ret = os.system(cmd)
if ret != 0:
    sys.exit(1)

f = open(HASHES_FN, "w")
for fn, h in h_file_hashes.items() + cpp_file_hashes.items():
    print >> f, fn, h
print >> f, HASHES_FILE_END_LINE
f.close()
