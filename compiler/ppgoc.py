#coding=utf8

import sys, getopt, os, time
import ppgoc_env, ppgoc_util, ppgoc_mod, ppgoc_out, ppgoc_cpp_helper

def parse_opts():
    def show_usage_and_exit():
        print >> sys.stderr, (
            "python2 ppgoc.py [-v] [-o <OUT_BIN>] [--run] <MAIN_MODULE_SPEC> <ARGS_FOR_RUNNING>")
        sys.exit(1)

    try:
        opts, args = getopt.getopt(sys.argv[1 :], "vo:", ["run"])
    except getopt.GetoptError:
        show_usage_and_exit()
    opts = dict(opts)
    if "-v" in opts:
        ppgoc_util.enable_vmode()
    out_bin = opts.get("-o")
    need_run = "--run" in opts

    if len(args) < 1:
        show_usage_and_exit()
    main_mod_path = ppgoc_util.abs_path(args[0])
    if not os.path.isdir(main_mod_path):
        ppgoc_util.exit("无效的主模块路径[%s]：不存在或不是目录" % main_mod_path)
    args_for_run = args[1 :]
    if not need_run and args_for_run:
        show_usage_and_exit()

    return out_bin, need_run, main_mod_path, args_for_run

def main():
    out_bin, need_run, main_mod_path, args_for_run = parse_opts()

    ppgoc_util.vlog("开始")

    ppgoc_env.init(main_mod_path)
    ppgoc_cpp_helper.init()

    assert ppgoc_mod.is_valid_mn(ppgoc_env.main_mn)

    ppgoc_util.vlog("初始化完毕，主模块'%s'" % ppgoc_env.main_mn)

    ppgoc_mod.precompile_mod(ppgoc_util.MN_BUILTINS)
    for name in "strings", "bytes", "bytes_views", "vecs", "vec_views", "maps", "sets":
        ppgoc_mod.precompile_mod(ppgoc_util.MN_BUILTINS + "/" + name)
    ppgoc_mod.precompile_mod("os")
    main_mod = ppgoc_mod.precompile_mod(ppgoc_env.main_mn)

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

    ppgoc_out.output(out_bin, need_run, args_for_run)

if __name__ == "__main__":
    main()
