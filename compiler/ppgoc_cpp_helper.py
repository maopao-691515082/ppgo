#coding=utf8

import ctypes

so = None

#输入代码中float字面量的token串，返回解析成功后转换成十六进制long double串，解析失败返回None
def parse_long_double(s):
    assert '\0' not in s
    so.ParseLongDouble.restype = ctypes.c_char_p
    return so.ParseLongDouble(s)
