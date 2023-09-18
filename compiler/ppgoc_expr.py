#coding=utf8

import re
import ppgoc_token, ppgoc_mod, ppgoc_type, ppgoc_util

UNARY_OP_SET = set(["~", "!", "neg", "pos"])
BINOCULAR_OP_SET = ppgoc_token.BINOCULAR_OP_SYM_SET
OP_PRIORITY_LIST = [["if", "else", "if-else"],
                    ["||"],
                    ["&&"],
                    ["|"],
                    ["^"],
                    ["&"],
                    ["==", "!="],
                    ["<", "<=", ">", ">="],
                    ["<<", ">>"],
                    ["+", "-"],
                    ["*", "/", "%"],
                    ["~", "!", "neg", "pos"]]
OP_PRIORITY_MAP = {}
for i in xrange(len(OP_PRIORITY_LIST)):
    for op in OP_PRIORITY_LIST[i]:
        OP_PRIORITY_MAP[op] = i
del i, op

class Expr:
    def __init__(self, op, arg, tp):
        self.op = op
        self.arg = arg
        self.tp = tp

        self.is_lvalue = op in ("gv", "lv", "vec[]", "map[]", ".")
        if op == "gv":
            gv = self.arg
            if "final" in gv.decrs:
                self.is_lvalue = False

        self.pos_info = None

