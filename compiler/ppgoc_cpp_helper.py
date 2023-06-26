#coding=utf8

import os, ctypes
import ppgoc_env, ppgoc_util

so = None

def init():
    global so

    rc = os.system("make -C %s/cpp_helper >/dev/null" % ppgoc_env.compiler_dir)
    if rc != 0:
        ppgoc_util.exit("构建cpp_helper失败")
    so = ctypes.CDLL("%s/ppgoc_helper.so" % ppgoc_env.tmp_dir)

#输入代码中float字面量的token串，返回解析成功后转换成十六进制long double串，解析失败返回None
def parse_long_double(s):
    assert '\0' not in s
    so.ParseLongDouble.restype = ctypes.c_char_p
    return so.ParseLongDouble(s)
