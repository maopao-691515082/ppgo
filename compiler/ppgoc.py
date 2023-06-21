#coding=utf8

import sys, getopt, os, time, itertools
import ppgoc_util, ppgoc_mod, ppgoc_token, ppgoc_out

def main():
    THIS_SCRIPT_NAME_SUFFIX = "/compiler/ppgoc.py"
    this_script_name = ppgoc_util.abs_path(sys.argv[0])
    assert this_script_name.endswith(THIS_SCRIPT_NAME_SUFFIX)
    ppgo_dir = this_script_name[: -len(THIS_SCRIPT_NAME_SUFFIX)]

    def _show_usage_and_exit():
        print >> sys.stderr, (
            "python2 ppgoc.py [-v] [-o <OUT_BIN>] [--run] <MAIN_MODULE_SPEC> <ARGS_FOR_RUNNING>")
        sys.exit(1)

    try:
        opts, args = getopt.getopt(sys.argv[1 :], "vo:", ["run"])
    except getopt.GetoptError:
        _show_usage_and_exit()
    opts = dict(opts)
    if "-v" in opts:
        ppgoc_util.enable_vmode()
    ppgoc_util.vlog("开始")
    out_bin = opts.get("-o")
    need_run = "--run" in opts

    if len(args) < 1:
        _show_usage_and_exit()
    main_mod_path = ppgoc_util.abs_path(args[0])
    if not os.path.isdir(main_mod_path):
        ppgoc_util.exit("无效的主模块路径[%s]：不存在或不是目录" % main_mod_path)
    args_for_run = args[1 :]
    if not need_run and args_for_run:
        _show_usage_and_exit()

    compiler_dir = ppgo_dir + "/compiler"
    slib_dir = ppgo_dir + "/lib"
    assert os.path.isdir(slib_dir)
    ulib_dir = ppgo_dir + "/ulib"
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

    ppgoc_mod.slib_dir = slib_dir
    ppgoc_mod.ulib_dir = ulib_dir

    if main_mod_path.startswith(slib_dir + "/"):
        main_mn = main_mod_path[len(slib_dir) + 1 :]
    elif main_mod_path.startswith(ulib_dir + "/"):
        main_mn = main_mod_path[len(ulib_dir) + 1 :]
    else:
        ppgoc_util.exit("主模块路径不存在于标准库或用户库[%s]" % main_mod_path)

    assert ppgoc_mod.is_valid_mn(main_mn)

    ppgoc_util.vlog("初始化完毕，主模块'%s'" % main_mn)

    ppgoc_mod.precompile_mod(ppgoc_util.MN_BUILTINS)
    for name in "strings", "vecs", "maps", "sets":
        ppgoc_mod.precompile_mod(ppgoc_util.MN_BUILTINS + "/" + name)
    ppgoc_mod.precompile_mod("os")  #for args
    main_mod = ppgoc_mod.precompile_mod(main_mn)

    ppgoc_util.vlog("所有模块预处理完毕")

    compile_start_time = time.time()

    for m in ppgoc_mod.mods.itervalues():
        m.check_type()

    for m in ppgoc_mod.mods.itervalues():
        m.check_init_func()

    main_mod.check_main_func()

    for m in ppgoc_mod.mods.itervalues():
        m.find_impled_intfs_of_clses()

    for m in ppgoc_mod.mods.itervalues():
        m.compile()

    ppgoc_util.vlog("语法编译完毕，耗时%.2f秒" % (time.time() - compile_start_time))

    ppgoc_out.main_mn = main_mod.name
    ppgoc_out.out_dir = tmp_out_dir + "/" + main_mn
    ppgoc_out.runtime_dir = ppgo_dir + "/runtime"
    ppgoc_out.confs = {
        #confs
    }
    ppgoc_out.output(out_bin, need_run, args_for_run)

if __name__ == "__main__":
    main()