class ParseStk:
    #解析表达式时使用的栈
    def __init__(self, t, mod, cls, fom):
        self.t = t
        self.mod = mod
        self.cls = cls
        self.fom = fom

        self.stk = []
        self.op_stk = []

    def push_op(self, op):
        #弹出所有优先级高的运算
        while self.op_stk:
            if OP_PRIORITY_MAP[self.op_stk[-1]] > OP_PRIORITY_MAP[op]:
                self.pop_top_op()
            elif OP_PRIORITY_MAP[self.op_stk[-1]] < OP_PRIORITY_MAP[op]:
                break
            else:
                #同优先级看结合性
                if op in UNARY_OP_SET:
                    #单目运算符右结合
                    break
                if op in ("if", "else"):
                    assert self.op_stk[-1] in ("if", "if-else")
                    if self.op_stk[-1] == "if" and op == "else":
                        #匹配了三元运算符，在外面统一处理合并
                        break
                    self.t.syntax_err("禁止多个'if-else'表达式直接混合运算，请加括号")
                self.pop_top_op()
        if op == "else":
            if self.op_stk and self.op_stk[-1] == "if":
                self.op_stk[-1] = "if-else"
                return
            self.t.syntax_err("非法的表达式，存在未匹配'if'的'else'")
        self.op_stk.append(op)

    def pop_top_op(self):
        op = self.op_stk.pop()
        if op in UNARY_OP_SET:
            #单目运算符
            if len(self.stk) < 1:
                self.t.syntax_err("非法的表达式")
            e = self.stk.pop()
            if op in ("neg", "pos"):
                if not e.tp.is_number_type:
                    self.t.syntax_err("非法的表达式：类型'%s'不可做正负运算" % e.tp)
            elif op == "!":
                if not e.tp.is_bool_type:
                    self.t.syntax_err("非法的表达式：类型'%s'不可做'!'运算" % e.tp)
            elif op == "~":
                if not e.tp.is_integer_type:
                    self.t.syntax_err("非法的表达式：类型'%s'不可做'~'运算" % e.tp)
            else:
                ppgoc_util.raise_bug()
            self.stk.append(Expr(op, e, e.tp))

        elif op in BINOCULAR_OP_SET:
            #双目运算符
            if len(self.stk) < 2:
                self.t.syntax_err("非法的表达式")
            eb = self.stk.pop()
            ea = self.stk.pop()

            class _InvalidBinocularOp(Exception):
                pass

            try:
                normal_binocular_op = False

                if op in ("&&", "||"):
                    if not ea.tp.is_bool_type or not eb.tp.is_bool_type:
                        self.t.syntax_err("非法的表达式：运算'%s'的左右分量必须是bool型" % op)
                    tp = ppgoc_type.BOOL_TYPE
                elif op in ("==", "!="):
                    if ea.tp.is_nil:
                        if eb.tp.is_nil:
                            pass
                        elif eb.tp.can_convert_from(ea.tp):
                            ea = Expr("convert", (eb.tp, ea), eb.tp)
                        else:
                            raise _InvalidBinocularOp()
                    elif eb.tp.is_nil:
                        if ea.tp.can_convert_from(eb.tp):
                            eb = Expr("convert", (ea.tp, eb), ea.tp)
                        else:
                            raise _InvalidBinocularOp()
                    elif ea.tp != eb.tp:
                        if ea.tp.can_convert_from(eb.tp):
                            eb = Expr("convert", (ea.tp, eb), ea.tp)
                        elif eb.tp.can_convert_from(ea.tp):
                            ea = Expr("convert", (eb.tp, ea), eb.tp)
                        else:
                            raise _InvalidBinocularOp()
                    elif (
                        ea.tp.is_bool_type or ea.tp.is_number_type or ea.tp.is_str_type or ea.tp.is_any or
                        ea.tp.is_coi_type):
                        pass
                    else:
                        raise _InvalidBinocularOp()
                    tp = ppgoc_type.BOOL_TYPE
                elif op in ("+", "-", "*", "/", "%", "<", ">", "<=", ">="):
                    if ea.tp.is_str_type and eb.tp.is_str_type and op not in ("-", "*", "/", "%"):
                        normal_binocular_op = True
                    elif ea.tp.is_number_type and eb.tp.is_number_type and ea.tp == eb.tp:
                        normal_binocular_op = True
                    else:
                        raise _InvalidBinocularOp()
                    if op in ("<", ">", "<=", ">="):
                        tp = ppgoc_type.BOOL_TYPE
                    else:
                        tp = None
                elif op in ("&", "|", "^"):
                    if ea.tp.is_integer_type and eb.tp.is_integer_type and ea.tp == eb.tp:
                        normal_binocular_op = True
                    else:
                        raise _InvalidBinocularOp()
                    tp = None
                elif op in ("<<", ">>"):
                    if not (ea.tp.is_integer_type and eb.tp.is_integer_type):
                        raise _InvalidBinocularOp()
                    tp = ea.tp
                else:
                    ppgoc_util.raise_bug()

                if normal_binocular_op:
                    if tp is None:
                        tp = ea.tp
                assert tp is not None
                if op not in ("<<", ">>"):
                    assert ea.tp == eb.tp
                self.stk.append(Expr(op, (ea, eb), tp))

            except _InvalidBinocularOp:
                self.t.syntax_err("非法的表达式：类型'%s'和'%s'无法做'%s'运算" % (ea.tp, eb.tp, op))

        elif op == "if":
            self.t.syntax_err("非法的表达式，存在未匹配'else'的'if'")

        elif op == "if-else":
            #三元运算符
            if len(self.stk) < 3:
                self.t.syntax_err("非法的表达式")
            eb = self.stk.pop()
            e_cond = self.stk.pop()
            ea = self.stk.pop()
            if not e_cond.tp.is_bool_type:
                self.t.syntax_err(
                    "非法的表达式：'if-else'运算的条件运算分量类型需要是'bool'，不能是'%s'" % e_cond.tp)
            if ea.tp != eb.tp:
                if ea.tp != eb.tp:
                    self.t.syntax_err(
                        "非法的表达式：'if-else'运算的两个结果运算分量类型不同：'%s'和'%s'" %
                        (ea.tp, eb.tp))
            tp = ea.tp
            self.stk.append(Expr(op, (e_cond, ea, eb), tp))

        else:
            ppgoc_util.raise_bug()

    def push_expr(self, e):
        self.stk.append(e)

    def pop_expr(self):
        return self.stk.pop()

    def finish(self):
        while self.op_stk:
            self.pop_top_op()
        if len(self.stk) != 1:
            self.t.syntax_err("非法的表达式")
        e = self.stk.pop()
        e.pos_info = self.t, (self.mod if self.fom is None else self.fom)
        return e

def is_expr_end(t):
    if t.is_sym:
        if t.value in [")", "]", ",", ";", ":", "}", ".."]:
            return True
        if t.value in ppgoc_token.ASSIGN_SYM_SET:
            return True
    return False

