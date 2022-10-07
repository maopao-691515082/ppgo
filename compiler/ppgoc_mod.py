#coding=utf8

import os
import ppgoc_util, ppgoc_token, ppgoc_stmt, ppgoc_type, ppgoc_expr

slib_dir = None
ulib_dir = None

builtins_mod = None
mods = ppgoc_util.OrderedDict()

def is_valid_mn(mn):
    return os.path.normpath(mn) == mn

dep_mod_token_map = {}
def find_mod_dir(mn):
    t = dep_mod_token_map.get(mn)
    err_exit = ppgoc_util.exit if t is None else t.syntax_err

    for d in slib_dir, ulib_dir:
        mod_path = d + "/" + mn
        if os.path.isdir(mod_path):
            return mod_path, d == slib_dir

    err_exit("找不到模块：%s" % mn)

def parse_decrs(tl):
    decrs = set()
    while True:
        t = tl.peek()
        for decr in "public", "final":
            if t.is_reserved(decr):
                if decr in decrs:
                    t.syntax_err("重复的修饰'%s'" % decr)
                decrs.add(decr)
                tl.pop()
                break
        else:
            return decrs

def parse_aor_defs(tl, dep_mns, arg_defs = None):
    aor_defs = ppgoc_util.OrderedDict()
    batch_type_args = ppgoc_util.OrderedDict()
    while True:
        if tl.peek().is_sym(")"):
            break

        t, name = tl.pop_name()
        if (arg_defs is not None and name in arg_defs) or name in aor_defs or name in batch_type_args:
            t.syntax_err("参数名重定义")

        batch_type_args[name] = t

        if tl.peek().is_sym(","):
            tl.pop_sym(",")
            continue

        tp = ppgoc_type.parse_tp(tl, dep_mns, allow_func = arg_defs is None)
        for name, t in batch_type_args.iteritems():
            aor_defs[name] = t, tp
        batch_type_args = ppgoc_util.OrderedDict()

        t = tl.peek()
        if t.is_sym(","):
            tl.pop_sym(",")
            continue

        if not t.is_sym(")"):
            t.syntax_err("需要','或')'")

    if batch_type_args:
        tl.peek().syntax_err("需要类型")
    return aor_defs

class COIBase:
    def __init__(self):
        self.is_cls = isinstance(self, Cls)
        self.is_intf = isinstance(self, Intf)

class MethodBase:
    def check_type(self):
        for _, tp in self.arg_defs.itervalues():
            tp.check(self.mod)
        for _, tp in self.ret_defs.itervalues():
            tp.check(self.mod)

        if (self.name == "str" and
            not (is_public(self) and not self.arg_defs and
                 len(self.ret_defs) == 1 and self.ret_defs.value_at(0)[1] == ppgoc_type.STR_TYPE)):
            self.name_t.syntax_err("'str'方法必须遵循标准: `public func str() string`")

class ClsAttr:
    def __init__(self, cls, decrs, name_t, name, tp):
        self.cls = cls
        self.decrs = decrs
        self.name_t = name_t
        self.name = name
        self.tp = tp

    __repr__ = __str__ = lambda self : "%s.%s" % (self.cls, self.name)

    def check_type(self):
        self.tp.check(self.cls.mod)

class ClsMethod(MethodBase):
    def __init__(self, cls, decrs, name_t, name, arg_defs, ret_defs, block_tl):
        self.fn = cls.fn
        self.cls = cls
        self.mod = self.cls.mod
        self.decrs = decrs
        self.name_t = name_t
        self.name = name
        self.arg_defs = arg_defs
        self.ret_defs = ret_defs
        self.block_tl = block_tl

    __repr__ = __str__ = lambda self : "%s.%s" % (self.cls, self.name)

    def compile(self):
        self.stmts = None
        if self.block_tl is not None:
            arg_tps = ppgoc_util.OrderedDict()
            for name, (_, tp) in self.arg_defs.iteritems():
                arg_tps[name] = tp
            if not (len(self.ret_defs) == 1 and None in self.ret_defs):
                for name, (_, tp) in self.ret_defs.iteritems():
                    assert name and name not in arg_tps
                    arg_tps[name] = tp
            self.stmts = ppgoc_stmt.Parser(
                self.block_tl, self.cls.mod, self.cls.mod.dep_mns_by_fn(self.fn), self.cls, self,
            ).parse((arg_tps,), 0)
            self.block_tl.pop_sym("}")
            assert not self.block_tl
        del self.block_tl

