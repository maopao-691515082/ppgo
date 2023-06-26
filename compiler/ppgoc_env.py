#coding=utf8

import sys, os
import ppgoc_util

ppgo_dir = None
compiler_dir = None
deps_dir = None
slib_dir = None
ulib_dir = None
runtime_dir = None
tmp_dir = None
tmp_out_dir = None

main_mn = None
out_dir = None
out_confs = {}

def init(main_mod_path):
    global ppgo_dir, compiler_dir, deps_dir, slib_dir, ulib_dir, runtime_dir, tmp_dir, tmp_out_dir
    global main_mn, out_dir

    THIS_SCRIPT_NAME_SUFFIX = "/compiler/ppgoc.py"
    this_script_name = ppgoc_util.abs_path(sys.argv[0])
    assert this_script_name.endswith(THIS_SCRIPT_NAME_SUFFIX)
    ppgo_dir = this_script_name[: -len(THIS_SCRIPT_NAME_SUFFIX)]

    compiler_dir = ppgo_dir + "/compiler"
    deps_dir = ppgo_dir + "/deps"
    slib_dir = ppgo_dir + "/lib"
    assert os.path.isdir(slib_dir)
    ulib_dir = ppgo_dir + "/ulib"
    runtime_dir = ppgo_dir + "/runtime"
    tmp_dir = ppgo_dir + "/tmp"
    for dn, d in ("用户库目录", ulib_dir), ("临时目录", tmp_dir):
        if not os.path.isdir(d):
            try:
                os.makedirs(d)
            except OSError:
                ppgoc_util.exit("创建%s[%s]失败" % (dn, d))

    tmp_out_dir = tmp_dir + "/out"
    if not os.path.isdir(tmp_out_dir):
        try:
            os.makedirs(tmp_out_dir)
        except OSError:
            ppgoc_util.exit("创建临时输出目录[%s]失败" % tmp_out_dir)

    first_level_std_mod_set = set()
    for fn in os.listdir(slib_dir):
        if os.path.isdir(slib_dir + "/" + fn):
            first_level_std_mod_set.add(fn)
    assert ppgoc_util.MN_BUILTINS in first_level_std_mod_set
    for fn in os.listdir(ulib_dir):
        if os.path.isdir(ulib_dir + "/" + fn) and fn in first_level_std_mod_set:
            ppgoc_util.exit("用户模块[%s]和标准库同名模块冲突" % fn)

    for d in slib_dir, ulib_dir:
        if main_mod_path.startswith(d + "/"):
            main_mn = main_mod_path[len(d) + 1 :]
            break
    else:
        ppgoc_util.exit("主模块路径不存在于标准库或用户库[%s]" % main_mod_path)

    out_dir = tmp_out_dir + "/" + main_mn
