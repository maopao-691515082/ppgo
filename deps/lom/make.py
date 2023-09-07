#codeing=utf8

import sys, os, hashlib

def file_hash(fn):
    return hashlib.md5(open(fn, "rb").read()).hexdigest().upper()

HASHES_FN = "hashes"
HASHES_FILE_END_LINE = "<HASHES FILE END>"

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

def do_cmd(cmd):
    sys.stdout.write(cmd + "\n")
    ret = os.system(cmd)
    if ret != 0:
        sys.exit(1)

lib_objs = []
test_objs = []
for fn, h in cpp_hashes.items():
    assert fn.endswith(".cpp")
    obj_fn = fn[: -4] + ".o"
    if header_changed or h != last_cpp_hashes.get(fn):
        do_cmd("%s -c -o tmpobjs/%s %s ../%s" % (cxx, obj_fn, cxx_flags, fn))
    else:
        do_cmd("cp last_tmpobjs/%s tmpobjs/%s" % (obj_fn, obj_fn))
    if fn.startswith("src/"):
        lib_objs.append("tmpobjs/" + obj_fn)
    else:
        assert fn.startswith("test/")
        test_objs.append("tmpobjs/" + obj_fn)

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