class Cls(COIBase):
    def __init__(self, mod, fn, decrs, name_t, name):
        COIBase.__init__(self)

        self.mod = mod
        self.fn = fn
        self.decrs = decrs
        self.name_t = name_t
        self.name = name

    __repr__ = __str__ = lambda self : "%s.%s" % (self.mod, self.name)

    def get_init_method(self):
        return self.methods["init"] if "init" in self.methods else None

    def parse(self, tl):
        self._parse(tl)
        init_method = self.get_init_method()
        if init_method is not None:
            #check init method
            if init_method.ret_defs:
                init_method.name_t.syntax_err("init方法不能有返回")

    def _parse(self, tl):
        dep_mns = self.mod.dep_mns_by_fn(self.fn)
        self.attrs = ppgoc_util.OrderedDict()
        self.methods = ppgoc_util.OrderedDict()
        while True:
            t = tl.peek()
            if t.is_sym("}"):
                break

            decrs = parse_decrs(tl)
            if decrs - set(["public"]):
                t.syntax_err("方法或属性只能用public修饰")

            t = tl.peek()

            if t.is_reserved("func"):
                tl.pop()
                name_t, name = tl.pop_name()
                if name in self.attrs or name in self.methods:
                    name_t.syntax_err("属性或方法名重定义")
                tl.pop_sym("(")
                arg_defs = parse_aor_defs(tl, dep_mns)
                tl.pop_sym(")")

                ret_defs = ppgoc_util.OrderedDict()
                t = tl.peek()
                if not (t.is_sym("{") or t.is_sym(";")):
                    if t.is_sym("("):
                        tl.pop_sym("(")
                        ret_defs = parse_aor_defs(tl, dep_mns, arg_defs)
                        tl.pop_sym(")")
                    else:
                        ret_defs[None] = tl.peek(), ppgoc_type.parse_tp(tl, dep_mns)

                block_tl = None
                t, sym = tl.pop_sym()
                if sym == "{":
                    block_tl, sym = ppgoc_token.parse_tl_until_sym(tl, ("}",))
                    assert sym == "}"
                elif sym != ";":
                    t.syntax_err("需要'{'或';'")

                self.methods[name] = ClsMethod(self, decrs, name_t, name, arg_defs, ret_defs, block_tl)
                continue

            if t.is_name:
                names = []
                while True:
                    names.append(tl.pop_name())
                    if not tl.peek().is_sym(","):
                        break
                    tl.pop_sym(",")
                assert names
                tp = ppgoc_type.parse_tp(tl, dep_mns)
                tl.pop_sym(";")
                for name_t, name in names:
                    if name in self.attrs or name in self.methods:
                        name_t.syntax_err("属性或方法名重定义")
                    self.attrs[name] = ClsAttr(self, decrs, name_t, name, tp)
                continue

            t.syntax_err()

    def check_type(self):
        for attr in self.attrs.itervalues():
            attr.check_type()
        for m in self.methods.itervalues():
            m.check_type()

    def compile(self):
        for m in self.methods.itervalues():
            m.compile()

    def get_aom(self, t, name):
        if name in self.attrs:
            return self.attrs[name], None
        if name in self.methods:
            return None, self.methods[name]
        t.syntax_err("类'%s'没有方法或属性'%s'" % (self, name))

    def find_impled_intfs(self):
        self.impled_intfs = []
        for mod in mods.itervalues():
            for intf in mod.intfs.itervalues():
                if intf.can_convert_from(self):
                    self.impled_intfs.append(intf)

class IntfMethod(MethodBase):
    def __init__(self, intf, decrs, name_t, name, arg_defs, ret_defs):
        self.intf = intf
        self.mod = self.intf.mod
        self.decrs = decrs
        self.name_t = name_t
        self.name = name
        self.arg_defs = arg_defs
        self.ret_defs = ret_defs

    __repr__ = __str__ = lambda self : "%s.%s" % (self.intf, self.name)

