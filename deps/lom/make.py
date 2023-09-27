#codeing=utf8

import sys, os, hashlib, subprocess

def file_hash(fn):
    return hashlib.md5(open(fn, "rb").read()).hexdigest().upper()

HASHES_FN = "hashes"
HASHES_FILE_END_LINE = "<HASHES FILE END>"

CXX_PARALLEL_COUNT = 8

cxx, cxx_flags, ar, ar_flags, ld, ld_flags, ld_std_lib_flags = sys.argv[1:]

dirs = set()
header_hashes = {}
cpp_hashes = {}
for src_d in ("include", "src", "test"):
    for d, _, fs in os.walk(src_d):
        dirs.add(d)
        for f in fs:
            f = d + "/" + f
            if f.endswith(".h"):
                header_hashes[f] = file_hash(f)
            if f.endswith(".cpp"):
                assert src_d != "include"
                cpp_hashes[f] = file_hash(f)

cases = os.listdir("test")

os.chdir("build")

#try read src hashes of last compiling
last_header_hashes = {}
last_cpp_hashes = {}
last_hashes_fn = "last_tmpobjs/" + HASHES_FN
if os.path.isfile(last_hashes_fn):
    lines = list(open(last_hashes_fn))
    if lines and lines[-1] == HASHES_FILE_END_LINE + "\n":
        for line in lines[: -1]:
            fn, h = line.split()
            if fn.endswith(".h"):
                last_header_hashes[fn] = h
            elif fn.endswith(".cpp"):
                last_cpp_hashes[fn] = h
            else:
                raise Exception("unknown src file `%s` in `%s`" % (fn, last_hashes_fn))

header_changed = header_hashes != last_header_hashes

for d in dirs:
    d = "tmpobjs/" + d
    if not os.path.isdir(d):
        os.makedirs(d)

def do_cmd(cmd, show = True):
    if show:
        sys.stdout.write(cmd + "\n")
    ret = os.system(cmd)
    if ret != 0:
        sys.exit(1)

cxx_cmds = []
lib_objs = []
test_objs = []
for fn, h in sorted(cpp_hashes.items(), key = lambda x: (int(x[0].startswith("src/")), x[0], x[1])):
    assert fn.endswith(".cpp")
    obj_fn = fn[: -4] + ".o"
    if header_changed or h != last_cpp_hashes.get(fn):
        cxx_cmds.append("%s -c -o tmpobjs/%s %s ../%s" % (cxx, obj_fn, cxx_flags, fn))
    else:
        do_cmd("cp last_tmpobjs/%s tmpobjs/%s" % (obj_fn, obj_fn), show = False)
    if fn.startswith("src/"):
        lib_objs.append("tmpobjs/" + obj_fn)
    else:
        assert fn.startswith("test/")
        test_objs.append("tmpobjs/" + obj_fn)

#parallel compiling
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
            sys.stderr.write(p.stderr.read())
            ok = False
    if not ok:
        sys.exit(1)

f = open("tmpobjs/" + HASHES_FN, "w")
for fn, h in list(header_hashes.items()) + list(cpp_hashes.items()):
    f.write("%s\t%s\n" % (fn, h))
f.write(HASHES_FILE_END_LINE + "\n")
f.close()

do_cmd("%s %s lom/lib/liblom.a %s" % (ar, ar_flags, " ".join(lib_objs)))

for case in cases:
    do_cmd(
        "%s -o test/%s %s %s lom/lib/liblom.a %s" %
        (ld, case, ld_flags,
         " ".join([obj_fn for obj_fn in test_objs if obj_fn.startswith("tmpobjs/test/%s/" % case)]),
         ld_std_lib_flags))