class Parser:
    def __init__(self, tl, mod, fn, dep_mns, cls, fom):
        self.tl = tl
        self.mod = mod
        self.fn = fn
        self.dep_mns = dep_mns
        self.cls = cls
        self.fom = fom

    def parse(self, vars_stk, need_type):
        parse_stk = ParseStk(self.tl.peek(), self.mod, self.cls, self.fom)
        while True:
            t = self.tl.pop()

            if t.is_sym and t.value in ("~", "!", "+", "-"):
                #单目运算
                if t.value == "+":
                    op = "pos"
                elif t.value == "-":
                    op = "neg"
                else:
                    op = t.value
                parse_stk.push_op(op)
                continue

            if t.is_sym("("):
                #子表达式
                parse_stk.push_expr(self.parse(vars_stk, None))
                self.tl.pop_sym(")")
            elif t.is_reserved("func"):
                f = ppgoc_mod.Closure(self.mod, self.fn, t, self.tl, vars_stk, self.cls)
                parse_stk.push_expr(Expr("closure", f, ppgoc_type.from_closure(f)))
            elif t.is_name:
                if t.value in self.dep_mns:
                    m = ppgoc_mod.mods[self.dep_mns[t.value]]
                    self.tl.pop_sym(".")
                    t, name = self.tl.pop_name()
                    expr = self.parse_cfgv(m, t, name, vars_stk)
                    parse_stk.push_expr(expr)
                else:
                    for vars in reversed(vars_stk):
                        if t.value in vars:
                            #局部变量
                            tp = vars[t.value]
                            if tp is None:
                                t.syntax_err("变量'%s'在初始化前使用" % (t.value))
                            parse_stk.push_expr(Expr("lv", t.value, tp))
                            break
                    else:
                        #当前模块或builtin模块
                        for m in self.mod, ppgoc_mod.builtins_mod:
                            if m.has_cls(t.value) or m.has_func(t.value) or m.has_gv(t.value):
                                expr = self.parse_cfgv(m, t, t.value, vars_stk)
                                parse_stk.push_expr(expr)
                                break
                        else:
                            t.syntax_err("未定义的标识符'%s'" % t.value)
            elif t.is_literal:
                assert t.type.startswith("literal_")
                tp = eval("ppgoc_type.%s_TYPE" % t.type[8 :].upper())
                e = Expr("literal", t, tp)
                if t.type == "literal_str" and self.tl.peek().is_sym(".") and self.tl.peek(1).is_sym("("):
                    #字符串字面量的format语法
                    fmt, el = self.parse_str_format(vars_stk, t)
                    e = Expr("str_fmt", (fmt, el), ppgoc_type.STR_TYPE)
                parse_stk.push_expr(e)
            elif t.is_reserved("this"):
                if self.cls is None:
                    t.syntax_err("'this'只能用于方法中")
                parse_stk.push_expr(Expr(
                    "this" if self.tl.peek().is_sym(".") else "this_sp", t, ppgoc_type.from_cls(self.cls)))
            elif t.is_sym("["):
                e = None
                self.tl.revert()
                tp = ppgoc_type.parse_tp(self.tl, self.dep_mns)
                tp.check(self.mod)
                if tp.is_vec:
                    self.tl.pop_sym("{")
                    el = []
                    while True:
                        if self.tl.peek().is_sym("}"):
                            self.tl.pop_sym("}")
                            break

                        el.append(self.parse(vars_stk, tp.vec_elem_tp))

                        t = self.tl.peek()
                        if not (t.is_sym and t.value in ("}", ",")):
                            t.syntax_err("需要','或'}'")
                        if t.value == ",":
                            self.tl.pop_sym(",")

                    e = Expr("new_vec", (tp, el), tp)

                elif tp.is_map:
                    ktp, vtp = tp.map_kv_tp
                    self.tl.pop_sym("{")
                    kvel = []
                    while True:
                        if self.tl.peek().is_sym("}"):
                            self.tl.pop_sym("}")
                            break

                        ke = self.parse(vars_stk, ktp)
                        self.tl.pop_sym(":")
                        ve = self.parse(vars_stk, vtp)
                        kvel.append((ke, ve))

                        t = self.tl.peek()
                        if not (t.is_sym and t.value in ("}", ",")):
                            t.syntax_err("需要','或'}'")
                        if t.value == ",":
                            self.tl.pop_sym(",")

                    e = Expr("new_map", (tp, kvel), tp)

                elif tp.is_set:
                    self.tl.pop_sym("{")
                    el = []
                    while True:
                        if self.tl.peek().is_sym("}"):
                            self.tl.pop_sym("}")
                            break

                        el.append(self.parse(vars_stk, tp.set_elem_tp))

                        t = self.tl.peek()
                        if not (t.is_sym and t.value in ("}", ",")):
                            t.syntax_err("需要','或'}'")
                        if t.value == ",":
                            self.tl.pop_sym(",")

                    e = Expr("new_set", (tp, el), tp)

                else:
                    ppgoc_util.raise_bug()

                assert e is not None
                parse_stk.push_expr(e)
            else:
                t.syntax_err("非法的表达式")

            assert parse_stk.stk

            #解析后缀运算符
            while self.tl:
                t = self.tl.pop()
                if t.is_sym("["):
                    ie = ibe = iee = None
                    to_parse_iee = self.tl.peek().is_sym(":")
                    if not to_parse_iee:
                        ie = self.parse(vars_stk, None)
                        to_parse_iee = self.tl.peek().is_sym(":")
                        if to_parse_iee:
                            ibe = ie
                            ie = None
                    if to_parse_iee:
                        self.tl.pop_sym(":")
                        if not self.tl.peek().is_sym("]"):
                            iee = self.parse(vars_stk, None)
                    self.tl.pop_sym("]")
                    oe = parse_stk.pop_expr()
                    e = None
                    if ie is None:
                        if oe.tp.is_str_type or oe.tp.is_vec or oe.tp.is_vec_view:
                            for sie in ibe, iee:
                                if not (sie is None or sie.tp.is_integer_type):
                                    t.syntax_err("需要整数下标")
                            if oe.tp.is_str_type:
                                e = Expr("str[:]", (oe, ibe, iee), oe.tp)
                            else:
                                elem_tp = oe.tp.vec_elem_tp if oe.tp.is_vec else oe.tp.vec_view_elem_tp
                                vv_type = ppgoc_type.make_vec_view_type(oe.tp.t, elem_tp)
                                e = Expr("vec[:]", (oe, ibe, iee), vv_type)
                        else:
                            t.syntax_err("不能对类型'%s'做切片运算" % oe.tp)
                    else:
                        if oe.tp.is_str_type or oe.tp.is_vec or oe.tp.is_vec_view:
                            if not ie.tp.is_integer_type:
                                t.syntax_err("需要整数下标")
                            if oe.tp.is_str_type:
                                e = Expr("str[]", (oe, ie), ppgoc_type.BYTE_TYPE)
                            else:
                                elem_tp = oe.tp.vec_elem_tp if oe.tp.is_vec else oe.tp.vec_view_elem_tp
                                e = Expr("vec[]", (oe, ie), elem_tp)
                        elif oe.tp.is_map:
                            ktp, vtp = oe.tp.map_kv_tp
                            if ie.tp != ktp:
                                t.syntax_err("下标类型'%s'不是键类型'%s'" % (ie.tp, ktp))
                            e = Expr("map[]", (oe, ie), vtp)
                        else:
                            t.syntax_err("不能对类型'%s'做下标运算" % oe.tp)
                    assert e is not None
                    parse_stk.push_expr(e)
                elif t.is_sym("("):
                    fe = parse_stk.pop_expr()
                    if not fe.tp.is_func:
                        t.syntax_err("类型'%s'不可调用" % fe.tp)
                    atps, rtps = fe.tp.func_arg_ret_tps
                    el = self.parse_exprs_of_calling_ex(vars_stk, atps)
                    e = Expr("()", (fe, el), rtps[0] if len(rtps) == 1 else ppgoc_type.make_multi(rtps))
                    parse_stk.push_expr(e)
                elif t.is_sym(".") and self.tl.peek().is_sym("<"):
                    self.tl.pop_sym("<")
                    tp = ppgoc_type.parse_tp(self.tl, self.dep_mns)
                    tp.check(self.mod)
                    self.tl.pop_sym(">")
                    oe = parse_stk.pop_expr()
                    if tp.can_force_convert_from(oe.tp):
                        parse_stk.push_expr(Expr("convert", (tp, oe), tp))
                    elif tp.can_assert_type_from(oe.tp):
                        parse_stk.push_expr(Expr("assert_type", (tp, oe), tp))
                    else:
                        t.syntax_err("类型'%s'不能强制转换或类型断言为'%s'" % (oe.tp, tp))
                elif t.is_sym("."):
                    t, name = self.tl.pop_name()
                    oe = parse_stk.pop_expr()
                    e = None
                    if oe.tp.is_optional:
                        #optional method
                        arg_defs = ppgoc_util.OrderedDict()
                        if name == "valid":
                            ret_tp = ppgoc_type.BOOL_TYPE
                        elif name == "get":
                            ret_tp = oe.tp.optional_arg_tp
                        elif name == "get_or":
                            arg_defs["value"] = None, oe.tp.optional_arg_tp
                            ret_tp = oe.tp.optional_arg_tp
                        elif name == "set":
                            arg_defs["value"] = None, oe.tp.optional_arg_tp
                            ret_tp = ppgoc_type.make_multi([])
                        elif name == "clear":
                            ret_tp = ppgoc_type.make_multi([])
                        else:
                            t.syntax_err("可选参数类型没有方法'%s'" % name)
                        el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                        e = Expr("call_optional_method", (oe, name, el), ret_tp)
                    elif oe.tp.is_str_type:
                        #str method
                        func = ppgoc_mod.mods[ppgoc_util.MN_BUILTINS + "/strings"].get_func(name)
                        if func is None or not ppgoc_mod.is_public(func):
                            t.syntax_err("'%s'没有伪方法'%s'" % (oe.tp, name))
                        assert func.arg_defs
                        _, tp = func.arg_defs.value_at(0)
                        assert tp.is_str_type
                        arg_defs = ppgoc_util.OrderedDict()
                        for k, v in list(func.arg_defs.iteritems())[1 :]:
                            arg_defs[k] = v
                        el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                        tps = [tp for _, tp in func.ret_defs.itervalues()]
                        e = Expr(
                            "call_func",
                            (func, [oe] + el),
                            tps[0] if len(tps) == 1 else ppgoc_type.make_multi(tps))
                    elif oe.tp.is_vec:
                        bytes_method_ok = False
                        if oe.tp.vec_elem_tp.is_byte_type:
                            #try bytes method
                            func = ppgoc_mod.mods[ppgoc_util.MN_BUILTINS + "/bytes"].get_func(name)
                            bytes_method_ok = func is not None and ppgoc_mod.is_public(func)
                            if bytes_method_ok:
                                #found
                                assert func.arg_defs
                                _, tp = func.arg_defs.value_at(0)
                                assert tp == oe.tp
                                arg_defs = ppgoc_util.OrderedDict()
                                for k, v in list(func.arg_defs.iteritems())[1 :]:
                                    arg_defs[k] = v
                                el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                                tps = [tp for _, tp in func.ret_defs.itervalues()]
                                e = Expr(
                                    "call_func",
                                    (func, [oe] + el),
                                    tps[0] if len(tps) == 1 else ppgoc_type.make_multi(tps))
                        if not bytes_method_ok:
                            #vec method
                            arg_defs = ppgoc_util.OrderedDict()
                            if name == "resize":
                                arg_defs["sz"] = None, ppgoc_type.INT_TYPE
                                ret_tp = oe.tp
                            elif name == "len":
                                ret_tp = ppgoc_type.INT_TYPE
                            elif name == "get":
                                arg_defs["idx"] = None, ppgoc_type.INT_TYPE
                                ret_tp = oe.tp.vec_elem_tp
                            elif name == "set":
                                arg_defs["idx"] = None, ppgoc_type.INT_TYPE
                                arg_defs["e"] = None, oe.tp.vec_elem_tp
                                ret_tp = oe.tp
                            elif name == "append":
                                arg_defs["e"] = None, oe.tp.vec_elem_tp
                                ret_tp = oe.tp
                            elif name == "extend":
                                arg_defs["es"] = None, oe.tp
                                ret_tp = oe.tp
                            elif name == "insert":
                                arg_defs["idx"] = None, ppgoc_type.INT_TYPE
                                arg_defs["e"] = None, oe.tp.vec_elem_tp
                                ret_tp = oe.tp
                            elif name == "insert_vec":
                                arg_defs["idx"] = None, ppgoc_type.INT_TYPE
                                arg_defs["es"] = None, oe.tp
                                ret_tp = oe.tp
                            elif name == "pop":
                                arg_defs["idx"] = None, ppgoc_type.make_optional_type("idx", ppgoc_type.INT_TYPE)
                                ret_tp = oe.tp.vec_elem_tp
                            else:
                                t.syntax_err("'%s'没有伪方法'%s'" % (oe.tp, name))
                            el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                            e = Expr("call_vec_method", (oe, name, el), ret_tp)
                    elif oe.tp.is_vec_view:
                        bytes_view_method_ok = False
                        if oe.tp.vec_view_elem_tp.is_byte_type:
                            #try bytes-view method
                            func = ppgoc_mod.mods[ppgoc_util.MN_BUILTINS + "/bytes_views"].get_func(name)
                            bytes_view_method_ok = func is not None and ppgoc_mod.is_public(func)
                            if bytes_view_method_ok:
                                #found
                                assert func.arg_defs
                                _, tp = func.arg_defs.value_at(0)
                                assert tp == oe.tp
                                arg_defs = ppgoc_util.OrderedDict()
                                for k, v in list(func.arg_defs.iteritems())[1 :]:
                                    arg_defs[k] = v
                                el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                                tps = [tp for _, tp in func.ret_defs.itervalues()]
                                e = Expr(
                                    "call_func",
                                    (func, [oe] + el),
                                    tps[0] if len(tps) == 1 else ppgoc_type.make_multi(tps))
                        if not bytes_view_method_ok:
                            #vec view method
                            arg_defs = ppgoc_util.OrderedDict()
                            if name == "valid":
                                ret_tp = ppgoc_type.BOOL_TYPE
                            elif name == "check_valid":
                                ret_tp = ppgoc_type.make_multi([])
                            elif name == "resolve":
                                ret_tp = ppgoc_type.make_multi([
                                    ppgoc_type.make_vec_type(oe.tp.t, oe.tp.vec_view_elem_tp),
                                    ppgoc_type.INT_TYPE, ppgoc_type.INT_TYPE])
                            elif name == "len":
                                ret_tp = ppgoc_type.INT_TYPE
                            elif name == "get":
                                arg_defs["idx"] = None, ppgoc_type.INT_TYPE
                                ret_tp = oe.tp.vec_view_elem_tp
                            elif name == "set":
                                arg_defs["idx"] = None, ppgoc_type.INT_TYPE
                                arg_defs["e"] = None, oe.tp.vec_view_elem_tp
                                ret_tp = ppgoc_type.make_multi([])
                            else:
                                t.syntax_err("'%s'没有伪方法'%s'" % (oe.tp, name))
                            el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                            e = Expr("call_vec_view_method", (oe, name, el), ret_tp)
                    elif oe.tp.is_map:
                        #map method
                        ktp, vtp = oe.tp.map_kv_tp
                        arg_defs = ppgoc_util.OrderedDict()
                        if name == "len":
                            ret_tp = ppgoc_type.INT_TYPE
                        elif name == "has_key":
                            arg_defs["k"] = None, ktp
                            ret_tp = ppgoc_type.BOOL_TYPE
                        elif name == "get":
                            arg_defs["k"] = None, ktp
                            ret_tp = vtp
                        elif name == "pop":
                            arg_defs["k"] = None, ktp
                            ret_tp = vtp
                        else:
                            t.syntax_err("'%s'没有伪方法'%s'" % (oe.tp, name))
                        el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                        e = Expr("call_map_method", (oe, name, el), ret_tp)
                    elif oe.tp.is_set:
                        #set method
                        arg_defs = ppgoc_util.OrderedDict()
                        if name == "len":
                            ret_tp = ppgoc_type.INT_TYPE
                        elif name == "has":
                            arg_defs["e"] = None, oe.tp.set_elem_tp
                            ret_tp = ppgoc_type.BOOL_TYPE
                        elif name == "add":
                            arg_defs["e"] = None, oe.tp.set_elem_tp
                            ret_tp = oe.tp
                        elif name == "remove":
                            arg_defs["e"] = None, oe.tp.set_elem_tp
                            ret_tp = oe.tp
                        else:
                            t.syntax_err("'%s'没有伪方法'%s'" % (oe.tp, name))
                        el = self.parse_exprs_of_calling(vars_stk, arg_defs)
                        e = Expr("call_set_method", (oe, name, el), ret_tp)
                    elif oe.tp.is_coi_type:
                        coi = oe.tp.get_coi()
                        a, m = coi.get_aom(t, name)
                        if a is not None:
                            assert m is None
                            if a.cls.mod is not self.mod and not ppgoc_mod.is_public(a):
                                t.syntax_err("无法访问'%s'的属性'%s'：没有权限" % (oe.tp, a))
                            e = Expr(".", (oe, a), a.tp)
                        else:
                            assert m is not None
                            if m.mod is not self.mod and not ppgoc_mod.is_public(m):
                                t.syntax_err("无法使用'%s'的方法'%s'：没有权限" % (oe.tp, m))
                            el = self.parse_exprs_of_calling(vars_stk, m.arg_defs)
                            tps = [tp for _, tp in m.ret_defs.itervalues()]
                            e = Expr(
                                "call_method",
                                (oe, m, el),
                                tps[0] if len(tps) == 1 else ppgoc_type.make_multi(tps))
                    else:
                        t.syntax_err("不能对类型'%s'取属性或调用方法" % oe.tp)
                    assert e is not None
                    parse_stk.push_expr(e)
                else:
                    self.tl.revert()
                    break

            if is_expr_end(self.tl.peek()):
                break

            t = self.tl.pop()
            if (t.is_sym and t.value in BINOCULAR_OP_SET) or (t.is_reserved and t.value in ("if", "else")):
                parse_stk.push_op(t.value)
            else:
                t.syntax_err("需要二元或三元运算符")

        expr = parse_stk.finish()
        expr_pos_info = expr.pos_info #保存一下pos info
        if need_type is not None:
            if isinstance(need_type, (tuple, list)):
                need_type_list = list(need_type)
            else:
                need_type_list = [need_type]
            for need_type in need_type_list:
                if need_type == expr.tp:
                    break
                if need_type.can_convert_from(expr.tp):
                    if need_type != expr.tp:
                        expr = Expr("convert", (need_type, expr), need_type)
                    break
            else:
                if len(need_type_list) == 1:
                    parse_stk.t.syntax_err(
                        "表达式（类型'%s'）无法隐式转换为类型'%s'" % (expr.tp, need_type_list[0]))
                else:
                    parse_stk.t.syntax_err(
                        "表达式（类型'%s'）无法隐式转换为类型%s其中任意一个" %
                        (expr.tp, str(need_type_list)))
        expr.pos_info = expr_pos_info #expr可能被修改了，恢复一下pos info
        return expr

    def parse_cfgv(self, mod, t, name, vars_stk):
        gv = mod.get_gv(name)
        if gv is not None:
            if mod is not self.mod and not ppgoc_mod.is_public(gv):
                if mod is ppgoc_mod.builtins_mod:
                    t.syntax_err("找不到'%s'" % name)
                else:
                    t.syntax_err("无法使用'%s'：没有权限" % gv)
            return Expr("gv", gv, gv.tp)

        cls = mod.get_cls(name)
        func = mod.get_func(name)
        cof = cls or func
        if cof is None:
            t.syntax_err("找不到'%s.%s'" % (mod, name))
        if mod is not self.mod and not ppgoc_mod.is_public(cof):
            t.syntax_err("无法使用'%s'：没有权限" % cof)

        if cls is not None:
            init_method = cls.get_init_method()
            if mod is not self.mod and (init_method is None or not ppgoc_mod.is_public(init_method)):
                t.syntax_err("无法创建类'%s'的实例，对init方法没有权限" % cls)
            arg_defs = ppgoc_util.OrderedDict() if init_method is None else init_method.arg_defs
        else:
            arg_defs = func.arg_defs

        el = self.parse_exprs_of_calling(vars_stk, arg_defs)

        if cls is not None:
            tp = ppgoc_type.Type(t, name, mn = mod.name)
            tp.check(mod)
            return Expr("new", (cls, el), tp)
        else:
            tps = [tp for _, tp in func.ret_defs.itervalues()]
            return Expr("call_func", (func, el), tps[0] if len(tps) == 1 else ppgoc_type.make_multi(tps))

    def parse_exprs_of_calling(self, vars_stk, arg_defs):
        self.tl.pop_sym("(")
        return self.parse_exprs_of_calling_ex(vars_stk, [tp for _, tp in arg_defs.itervalues()])

    def parse_exprs_of_calling_ex(self, vars_stk, tps):
        pos_arg_tps = []
        opt_arg_tps = ppgoc_util.OrderedDict()
        for tp in tps:
            if tp.is_optional:
                assert tp.optional_arg_name not in opt_arg_tps
                opt_arg_tps[tp.optional_arg_name] = tp
            else:
                assert not opt_arg_tps
                pos_arg_tps.append(tp)

        #pos args
        pos_arg_tps = pos_arg_tps[:: -1]
        t = self.tl.peek()
        pos_arg_el = []
        while True:
            if self.tl.peek().is_sym(")"):
                if pos_arg_tps:
                    t.syntax_err("参数数量错误")
                break

            if not pos_arg_tps:
                break

            pos_arg_el.append(self.parse(vars_stk, pos_arg_tps.pop()))

            t = self.tl.peek()
            if not (t.is_sym and t.value in (")", ",")):
                t.syntax_err("需要','或')'")
            if t.value == ",":
                self.tl.pop_sym()

        #opt args
        opt_arg_el = ppgoc_util.OrderedDict()
        for name, tp in opt_arg_tps.iteritems():
            opt_arg_el[name] = Expr("make_empty_optional", tp, tp)
        passed_opt_arg_names = set()
        while True:
            if self.tl.peek().is_sym(")"):
                break

            t, name = self.tl.pop_name()
            self.tl.pop_sym("=")
            if name not in opt_arg_tps:
                t.syntax_err("未知可选参数")
            tp = opt_arg_tps[name]
            if name in passed_opt_arg_names:
                t.syntax_err("可选参数重复指定")
            passed_opt_arg_names.add(name)
            e = self.parse(vars_stk, [tp, tp.optional_arg_tp])
            if e.tp != tp:
                assert e.tp == tp.optional_arg_tp
                e = Expr("make_optional", e, tp)
            opt_arg_el[name] = e

            t = self.tl.peek()
            if not (t.is_sym and t.value in (")", ",")):
                t.syntax_err("需要','或')'")
            if t.value == ",":
                self.tl.pop_sym()

        self.tl.pop_sym(")")

        return pos_arg_el + list(opt_arg_el.itervalues())

    def parse_str_format(self, vars_stk, fmt_t):
        assert fmt_t.type == "literal_str"
        self.tl.pop_sym(".")
        self.tl.pop_sym("(")

        el = []
        while True:
            if self.tl.peek().is_sym(")"):
                self.tl.pop_sym(")")
                break

            el.append(self.parse(vars_stk, None))

            t = self.tl.peek()
            if not (t.is_sym and t.value in (")", ",")):
                t.syntax_err("需要','或')'")
            if t.value == ",":
                self.tl.pop_sym()

        t = fmt_t

        fmt = ""
        pos = 0
        ei = 0
        while pos < len(t.value):
            if t.value[pos] != "%":
                fmt += t.value[pos]
                pos += 1
                continue

            class FmtError:
                def __init__(self, msg):
                    self.msg = msg

            try:
                pos += 1
                if t.value[pos] == "%":
                    fmt += "%%"
                    pos += 1
                    continue

                if ei >= len(el):
                    t.syntax_err("format格式化参数不足")
                e = el[ei]
                ei += 1

                #先解析前缀
                conv_spec = "%"
                ds = ""
                while t.value[pos] in "+-0\x20":
                    d = t.value[pos]
                    pos += 1
                    if d in ds:
                        t.syntax_err("format格式存在重复的前缀修饰：'%s...'" % `t.value[: pos]`[1 : -1])
                    conv_spec += d
                    ds += d

                #解析宽度和精度字段
                wp, = re.match(r"""^(\d*\.?\d*)""", t.value[pos :]).groups()
                pos += len(wp)
                conv_spec += wp

                #解析格式字符
                verb = t.value[pos]
                pos += 1
                if verb == "t":
                    if not e.tp.is_bool_type:
                        raise FmtError("'%t'需要类型'bool'")
                    for d in "+0\x20":
                        if d in ds:
                            raise FmtError("'%%t'不能有前缀'%s'" % d)
                    el[ei - 1] = Expr("%t", e, ppgoc_type.STR_TYPE)
                    verb = "s"
                elif verb == "c":
                    if not e.tp.is_byte_type:
                        raise FmtError("'%c'需要类型'byte'")
                    for d in "+0\x20":
                        if d in ds:
                            raise FmtError("'%%c'不能有前缀'%s'" % d)
                    if "." in wp:
                        raise FmtError("'%c'不能指定精度")
                elif verb == "d":
                    if not e.tp.is_integer_type:
                        raise FmtError("'%d'需要整数类型")
                    if e.tp.is_unsigned_integer_type:
                        for d in "+\x20":
                            if d in ds:
                                raise FmtError("'%%d'匹配无符号整数时不能有前缀'%s'" % d)
                        verb = "u"
                    el[ei - 1] = Expr("%d", e, ppgoc_type.STR_TYPE)
                    verb = "ll" + verb
                elif verb == "o":
                    if not e.tp.is_unsigned_integer_type:
                        raise FmtError("'%o'需要无符号整数类型")
                    for d in "+\x20":
                        if d in ds:
                            raise FmtError("'%%o'不能有前缀'%s'" % d)
                    el[ei - 1] = Expr("%d", e, ppgoc_type.STR_TYPE)
                    verb = "ll" + verb
                elif verb in "xX":
                    if e.tp.is_unsigned_integer_type:
                        for d in "+\x20":
                            if d in ds:
                                raise FmtError("'%%%s'匹配无符号整数时不能有前缀'%s'" % (verb, d))
                        el[ei - 1] = Expr("%d", e, ppgoc_type.STR_TYPE)
                        verb = "ll" + verb
                    elif e.tp.is_float_type:
                        verb = "La" if verb == "x" else "LA"
                    elif e.tp.is_str_type:
                        for d in "+0\x20":
                            if d in ds:
                                raise FmtError("'%%%s'匹配字符串时不能有前缀'%s'" % (verb, d))
                        el[ei - 1] = Expr("str%%%s" % verb, e, ppgoc_type.STR_TYPE)
                        verb = "s"
                    else:
                        raise FmtError("'%%%s'需要无符号整数、浮点数或字符串" % verb)
                elif verb in "eEfFgG":
                    if not e.tp.is_float_type:
                        raise FmtError("'%%%s'需要浮点数类型" % verb)
                    verb = "L" + verb
                elif verb in ("s", "T"):
                    if not ppgoc_type.ANY_TYPE.can_convert_from(e.tp):
                        raise FmtError("'%%%s'需要可转为any的类型" % verb)
                    if e.tp != ppgoc_type.ANY_TYPE:
                        e = Expr("convert", (ppgoc_type.ANY_TYPE, e), ppgoc_type.ANY_TYPE)
                    for d in "+0\x20":
                        if d in ds:
                            raise FmtError("'%%%s'不能有前缀'%s'" % (verb, d))
                    el[ei - 1] = Expr("%%%s" % verb, e, ppgoc_type.STR_TYPE)
                    verb = "s"
                else:
                    t.syntax_err("非法的格式符：'%s...'" % `t.value[: pos]`[1 : -1])
                conv_spec += verb
                fmt += conv_spec
            except IndexError:
                t.syntax_err("format格式串非正常结束")
            except FmtError, exc:
                t.syntax_err("format格式化项#%d错误：%s" % (ei, exc.msg))
        if ei < len(el):
            t.syntax_err("format格式化参数过多")
        return fmt, el