class Intf(COIBase):
    def __init__(self, mod, fn, decrs, name_t, name):
        COIBase.__init__(self)

        self.mod = mod
        self.fn = fn
        self.decrs = decrs
        self.name_t = name_t
        self.name = name

    __repr__ = __str__ = lambda self : "%s.%s" % (self.mod, self.name)

    def parse(self, tl):
        dep_mns = self.mod.dep_mns_by_fn(self.fn)
        self.methods = ppgoc_util.OrderedDict()
        while True:
            t = tl.peek()
            if t.is_sym("}"):
                break

            decrs = parse_decrs(tl)
            if decrs - set(["public"]):
                t.syntax_err("接口方法只能用public修饰")

            name_t, name = tl.pop_name()
            if name in self.methods:
                name_t.syntax_err("方法重定义")
            tl.pop_sym("(")
            arg_defs = parse_aor_defs(tl, dep_mns)
            tl.pop_sym(")")

            ret_defs = ppgoc_util.OrderedDict()
            t = tl.peek()
            if not t.is_sym(";"):
                if t.is_sym("("):
                    tl.pop_sym("(")
                    ret_defs = parse_aor_defs(tl, dep_mns, arg_defs)
                    tl.pop_sym(")")
                else:
                    ret_defs[None] = tl.peek(), ppgoc_type.parse_tp(tl, dep_mns)

            tl.pop_sym(";")

            self.methods[name] = IntfMethod(self, decrs, name_t, name, arg_defs, ret_defs)

    def check_type(self):
        for m in self.methods.itervalues():
            m.check_type()

    def get_aom(self, t, name):
        if name in self.methods:
            return None, self.methods[name]
        t.syntax_err("接口'%s'没有方法'%s'" % (self, name))

    def can_convert_from(self, other):
        for m in self.methods.itervalues():
            if m.name not in other.methods:
                return False
            om = other.methods[m.name]
            if is_public(m) != is_public(om):
                return False
            if not is_public(m) and self.mod is not other.mod:
                return False
            if len(m.arg_defs) != len(om.arg_defs) or len(m.ret_defs) != len(om.ret_defs):
                return False
            for (_, tp), (_, otp) in zip(
                list(m.arg_defs.itervalues()) + list(m.ret_defs.itervalues()),
                list(om.arg_defs.itervalues()) + list(om.ret_defs.itervalues())):
                if tp != otp:
                    return False
        return True

class Closure:
    def __init__(self, mod, fn, func_t, tl, vars_stk, cls):
        self.mod = mod
        self.fn = fn
        self.func_t = func_t
        self.name = "<closure@%d:%d>" % (func_t.line_no, func_t.pos + 1)

        dep_mns = self.mod.dep_mns_by_fn(self.fn)

        tl.pop_sym("(")
        self.arg_defs = parse_aor_defs(tl, dep_mns)
        tl.pop_sym(")")

        self.ret_defs = ppgoc_util.OrderedDict()
        t = tl.peek()
        if not t.is_sym("{"):
            if t.is_sym("("):
                tl.pop_sym("(")
                self.ret_defs = parse_aor_defs(tl, dep_mns, self.arg_defs)
                tl.pop_sym(")")
            else:
                self.ret_defs[None] = tl.peek(), ppgoc_type.parse_tp(tl, dep_mns)

        for var_defs in self.arg_defs, self.ret_defs:
            for name, (t, _) in var_defs:
                if name is not None:
                    for vars in vars_stk:
                        if name in vars:
                            t.syntax_err("与上层变量名冲突")

        tl.pop_sym("{")
        arg_tps = ppgoc_util.OrderedDict()
        for name, (_, tp) in self.arg_defs.iteritems():
            arg_tps[name] = tp
        if not (len(self.ret_defs) == 1 and None in self.ret_defs):
            for name, (_, tp) in self.ret_defs.iteritems():
                assert name and name not in arg_tps
                arg_tps[name] = tp
        self.stmts = ppgoc_stmt.Parser(
            tl, self.mod, dep_mns, cls, self,
        ).parse(vars_stk + (arg_tps,), 0)
        tl.pop_sym("}")

    __repr__ = __str__ = lambda self : "%s.%s" % (self.mod, self.name)

class Func:
    def __init__(self, mod, fn, decrs, name_t, name, arg_defs, ret_defs, block_tl):
        self.mod = mod
        self.fn = fn
        self.decrs = decrs
        self.name_t = name_t
        self.name = name
        self.arg_defs = arg_defs
        self.ret_defs = ret_defs
        self.block_tl = block_tl

    __repr__ = __str__ = lambda self : "%s.%s" % (self.mod, self.name)

    def check_type(self):
        for _, tp in self.arg_defs.itervalues():
            tp.check(self.mod)
        for _, tp in self.ret_defs.itervalues():
            tp.check(self.mod)

    def compile(self):
        self.stmts = None
        if self.block_tl is not None:
            arg_tps = ppgoc_util.OrderedDict()
            for name, (_, tp) in self.arg_defs.iteritems():
                arg_tps[name] = tp
            if not (len(self.ret_defs) == 1 and None in self.ret_defs):
                for name, (_, tp) in self.ret_defs.iteritems():
                    assert name and name not in arg_tps
                    arg_tps[name] = tp
            self.stmts = ppgoc_stmt.Parser(
                self.block_tl, self.mod, self.mod.dep_mns_by_fn(self.fn), None, self,
            ).parse((arg_tps,), 0)
            self.block_tl.pop_sym("}")
            assert not self.block_tl
        del self.block_tl

class GlobalVar:
    def __init__(self, mod, fn, decrs, name_t, name, tp, init_expr_tl):
        self.mod = mod
        self.fn = fn
        self.decrs = decrs
        self.name_t = name_t
        self.name = name
        self.tp = tp
        self.init_expr_tl = init_expr_tl

    __repr__ = __str__ = lambda self : "%s.%s" % (self.mod, self.name)

    def check_type(self):
        self.tp.check(self.mod)

    def compile(self):
        if self.init_expr_tl is None:
            self.init_expr = None
        else:
            self.init_expr = ppgoc_expr.Parser(
                self.init_expr_tl, self.mod, self.fn, self.mod.dep_mns_by_fn(self.fn), None, None,
            ).parse((), self.tp)
            t, sym = self.init_expr_tl.pop_sym()
            assert not self.init_expr_tl and sym == ";"
        del self.init_expr_tl

class Mod:
    def __init__(self, name):
        assert is_valid_mn(name)
        self.dir, self.is_slib_mod = find_mod_dir(name)
        assert os.path.isdir(self.dir)
        assert self.dir.endswith(name)
        self.name = name

        #{file_name: {import_name/alias: mn}}
        self.file_dep_mns = ppgoc_util.OrderedDict()

        self.clses = ppgoc_util.OrderedDict()
        self.intfs = ppgoc_util.OrderedDict()
        self.funcs = ppgoc_util.OrderedDict()
        self.gvs = ppgoc_util.OrderedDict()

        self.literals = []

        for fn in sorted([fn for fn in os.listdir(self.dir) if fn.endswith(".ppgo")]):
            self.precompile(fn)

        self.check_name_conflict()

    __repr__ = __str__ = lambda self: self.name

    def dep_mns_by_fn(self, fn):
        return self.file_dep_mns[fn]

    def iter_dep_mns(self):
        for dep_mns in self.file_dep_mns.itervalues():
            for mn in dep_mns.itervalues():
                yield mn

    def precompile(self, fn):
        fp = self.dir + "/" + fn
        if not os.path.isfile(fp):
            ppgoc_util.exit("[%s]需要是一个文件" % fp)
        tl = ppgoc_token.parse_token_list(self.name, fp)
        self.parse_text(fn, tl)

    def check_name_conflict(self):
        if self.name != ppgoc_util.MN_BUILTINS:
            for map in self.clses, self.intfs, self.funcs, self.gvs:
                for i in map.itervalues():
                    elem = builtins_mod.get_elem(i.name, public_only = True)
                    if elem is not None:
                        i.name_t.syntax_err("'%s'与'%s'名字冲突" % (i, elem))

    def parse_text(self, fn, tl):
        self.file_dep_mns[fn] = dep_mns = ppgoc_util.OrderedDict()
        self.literals += [t for t in tl if t.is_literal]
        if tl and tl.peek().is_reserved("import"):
            self.parse_import(tl, dep_mns)
        while tl:
            decrs = parse_decrs(tl)
            t = tl.pop()

            if t.is_reserved("class") or t.is_reserved("interface") or t.is_reserved("func"):
                if decrs - set(["public"]):
                    t.syntax_err("class、interface或func只能用public修饰")
                {
                    "class":        self.parse_cls,
                    "interface":    self.parse_intf,
                    "func":         self.parse_func,
                }[t.value](fn, dep_mns, decrs, tl)
                continue

            if t.is_reserved("var"):
                self.parse_gv(fn, dep_mns, decrs, tl)
                continue

            t.syntax_err()

    def parse_import(self, tl, dep_mns):
        t = tl.pop()
        assert t.is_reserved("import")
        is_batch_import = tl.peek().is_sym("(")
        if is_batch_import:
            tl.pop_sym("(")
        def check_p(p, t):
            if p:
                has_sub = p[-1] == "/"
                if has_sub:
                    p = p[: -1]
                np = os.path.normpath(p)
                if p == np and not p.startswith(".") and not p.startswith("/"):
                    return has_sub
            t.syntax_err("非法的import路径")
        def parse_alias(p, t):
            if tl.peek().is_sym(":"):
                tl.pop_sym(":")
                t, alias = tl.pop_name()
                return alias
            alias = p.split("/")[-1]
            if not ppgoc_token.is_valid_name(alias):
                t.syntax_err("不是合法名字的模块目录，必须指定别名")
            return alias
        pl = []
        while True:
            t = tl.pop()
            if not t.is_literal("str"):
                t.syntax_err("需要字符串")
            p = t.value
            rp = None
            if p.startswith("./"):
                rp = 0
                p = p[2 :]
            elif p.startswith("../"):
                rp = 0
                while p.startswith("../"):
                    rp += 1
                    p = p[3 :]
            if rp is None or p: #绝对路径，或者是相对路径且有相对之后的实际path
                has_sub = check_p(p, t)
            else:
                has_sub = True

            def parse_sub_p():
                tl.pop_sym("(")
                while True:
                    t = tl.pop()
                    if not t.is_literal("str"):
                        t.syntax_err("需要字符串")
                    p = t.value

                    has_sub = check_p(p, t)
                    if has_sub:
                        for sub_p, alias, t in parse_sub_p():
                            yield p + sub_p, alias, t
                    else:
                        alias = parse_alias(p, t)
                        tl.pop_sym(";")
                        yield p, alias, t

                    if tl.peek().is_sym(")"):
                        tl.pop()
                        break

            if has_sub:
                for sub_p, alias, t in parse_sub_p():
                    pl.append((rp, p + sub_p, alias, t))
            else:
                alias = parse_alias(p, t)
                tl.pop_sym(";")
                pl.append((rp, p, alias, t))

            if not is_batch_import:
                break
            if tl.peek().is_sym(")"):
                tl.pop()
                break

        for rp, p, alias, t in pl:
            if rp is None:
                mn = p
            else:
                mn = self.name + "/" + "../" * rp + p
            mn = os.path.normpath(mn)
            if mn.startswith("."):
                t.syntax_err("非法的import路径")
            if mn.split("/")[0] == "__builtins":
                t.syntax_err("不能显式import __builtins模块")
            dep_mod_token_map[mn] = t
            if alias in dep_mns:
                t.syntax_err("模块名重复")
            dep_mns[alias] = mn

    def check_redefine(self, t, name, dep_mns):
        if name in dep_mns:
            t.syntax_err("定义的名字和导入模块名重名")
        for i in self.clses, self.intfs, self.gvs, self.funcs:
            if name in i:
                t.syntax_err("名字重定义")

    def parse_cls(self, fn, dep_mns, decrs, tl):
        name_t, name = tl.pop_name()
        self.check_redefine(name_t, name, dep_mns)
        tl.pop_sym("{")
        cls = Cls(self, fn, decrs, name_t, name)
        cls.parse(tl)
        tl.pop_sym("}")
        self.clses[name] = cls

    def parse_intf(self, fn, dep_mns, decrs, tl):
        name_t, name = tl.pop_name()
        self.check_redefine(name_t, name, dep_mns)
        tl.pop_sym("{")
        intf = Intf(self, fn, decrs, name_t, name)
        intf.parse(tl)
        tl.pop_sym("}")
        self.intfs[name] = intf

    def parse_func(self, fn, dep_mns, decrs, tl):
        name_t, name = tl.pop_name()
        self.check_redefine(name_t, name, dep_mns)
        tl.pop_sym("(")
        arg_defs = parse_aor_defs(tl, dep_mns)
        tl.pop_sym(")")

        ret_defs = ppgoc_util.OrderedDict()
        t = tl.peek()
        if not (t.is_sym("{") or t.is_sym(";")):
            if t.is_sym("("):
                tl.pop_sym("(")
                ret_defs = parse_aor_defs(tl, dep_mns, arg_defs)
                tl.pop_sym(")")
            else:
                ret_defs[None] = tl.peek(), ppgoc_type.parse_tp(tl, dep_mns)

        block_tl = None
        t, sym = tl.pop_sym()
        if sym == "{":
            block_tl, sym = ppgoc_token.parse_tl_until_sym(tl, ("}",))
            assert sym == "}"
        elif sym != ";":
            t.syntax_err("需要'{'或';'")

        self.funcs[name] = Func(self, fn, decrs, name_t, name, arg_defs, ret_defs, block_tl)

    def parse_gv(self, fn, dep_mns, decrs, tl):
        is_batch_def = tl.peek().is_sym("(")
        if is_batch_def:
            tl.pop_sym("(")
        while True:
            names = []
            while True:
                names.append(tl.pop_name())
                if not tl.peek().is_sym(","):
                    break
                tl.pop_sym(",")
            assert names
            tp = ppgoc_type.parse_tp(tl, dep_mns)
            if len(names) == 1 and tl.peek().is_sym("="):
                #var x type = expr;
                tl.pop_sym("=")
                init_expr_tl, sym = ppgoc_token.parse_tl_until_sym(tl, (";",))
                assert sym == ";"
            else:
                #var x, y type;
                init_expr_tl = None
                tl.pop_sym(";")
            for name_t, name in names:
                self.check_redefine(name_t, name, dep_mns)
                self.gvs[name] = GlobalVar(self, fn, decrs, name_t, name, tp, init_expr_tl)

            if not is_batch_def:
                break
            if tl.peek().is_sym(")"):
                tl.pop_sym(")")
                break

    def get_elem(self, name, public_only = False):
        for map in self.clses, self.intfs, self.funcs, self.gvs:
            if name in map:
                elem = map[name]
                return None if public_only and not is_public(elem) else elem
        return None

    def has_cls(self, name):
        return name in self.clses

    def has_intf(self, name):
        return name in self.intfs

    def has_func(self, name):
        return name in self.funcs

    def has_gv(self, name):
        return name in self.gvs

    def check_type(self):
        for map in self.clses, self.intfs, self.funcs, self.gvs:
            for i in map.itervalues():
                i.check_type()

    def get_coi(self, tp):
        if tp.name in self.clses:
            return self.clses[tp.name]
        if tp.name in self.intfs:
            return self.intfs[tp.name]
        return None

    def get_gv(self, name):
        return self.gvs[name] if self.has_gv(name) else None

    def get_cls(self, name):
        return self.clses[name] if self.has_cls(name) else None

    def get_intf(self, name):
        return self.intfs[name] if self.has_intf(name) else None

    def get_func(self, name):
        return self.funcs[name] if self.has_func(name) else None

    def check_init_func(self):
        if "init" not in self.funcs:
            return
        init_func = self.funcs["init"]
        def err_exit(msg):
            init_func.name_t.syntax_err("模块[%s]的初始化函数定义错误：%s" % (self, msg))
        if is_public(init_func):
            err_exit("不能是public的")
        if init_func.arg_defs:
            err_exit("不能有参数")
        if init_func.ret_defs:
            err_exit("不能有返回值")

    def check_main_func(self):
        if "main" not in self.funcs:
            ppgoc_util.exit("主模块[%s]没有main函数" % self)
        main_func = self.funcs["main"]
        def err_exit(msg):
            main_func.name_t.syntax_err("主模块[%s]的main函数定义错误：%s" % (self, msg))
        if not is_public(main_func):
            err_exit("必须是public的")
        if main_func.arg_defs:
            err_exit("不能有参数")
        if main_func.ret_defs:
            err_exit("不能有返回值")

    def find_impled_intfs_of_clses(self):
        for cls in self.clses.itervalues():
            cls.find_impled_intfs()

    def compile(self):
        for map in self.clses, self.funcs, self.gvs:
            for i in map.itervalues():
                i.compile()

def precompile_mod(mn):
    global builtins_mod

    chain = []

    def do(mn):
        chain.append(mn)
        if chain.count(mn) > 1:
            ppgoc_util.exit("检测到循环import：\n" + "\n".join(["\t" + mn for mn in chain]))

        if mn in mods:
            m = mods[mn]
        else:
            mods[mn] = m = Mod(mn)
            for mn in m.iter_dep_mns():
                do(mn)

        chain.pop()
        return m

    m = do(mn)
    if mn == "__builtins":
        builtins_mod = m
    return m

def is_public(x):
    return "public" in x.decrs
