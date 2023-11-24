#coding=utf8

import os, shutil, hashlib, time, re, sys, platform
import ppgoc_env, ppgoc_util, ppgoc_mod, ppgoc_token, ppgoc_type

main_mod = None

#对所有模块按照hash来做输出的code
mncs = {}
main_mnc = None

exe_file = None

unique_id = None
def new_id():
    global unique_id
    unique_id += 1
    return unique_id

class Code:
    class CodeBlk:
        def __init__(self, code, is_namespace):
            self.code = code
            self.is_namespace = is_namespace

        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc_value, traceback):
            if exc_type is not None:
                return
            if self.is_namespace:
                self.code += ""
            else:
                assert len(self.code.indent) >= 4
                self.code.indent = self.code.indent[: -4]
            self.code += "}"

    def __init__(self, fn, use_id = False):
        self.fn = fn
        self.use_id = use_id

        self.lines = []
        self.indent = ""

    def __iadd__(self, line):
        self.lines.append(self.indent + line)
        return self

    def __enter__(self):
        global unique_id
        if self.use_id:
            assert unique_id is None
            unique_id = 0
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        global unique_id
        if self.use_id:
            assert unique_id is not None
            unique_id = None
        if exc_type is not None:
            return
        if self.fn is None:
            #to string
            self.content = "\n".join(self.lines) + "\n"
            return
        with open(self.fn, "w") as f:
            for line in self.lines:
                print >> f, line

    def new_blk(self, title, start_with_blank_line = True):
        if start_with_blank_line:
            self += ""
        if title:
            self += title
        self += "{"
        is_namespace = title.startswith("namespace ")
        if is_namespace:
            self += ""
        else:
            self.indent += " " * 4
        return self.CodeBlk(self, is_namespace)

def gen_all_mncs():
    for hash_prefix_len in xrange(4, 33):
        hm = {}
        for name in ppgoc_mod.mods:
            hm[name] = hashlib.md5(name).hexdigest().upper()[: hash_prefix_len]
        if len(hm) == len(set(hm.itervalues())):
            break
    else:
        rhm = {}
        for name, h in hm.iteritems():
            if h in rhm:
                ppgoc_util.exit("恭喜你找到了两个md5一样的字符串：'%s'和'%s'" % (rhm[h], name))
        ppgoc_util.raise_bug()

    for name in ppgoc_mod.mods:
        mncs[name] = "mod%s" % hm[name]

def gen_mnc(mod):
    return mncs[mod.name]

def gen_ns_code(mod):
    return "namespace %s /* %s */" % (gen_mnc(mod), mod.name)

def gen_tp_code(tp):
    if tp.is_optional:
        return "::std::optional<%s >" % gen_tp_code(tp.optional_arg_tp)
    if tp.is_any:
        return "::ppgo::Any::Ptr"
    if tp.is_base_type:
        return "::ppgo::tp_%s" % tp.name
    if tp.is_vec:
        return "::ppgo::Vec<%s >" % gen_tp_code(tp.vec_elem_tp)
    if tp.is_vec_view:
        return "::ppgo::VecView<%s >" % gen_tp_code(tp.vec_view_elem_tp)
    if tp.is_map:
        ktp, vtp = tp.map_kv_tp
        return "::ppgo::Map<%s, %s >" % (gen_tp_code(ktp), gen_tp_code(vtp))
    if tp.is_set:
        return "::ppgo::Set<%s >" % gen_tp_code(tp.set_elem_tp)
    if tp.is_func:
        atps, rtps = tp.func_arg_ret_tps
        cs = ["::std::function<::ppgo::Exc::Ptr (%s &" % gen_func_ret_tp_code_from_tps(rtps)]
        for atp in atps:
            cs += [",", gen_tp_code(atp)]
        cs.append(")>")
        return "".join(cs)
    assert tp.is_coi_type
    coi = tp.get_coi()
    mnc = mncs[coi.mod.name]
    for prefix in "cls", "intf":
        if eval("coi.is_%s" % prefix):
            break
    else:
        ppgoc_util.raise_bug()
    return "std::shared_ptr<::ppgo::%s::%s_%s>" % (mnc, prefix, coi.name)

def gen_func_ret_tp_code_from_tps(tps):
    cs = ["::std::tuple<"]
    for i, tp in enumerate(tps):
        if i > 0:
            cs.append(", ")
        cs.append(gen_tp_code(tp))
    cs.append(">")
    return "".join(cs)

def gen_func_ret_tp_code(func):
    return gen_func_ret_tp_code_from_tps([tp for _, tp in func.ret_defs.itervalues()])

def gen_method_ret_tp_code(method):
    return gen_func_ret_tp_code_from_tps([tp for _, tp in method.ret_defs.itervalues()])

def gen_func_def(func):
    cs = ["::ppgo::Exc::Ptr func_%s(%s &ret" % (func.name, gen_func_ret_tp_code(func))]
    for name, (_, tp) in func.arg_defs.iteritems():
        cs.append(", ")
        cs.append("%s l_%s" % (gen_tp_code(tp), name))
    cs.append(")")
    return "".join(cs)

def gen_method_def(method, with_cls_name = False):
    cs = ["::ppgo::Exc::Ptr "]
    if with_cls_name:
        cs.append("cls_%s::" % method.cls.name)
    cs.append("method_%s(%s &ret" % (method.name, gen_method_ret_tp_code(method)))
    for name, (_, tp) in method.arg_defs.iteritems():
        cs.append(", ")
        cs.append("%s l_%s" % (gen_tp_code(tp), name))
    cs.append(")")
    return "".join(cs)

def output_prog_h(native_header_fns):
    with Code(ppgoc_env.out_dir + "/_prog.h") as code:
        code += "#pragma once"
        with code.new_blk("namespace ppgo"):

            #mod nss
            code += "//mod namespaces:"
            for mn, mnc in sorted(mncs.iteritems(), key = lambda (mn, mnc): mn):
                code += "//  %s %s" % (mnc, mn)

            #intf & cls name decl
            for mod in ppgoc_mod.mods.itervalues():
                with code.new_blk(gen_ns_code(mod)):
                    for intf in mod.intfs.itervalues():
                        code += "struct intf_%s;" % intf.name
                    for cls in mod.clses.itervalues():
                        code += "struct cls_%s;" % cls.name

        for fn in native_header_fns:
            code += '#include "%s"' % fn

        with code.new_blk("namespace ppgo"):

            #intf def
            for mod in ppgoc_mod.mods.itervalues():
                with code.new_blk(gen_ns_code(mod)):
                    for intf in mod.intfs.itervalues():
                        with code.new_blk("struct intf_%s : public virtual ::ppgo::Any" % intf.name):
                            for m in intf.methods.itervalues():
                                code += "virtual %s = 0;" % gen_method_def(m)
                            with code.new_blk("static std::string TypeName()"):
                                code += "return %s;" % c_str_literal(str(intf))
                        code += ";"

            #cls def
            for mod in ppgoc_mod.mods.itervalues():
                with code.new_blk(gen_ns_code(mod)):
                    for cls in mod.clses.itervalues():
                        derived_intfs_codes = []
                        overrided_method_names = set(["str"])
                        for intf in cls.impled_intfs:
                            derived_intfs_codes.append(
                                ", public virtual ::ppgo::%s::intf_%s" % (mncs[intf.mod.name], intf.name))
                            for name in intf.methods:
                                overrided_method_names.add(name)
                        with code.new_blk(
                            "struct cls_%s final : "
                            "public ::std::enable_shared_from_this<cls_%s>, public virtual ::ppgo::Any%s" %
                            (cls.name, cls.name, "".join(derived_intfs_codes))
                        ):
                            code += "::ppgo::NativeAttrs<cls_%s> nas;" % cls.name
                            code += "virtual ~cls_%s();" % cls.name
                            with code.new_blk("virtual ::std::string R_TypeName() const override"):
                                code += "return %s;" % c_str_literal(str(cls))
                            code += ""
                            code += "//attrs"
                            for a in cls.attrs.itervalues():
                                code += "%s attr_%s%s;" % (gen_tp_code(a.tp), a.name, gen_var_init_code(a.tp))
                            code += ""
                            code += "//methods"
                            for m in cls.methods.itervalues():
                                code += (
                                    "virtual %s %s;" %
                                    (gen_method_def(m),
                                     "override" if m.name in overrided_method_names else ""))
                            with code.new_blk("static std::string TypeName()"):
                                code += "return %s;" % c_str_literal(str(cls))
                        code += ";"

            #func & gv decl
            for mod in ppgoc_mod.mods.itervalues():
                with code.new_blk(gen_ns_code(mod)):
                    for func in mod.funcs.itervalues():
                        code += gen_func_def(func) + ";"
                    for gv in mod.gvs.itervalues():
                        code += "extern %s gv_%s;" % (gen_tp_code(gv.tp), gv.name)

            #init func decl
            for mod in ppgoc_mod.mods.itervalues():
                with code.new_blk(gen_ns_code(mod)):
                    code += "::ppgo::Exc::Ptr init();"

def c_str_literal(s):
    cs = ['"']
    for c in s:
        n = ord(c)
        if 32 <= n <= 126:
            cs.append(c)
        else:
            cs.append("\\%03o" % n)
    cs.append('"')
    return "".join(cs)

def gen_wrap_convert(c, c_tp, to_tp):
    if c_tp == to_tp:
        return c
    assert to_tp.can_force_convert_from(c_tp)
    if c_tp.is_coi_type:
        if c_tp.get_coi().is_intf and not to_tp.is_any:
            assert to_tp.is_coi_type and to_tp.get_coi().is_intf
            #intf -> intf and must success
            to_tp_code = gen_tp_code(to_tp)
            P, S = "std::shared_ptr<", ">"
            assert to_tp_code.startswith(P) and to_tp_code.endswith(S)
            c = "std::dynamic_pointer_cast<%s>(%s)" % (to_tp_code[len(P) : -len(S)], c)
    if to_tp.is_any:
        if c_tp.is_bool_type or c_tp.is_number_type or c_tp.is_str_type:
            return "::ppgo::base_type_boxing::Obj<%s>::New(%s)" % (gen_tp_code(c_tp), c)
        if c_tp.is_vec or c_tp.is_map or c_tp.is_set:
            return "(%s).AsAny()" % c
    return "%s(%s)" % (gen_tp_code(to_tp), c)

def gen_expr_code(expr, pos_info = None, mode = "r"):
    if pos_info is None:
        pos_info = expr.pos_info
    assert pos_info is not None

    if expr.op == "call_func":
        func, el = expr.arg
        rv = "ret_%d" % new_id()
        excv = "exc_%d" % new_id()
        cs = [
            "({",
            "%s %s;" % (gen_func_ret_tp_code(func), rv),
            "auto %s = ::ppgo::%s::func_%s(%s" % (excv, mncs[func.mod.name], func.name, rv),
        ]
        ecs = ["(%s)" % gen_expr_code(e, pos_info) for e in el]
        if ecs:
            cs.append(", " + ", ".join(ecs))
        cs.append(");")
        t, fom = pos_info
        cs += [
            "if (%s) {" % excv,
            "%s->PushTB(%s, %d, %s);" %
                (excv, c_str_literal(t.src_file), t.line_no,
                 c_str_literal("<none>" if fom is None else str(fom))),
            "return %s;}" % excv,
        ]
        if len(func.ret_defs) == 1:
            cs.append("::std::get<0>(%s);" % rv)
        else:
            cs.append("%s;" % rv)
        cs.append("})")
        return "\n".join(cs)

    if expr.op == "literal":
        t = expr.arg
        assert t.type.startswith("literal_")
        literal_type = t.type[8 :]
        if literal_type == "nil":
            return "nullptr"
        if literal_type == "bool":
            return "%s" % t.value
        if literal_type == "byte":
            assert 0 <= t.value <= 0xFF
            return "::ppgo::tp_byte{%s}" % str(t.value)
        if literal_type == "int":
            return "::ppgo::tp_int{%s}" % (str(t.value) + "LL")
        if literal_type == "uint":
            return "::ppgo::tp_uint{%s}" % (str(t.value) + "ULL")
        if literal_type == "float":
            return "::ppgo::tp_float{%sL}" % t.value
        if literal_type == "str":
            slid = new_id()
            return (
                "({static const ::ppgo::tp_string _str_literal_%d(%s, %d); _str_literal_%d;})" %
                (slid, c_str_literal(t.value), len(t.value), slid))
        ppgoc_util.raise_bug()

    if expr.op == "convert":
        tp, e = expr.arg
        ec = gen_expr_code(e, pos_info)
        return gen_wrap_convert(ec, e.tp, tp)

    if expr.op in ("call_vec_method", "call_vec_view_method", "call_map_method", "call_set_method"):
        if expr.op == "call_vec_method":
            mnc = mncs[ppgoc_util.MN_BUILTINS + "/vecs"]
        elif expr.op == "call_vec_view_method":
            mnc = mncs[ppgoc_util.MN_BUILTINS + "/vec_views"]
        elif expr.op == "call_map_method":
            mnc = mncs[ppgoc_util.MN_BUILTINS + "/maps"]
        elif expr.op == "call_set_method":
            mnc = mncs[ppgoc_util.MN_BUILTINS + "/sets"]
        else:
            ppgoc_util.raise_bug()
        oe, method_name, el = expr.arg
        el = [oe] + el
        tps = expr.tp.multi_tps if expr.tp.is_multi else [expr.tp]

        rv = "ret_%d" % new_id()
        excv = "exc_%d" % new_id()
        cs = [
            "({",
            "%s %s;" % (gen_func_ret_tp_code_from_tps(tps), rv),
            "auto %s = ::ppgo::%s::func_%s(%s" %
                (excv, mnc, method_name, rv),
        ]
        ecs = ["(%s)" % gen_expr_code(e, pos_info) for e in el]
        if ecs:
            cs.append(", " + ", ".join(ecs))
        cs.append(");")
        t, fom = pos_info
        cs += [
            "if (%s) {" % excv,
            "%s->PushTB(%s, %d, %s);" %
                (excv, c_str_literal(t.src_file), t.line_no,
                 c_str_literal("<none>" if fom is None else str(fom))),
            "return %s;}" % excv,
        ]
        if len(tps) == 1:
            cs.append("::std::get<0>(%s);" % rv)
        else:
            cs.append("%s;" % rv)
        cs.append("})")
        return "\n".join(cs)

    if expr.op == "gv":
        gv = expr.arg
        c = "::ppgo::%s::gv_%s" % (mncs[gv.mod.name], gv.name)
        return c

    if expr.op == "lv":
        name = expr.arg
        c = "l_%s" % name
        return c

    if expr.op in ("~", "!", "neg", "pos"):
        e = expr.arg
        return "%s(%s)" % ({"neg" : "-", "pos" : "+"}.get(expr.op, expr.op), gen_expr_code(e, pos_info))

    if expr.op in ppgoc_token.BINOCULAR_OP_SYM_SET:
        ea, eb = expr.arg
        if ea.tp.is_str_type and eb.tp.is_str_type:
            if expr.op == "+":
                return "(%s).Concat(%s)" % (gen_expr_code(ea, pos_info), gen_expr_code(eb, pos_info))
            if expr.op in ("==", "!=", "<", ">", "<=", ">="):
                return (
                    "(%s).Cmp(%s) %s 0" %
                    (gen_expr_code(ea, pos_info), gen_expr_code(eb, pos_info), expr.op))
        if ea.tp.is_any and eb.tp.is_any:
            if expr.op in ("==", "!="):
                return (
                    "%s::ppgo::Any::Equal((%s), (%s))" %
                    ("" if expr.op == "==" else "!",
                     gen_expr_code(ea, pos_info), gen_expr_code(eb, pos_info)))
        return "(%s) %s (%s)" % (gen_expr_code(ea, pos_info), expr.op, gen_expr_code(eb, pos_info))

    if expr.op == "str[]":
        se, ie = expr.arg
        return "(%s).ByteAt(%s)" % (gen_expr_code(se, pos_info), gen_expr_code(ie, pos_info))

    if expr.op == "str[:]":
        se, ibe, iee = expr.arg
        sec = gen_expr_code(se, pos_info)
        ibec = "0" if ibe is None else gen_expr_code(ibe, pos_info)
        if iee is None:
            return "(%s).SliceSubString(%s)" % (sec, ibec)
        else:
            return "(%s).SliceSubString(%s, %s)" % (sec, ibec, gen_expr_code(iee, pos_info))

    if expr.op == "if-else":
        e_cond, ea, eb = expr.arg
        return (
            "(%s) ? (%s) : (%s)" %
            (gen_expr_code(e_cond, pos_info), gen_expr_code(ea, pos_info), gen_expr_code(eb, pos_info)))

    if expr.op in ("vec[]", "map[]"):
        oe, ie = expr.arg
        if mode == "r":
            method = "Get"
        elif mode == "w":
            method = "GetForSet"
        else:
            assert mode == "rw"
            method = "GetRef"
        return "(%s).%s(%s)" % (gen_expr_code(oe, pos_info), method, gen_expr_code(ie, pos_info))

    if expr.op == "vec[:]":
        oe, ibe, iee = expr.arg
        etpc = gen_tp_code(expr.tp)
        oec = gen_expr_code(oe, pos_info)
        ibec = "0" if ibe is None else gen_expr_code(ibe, pos_info)
        if iee is None:
            return "%s(%s, %s)" % (etpc, oec, ibec)
        else:
            return "%s(%s, %s, %s)" % (etpc, oec, ibec, gen_expr_code(iee, pos_info))

    if expr.op == ".":
        oe, a = expr.arg
        c = "(%s)->attr_%s" % (gen_expr_code(oe, pos_info), a.name)
        return c

    if expr.op == "call_method":
        oe, m, el = expr.arg
        oec = gen_expr_code(oe, pos_info)
        rv = "ret_%d" % new_id()
        excv = "exc_%d" % new_id()
        cs = [
            "({",
            "%s %s;" % (gen_method_ret_tp_code(m), rv),
            "auto %s = ::ppgo::util::CopyPtr(%s)->method_%s(%s" % (excv, oec, m.name, rv),
        ]
        ecs = ["(%s)" % gen_expr_code(e, pos_info) for e in el]
        if ecs:
            cs.append(", " + ", ".join(ecs))
        cs.append(");")
        t, fom = pos_info
        cs += [
            "if (%s) {" % excv,
            "%s->PushTB(%s, %d, %s);" %
                (excv, c_str_literal(t.src_file), t.line_no,
                 c_str_literal("<none>" if fom is None else str(fom))),
            "return %s;}" % excv,
        ]
        if len(m.ret_defs) == 1:
            cs.append("::std::get<0>(%s);" % rv)
        else:
            cs.append("%s;" % rv)
        cs.append("})")
        return "\n".join(cs)

    if expr.op == "new":
        cls, el = expr.arg
        newv = "new_%d" % new_id()
        cs = [
            "({",
            "%s %s(new ::ppgo::%s::cls_%s);" % (gen_tp_code(expr.tp), newv, mncs[cls.mod.name], cls.name),
        ]
        init_method = cls.get_init_method()
        if init_method is not None:
            assert not init_method.ret_defs
            rv = "ret_%d" % new_id()
            excv = "exc_%d" % new_id()
            cs += [
                "::std::tuple<> %s;" % rv,
                "auto %s = %s->method_init(%s" % (excv, newv, rv),
            ]
            ecs = ["(%s)" % gen_expr_code(e, pos_info) for e in el]
            if ecs:
                cs.append(", " + ", ".join(ecs))
            cs.append(");")
            t, fom = pos_info
            cs += [
                "if (%s) {" % excv,
                "%s->PushTB(%s, %d, %s);" %
                    (excv, c_str_literal(t.src_file), t.line_no,
                    c_str_literal("<none>" if fom is None else str(fom))),
                "return %s;}" % excv,
            ]
        cs.append("%s;})" % newv)
        return "\n".join(cs)

    if expr.op == "str_fmt":
        fmt, el = expr.arg
        return (
            "::ppgo::tp_string::Sprintf(%s, %s)" %
            (c_str_literal(fmt), ", ".join([gen_expr_code(e, pos_info) for e in el])))

    if expr.op == "this":
        return "this"

    if expr.op == "this_sp":
        return "shared_from_this()"

    if expr.op == "%d":
        e = expr.arg
        if e.tp.is_unsigned_integer_type:
            return "static_cast<unsigned long long>(%s)" % gen_expr_code(e, pos_info)
        assert e.tp.is_integer_type
        return "static_cast<long long>(%s)" % gen_expr_code(e, pos_info)

    if expr.op == "%Lf":
        e = expr.arg
        assert e.tp.is_float_type
        return "static_cast<long double>(%s)" % gen_expr_code(e, pos_info)

    if expr.op == "new_vec":
        tp, el = expr.arg
        assert tp.is_vec
        return "%s({%s})" % (gen_tp_code(tp), ", ".join([gen_expr_code(e, pos_info) for e in el]))

    if expr.op == "new_map":
        tp, kvel = expr.arg
        assert tp.is_map
        ktp, vtp = tp.map_kv_tp
        ecs = []
        for ke, ve in kvel:
            ecs.append(
                "::std::pair<const %s, %s>((%s), (%s))" %
                (gen_tp_code(ktp), gen_tp_code(vtp),
                gen_expr_code(ke, pos_info), gen_expr_code(ve, pos_info)))
        return "%s({%s})" % (gen_tp_code(tp), ", ".join(ecs))

    if expr.op == "new_set":
        tp, el = expr.arg
        assert tp.is_set
        return "%s({%s})" % (gen_tp_code(tp), ", ".join([gen_expr_code(e, pos_info) for e in el]))

    if expr.op == "%s":
        e = expr.arg
        assert e.tp == ppgoc_type.ANY_TYPE
        return "::ppgo::Any::ToStr(%s).Data()" % gen_expr_code(e, pos_info)

    if expr.op == "%T":
        e = expr.arg
        assert e.tp == ppgoc_type.ANY_TYPE
        return "::ppgo::Any::GetTypeName(%s).c_str()" % gen_expr_code(e, pos_info)

    if expr.op == "%t":
        e = expr.arg
        assert e.tp.is_bool_type
        return '(%s) ? "true" : "false"' % gen_expr_code(e, pos_info)

    if expr.op in ("str%x", "str%X"):
        e = expr.arg
        assert e.tp.is_str_type
        return (
            "(%s).Hex(%s).Data()" %
            (gen_expr_code(e, pos_info), "true" if expr.op[-1] == 'X' else "false"))

    if expr.op == "assert_type":
        tp, e = expr.arg
        ec = gen_expr_code(e, pos_info)
        if not e.tp.is_any:
            assert e.tp.is_coi_type
            ec = "std::static_pointer_cast<::ppgo::Any>(%s)" % ec
        rv = "ret_%d" % new_id()
        excv = "exc_%d" % new_id()
        t, fom = pos_info
        cs = [
            "({",
            "%s %s;" % (gen_tp_code(tp), rv),
            "auto %s = ::ppgo::AssertType((%s), %s);" % (excv, ec, rv),
            "if (%s) {" % excv,
            "%s->PushTB(%s, %d, %s);" %
                (excv, c_str_literal(t.src_file), t.line_no,
                 c_str_literal("<none>" if fom is None else str(fom))),
            "return %s;}" % excv,
            "%s; })" % rv,
        ]
        return "\n".join(cs)

    if expr.op == "()":
        fe, el = expr.arg
        assert fe.tp.is_func
        atps, rtps = fe.tp.func_arg_ret_tps
        rv = "ret_%d" % new_id()
        excv = "exc_%d" % new_id()
        cs = [
            "({",
            "%s %s;" % (gen_func_ret_tp_code_from_tps(rtps), rv),
            "auto %s = (%s)(%s" % (excv, gen_expr_code(fe, pos_info), rv),
        ]
        ecs = ["(%s)" % gen_expr_code(e, pos_info) for e in el]
        if ecs:
            cs.append(", " + ", ".join(ecs))
        cs.append(");")
        t, fom = pos_info
        cs += [
            "if (%s) {" % excv,
            "%s->PushTB(%s, %d, %s);" %
                (excv, c_str_literal(t.src_file), t.line_no,
                 c_str_literal("<none>" if fom is None else str(fom))),
            "return %s;}" % excv,
        ]
        if len(rtps) == 1:
            cs.append("::std::get<0>(%s);" % rv)
        else:
            cs.append("%s;" % rv)
        cs.append("})")
        return "\n".join(cs)

    if expr.op == "closure":
        f = expr.arg
        rv = "ret_%d" % new_id()
        cs = ["([&](%s &%s" % (gen_func_ret_tp_code(f), rv)]
        for name, (_, tp) in f.arg_defs.iteritems():
            cs.append(", ")
            cs.append("%s l_%s" % (gen_tp_code(tp), name))
        cs.append(") -> ::ppgo::Exc::Ptr {\n")
        ret_var_name_stk.append(rv)
        code = Code(None)
        with code:
            output_fom_named_ret_vars(code, f)
            output_stmts(code, f.stmts)
        cs.append(code.content)
        ret_var_name_stk.pop()
        cs.append("return nullptr;})")
        return "".join(cs)

    if expr.op == "call_optional_method":
        oe, method_name, el = expr.arg
        return (
            "::ppgo::util::optional_type_method::%s(%s)" %
            (method_name, ", ".join([gen_expr_code(e, pos_info) for e in ([oe] + el)])))

    if expr.op == "make_empty_optional":
        tp = expr.arg
        return "%s()" % gen_tp_code(tp)

    if expr.op == "make_optional":
        e = expr.arg
        tp = expr.tp
        return "%s(%s)" % (gen_tp_code(tp), gen_expr_code(e, pos_info))

    print expr.op
    ppgoc_util.raise_bug()

def gen_var_init_code(tp):
    if tp.is_bool_type:
        return " = false"
    if tp.is_number_type:
        return " = 0"
    if tp.is_uptr_type:
        return " = nullptr"
    return ""

ret_var_name_stk = []
def get_ret_var_name():
    return ret_var_name_stk[-1] if ret_var_name_stk else "ret"

def output_stmts(code, stmts):
    for stmt in stmts:
        if stmt.type == "expr":
            ec = gen_expr_code(stmt.expr)
            code += "(void)(%s);" % ec
            continue

        if stmt.type == "block":
            with code.new_blk(""):
                output_stmts(code, stmt.stmts)
            continue

        if stmt.type == "for_vec":
            with code.new_blk(""):
                vv = "vec_%d" % new_id()
                code += "auto %s = (%s);" % (vv, gen_expr_code(stmt.expr))
                viv = "vec_idx_%d" % new_id()
                with code.new_blk("for (::ppgo::tp_int %s = 0; %s < %s.Len(); ++ %s)" %
                                  (viv, viv, vv, viv)):
                    if not stmt.idx_var_name.startswith("_."):
                        code += "::ppgo::tp_int l_%s = %s;" % (stmt.idx_var_name, viv)
                    if not stmt.value_var_name.startswith("_."):
                        code += (
                            "%s l_%s = %s.Get(%s);" %
                            (gen_tp_code(stmt.expr.tp.vec_elem_tp or stmt.expr.tp.vec_view_elem_tp),
                             stmt.value_var_name, vv, viv))
                    output_stmts(code, stmt.stmts)
            continue

        if stmt.type == "for_map":
            ktp, vtp = stmt.expr.tp.map_kv_tp
            with code.new_blk(""):
                mv = "map_%d" % new_id()
                code += "auto %s = (%s);" % (mv, gen_expr_code(stmt.expr))
                miv = "map_iter_%d" % new_id()
                with code.new_blk("for (auto %s = %s.NewIter(); %s.Valid(); %s.Inc())" %
                                  (miv, mv, miv, miv)):
                    if not stmt.k_var_name.startswith("_."):
                        code += "%s l_%s = %s.Key();" % (gen_tp_code(ktp), stmt.k_var_name, miv)
                    if not stmt.v_var_name.startswith("_."):
                        code += "%s l_%s = %s.Value();" % (gen_tp_code(vtp), stmt.v_var_name, miv)
                    output_stmts(code, stmt.stmts)
            continue

        if stmt.type == "for_set":
            with code.new_blk(""):
                sv = "set_%d" % new_id()
                code += "auto %s = (%s);" % (sv, gen_expr_code(stmt.expr))
                siv = "set_iter_%d" % new_id()
                with code.new_blk("for (auto %s = %s.NewIter(); %s.Valid(); %s.Inc())" %
                                  (siv, sv, siv, siv)):
                    if not stmt.var_name.startswith("_."):
                        code += "%s l_%s = %s.Elem();" % (gen_tp_code(stmt.expr.tp.set_elem_tp), stmt.var_name, siv)
                    output_stmts(code, stmt.stmts)
            continue

        if stmt.type in ("break", "continue"):
            code += "%s;" % stmt.type
            continue

        if stmt.type == "var":
            code += "%s l_%s%s;" % (gen_tp_code(stmt.tp), stmt.name, gen_var_init_code(stmt.tp))
            continue

        if stmt.type == "var_with_init":
            code += "%s l_%s = (%s);" % (gen_tp_code(stmt.tp), stmt.name, gen_expr_code(stmt.expr))
            continue

        if stmt.type == "var_init":
            ec = gen_expr_code(stmt.expr)
            if stmt.expr.tp.is_multi:
                tps = stmt.expr.tp.multi_tps
                assert len(tps) == 0 or len(tps) > 1
                if len(stmt.new_vars) == len(tps):
                    code += (
                        "::std::tie(%s) = (%s);" % (
                            ",".join([
                                ("::std::ignore" if name.startswith("_.") else "l_" + name)
                                for name in stmt.new_vars]),
                            ec))
                else:
                    rv_tp = gen_func_ret_tp_code_from_tps(tps)
                    rv = "ret_%d" % new_id()
                    ecf = (
                        "[&] (%s &%s) -> ::ppgo::Exc::Ptr { %s = (%s); return nullptr; }" %
                        (rv_tp, rv, rv, ec))
                    rv = "ret_%d" % new_id()
                    code += "%s %s;" % (rv_tp, rv)
                    excv = "exc_%d" % new_id()
                    code += "auto %s = (%s)(%s);" % (excv, ecf, rv)
                    assert len(stmt.new_vars) > len(tps)
                    code += "if (%s) {" % excv
                    exc_var_name = stmt.new_vars.key_at(len(tps))
                    if not exc_var_name.startswith("_."):
                        code += "l_%s = %s->Throwed();" % (exc_var_name, excv)
                    if len(stmt.new_vars) > len(tps) + 1:
                        assert len(stmt.new_vars) == len(tps) + 2
                        tb_var_name = stmt.new_vars.key_at(len(tps) + 1)
                        if not tb_var_name.startswith("_."):
                            code += "l_%s = %s->FormatWithTB();" % (tb_var_name, excv)
                    code += (
                        "} else { ::std::tie(%s) = %s; }" % (
                            ",".join([
                                ("::std::ignore" if name.startswith("_.") else "l_" + name)
                                for name in list(stmt.new_vars)[: len(tps)]]),
                            rv))
            else:
                if len(stmt.new_vars) == 1:
                    var_name = stmt.new_vars.key_at(0)
                    if var_name.startswith("_."):
                        code += "(void)(%s);" % ec
                    else:
                        code += "l_%s = (%s);" % (var_name, ec)
                else:
                    rv_tp = gen_tp_code(stmt.expr.tp)
                    rv = "ret_%d" % new_id()
                    ecf = (
                        "[&] (%s &%s) -> ::ppgo::Exc::Ptr { %s = (%s); return nullptr; }" %
                        (rv_tp, rv, rv, ec))
                    rv = "ret_%d" % new_id()
                    code += "%s %s%s;" % (rv_tp, rv, gen_var_init_code(stmt.expr.tp))
                    excv = "exc_%d" % new_id()
                    code += "auto %s = (%s)(%s);" % (excv, ecf, rv)
                    assert len(stmt.new_vars) > 1
                    code += "if (%s) {" % excv
                    exc_var_name = stmt.new_vars.key_at(1)
                    if not exc_var_name.startswith("_."):
                        code += "l_%s = %s->Throwed();" % (exc_var_name, excv)
                    if len(stmt.new_vars) > 2:
                        assert len(stmt.new_vars) == 3
                        tb_var_name = stmt.new_vars.key_at(2)
                        if not tb_var_name.startswith("_."):
                            code += "l_%s = %s->FormatWithTB();" % (tb_var_name, excv)
                    code += "}"
                    var_name = stmt.new_vars.key_at(0)
                    if not var_name.startswith("_."):
                        code += "else { l_%s = %s; }" % (var_name, rv)
            continue

        if stmt.type == "return":
            if stmt.expr is not None:
                code += "::std::get<0>(%s) = (%s);" % (get_ret_var_name(), gen_expr_code(stmt.expr))
            code += "return nullptr;"
            continue

        if stmt.type == "while":
            with code.new_blk("while (%s)" % gen_expr_code(stmt.expr)):
                output_stmts(code, stmt.stmts)
            continue

        if stmt.type == "if":
            assert stmt.if_expr_stmts_list
            for i, (e, if_stmts) in enumerate(stmt.if_expr_stmts_list):
                with code.new_blk(
                    "%sif (%s)" % ("" if i == 0 else "else ", gen_expr_code(e)),
                    start_with_blank_line = i == 0
                ):
                    output_stmts(code, if_stmts)
            if stmt.else_stmts is not None:
                with code.new_blk("else", start_with_blank_line = False):
                    output_stmts(code, stmt.else_stmts)
            continue

        if stmt.type == "=":
            ec = gen_expr_code(stmt.expr)
            if stmt.expr.tp.is_multi:
                tps = stmt.expr.tp.multi_tps
                assert len(tps) == 0 or len(tps) > 1
                if len(stmt.lvalues) == len(tps):
                    rv = "ret_%d" % new_id()
                    code += "auto %s = (%s);" % (rv, ec)
                    for i, (lvalue, tp) in enumerate(zip(stmt.lvalues, tps)):
                        if lvalue is not None:
                            c = "::std::get<%d>(%s)" % (i, rv)
                            if lvalue.tp != tp:
                                assert lvalue_tp.can_convert_from(tp)
                                c = gen_wrap_convert(c, tp, lvalue_tp)
                            code += "(%s) = (%s);" % (gen_expr_code(lvalue, mode = "w"), c)
                else:
                    rv_tp = gen_func_ret_tp_code_from_tps(tps)
                    rv = "ret_%d" % new_id()
                    ecf = (
                        "[&] (%s &%s) -> ::ppgo::Exc::Ptr { %s = (%s); return nullptr; }" %
                        (rv_tp, rv, rv, ec))
                    rv = "ret_%d" % new_id()
                    code += "%s %s;" % (rv_tp, rv)
                    excv = "exc_%d" % new_id()
                    code += "auto %s = (%s)(%s);" % (excv, ecf, rv)
                    assert len(stmt.lvalues) > len(tps)
                    code += "if (%s) {" % excv
                    lvalue = stmt.lvalues[len(tps)]
                    if lvalue is not None:
                        code += "(%s) = %s->Throwed();" % (gen_expr_code(lvalue, mode = "w"), excv)
                    if len(stmt.lvalues) > len(tps) + 1:
                        assert len(stmt.lvalues) == len(tps) + 2
                        lvalue = stmt.lvalues[len(tps) + 1]
                        if lvalue is not None:
                            code += (
                                "(%s) = %s->FormatWithTB();" %
                                (gen_expr_code(lvalue, mode = "w"), excv))
                    code += "} else {"
                    for i, (lvalue, tp) in enumerate(zip(stmt.lvalues[: len(tps)], tps)):
                        if lvalue is not None:
                            c = "::std::get<%d>(%s)" % (i, rv)
                            if lvalue.tp != tp:
                                assert lvalue_tp.can_convert_from(tp)
                                c = gen_wrap_convert(c, tp, lvalue_tp)
                            code += "(%s) = (%s);" % (gen_expr_code(lvalue, mode = "w"), c)
                    lvalue = stmt.lvalues[len(tps)]
                    if lvalue is not None:
                        code += "(%s) = ::ppgo::Any::Ptr();" % gen_expr_code(lvalue, mode = "w")
                    if len(stmt.lvalues) > len(tps) + 1:
                        lvalue = stmt.lvalues[len(tps) + 1]
                        if lvalue is not None:
                            code += "(%s) = ::ppgo::tp_string();" % gen_expr_code(lvalue, mode = "w")
                    code += "}"
            else:
                assert len(stmt.lvalues) >= 1
                lvalue = stmt.lvalues[0]
                if lvalue is not None and lvalue.tp != stmt.expr.tp:
                    assert lvalue.tp.can_convert_from(stmt.expr.tp)
                    ec = gen_wrap_convert(ec, stmt.expr.tp, lvalue.tp)
                if len(stmt.lvalues) == 1:
                    lvalue = stmt.lvalues[0]
                    if lvalue is None:
                        code += "(void)(%s);" % ec
                    else:
                        code += "(%s) = (%s);" % (gen_expr_code(lvalue, mode = "w"), ec)
                else:
                    rv_tp = gen_tp_code(stmt.expr.tp)
                    rv = "ret_%d" % new_id()
                    ecf = (
                        "[&] (%s &%s) -> ::ppgo::Exc::Ptr { %s = (%s); return nullptr; }" %
                        (rv_tp, rv, rv, ec))
                    rv = "ret_%d" % new_id()
                    code += "%s %s%s;" % (rv_tp, rv, gen_var_init_code(stmt.expr.tp))
                    excv = "exc_%d" % new_id()
                    code += "auto %s = (%s)(%s);" % (excv, ecf, rv)
                    code += "if (%s) {" % excv
                    if stmt.lvalues[1] is not None:
                        code += (
                            "(%s) = %s->Throwed();" %
                            (gen_expr_code(stmt.lvalues[1], mode = "w"), excv))
                    if len(stmt.lvalues) > 2:
                        assert len(stmt.lvalues) == 3
                        if stmt.lvalues[2] is not None:
                            code += (
                                "(%s) = %s->FormatWithTB();" %
                                (gen_expr_code(stmt.lvalues[2], mode = "w"), excv))
                    code += "} else {"
                    if stmt.lvalues[0] is not None:
                        code += "(%s) = %s;" % (gen_expr_code(stmt.lvalues[0], mode = "w"), rv)
                    if stmt.lvalues[1] is not None:
                        code += (
                            "(%s) = ::ppgo::Any::Ptr();" % gen_expr_code(stmt.lvalues[1], mode = "w"))
                    if len(stmt.lvalues) > 2 and stmt.lvalues[2] is not None:
                        code += (
                            "(%s) = ::ppgo::tp_string();" %
                            gen_expr_code(stmt.lvalues[2], mode = "w"))
                    code += "}"
            continue

        if stmt.type in ("++", "--"):
            code += "%s (%s);" % (stmt.type, gen_expr_code(stmt.lvalue, mode = "rw"))
            continue

        if stmt.type == "op=":
            code += "(%s) %s= (%s);" % (gen_expr_code(stmt.lvalue, mode = "rw"), stmt.op, gen_expr_code(stmt.expr))
            continue

        if stmt.type == "for..":
            ea, eb = stmt.ep
            assert ea.tp == eb.tp and ea.tp.is_integer_type
            iv = "idx_%d" % new_id()
            endv = "end_%d" % new_id()
            with code.new_blk(
                "for (%s %s = (%s), %s = (%s); %s < %s; ++ %s)" %
                (gen_tp_code(ea.tp), iv, gen_expr_code(ea), endv, gen_expr_code(eb), iv, endv, iv)):
                if not stmt.var_name.startswith("_."):
                    code += "auto l_%s = %s;" % (stmt.var_name, iv)
                output_stmts(code, stmt.stmts)
            continue

        if stmt.type == "with":
            with code.new_blk(""):
                wv = "with_%d" % new_id()
                wtpc = gen_tp_code(ppgoc_type.WITHABLE_TYPE)
                P, S = "std::shared_ptr<", ">"
                assert wtpc.startswith(P) and wtpc.endswith(S)
                code += "::ppgo::WithGuard<%s> %s{%s};" % (wtpc[len(P):-len(S)], wv, gen_expr_code(stmt.expr))
                excv = "exc_%d" % new_id()
                code += "auto %s = %s.ExcOfEnter();" % (excv, wv)
                with code.new_blk("if (%s)" % excv):
                    t, fom = stmt.expr.pos_info
                    code += "%s->PushTB(%s, %d, %s);" % (
                        excv, c_str_literal(t.src_file), t.line_no,
                        c_str_literal("<none>" if fom is None else str(fom)))
                    code += "return %s;" % excv
                output_stmts(code, stmt.stmts)
            continue

        print stmt.type
        ppgoc_util.raise_bug()

def output_fom_named_ret_vars(code, fom):
    if len(fom.ret_defs) == 1 and None in fom.ret_defs:
        return
    for i, name in enumerate(fom.ret_defs):
        assert name
        code += "auto &l_%s = ::std::get<%d>(%s);" % (name, i, get_ret_var_name())

def output_prog_cpp():
    for mod in ppgoc_mod.mods.itervalues():
        with Code(ppgoc_env.out_dir + "/prog-%s.cpp" % gen_mnc(mod), use_id = True) as code:
            code += '#include "ppgo.h"'
            with code.new_blk("namespace ppgo"):
                with code.new_blk(gen_ns_code(mod)):

                    for cls in mod.clses.itervalues():
                        with code.new_blk("cls_%s::~cls_%s()" % (cls.name, cls.name)):
                            if cls.get_deinit_method() is not None:
                                code += "::std::tuple<> ret;"
                                code += "auto exc = this->method_deinit(ret);"
                                with code.new_blk("if (exc)", start_with_blank_line = False):
                                    code += "auto ftb = exc->FormatWithTB();"
                                    code += (
                                        '::ppgo::util::OutputUnexpectedErrMsg(::lom::Sprintf('
                                        '"Uncached exception in " %s ".deinit: %%s\\n", '
                                        'ftb.Data()));' % c_str_literal(str(cls)))
                        for m in cls.methods.itervalues():
                            if m.stmts is not None:
                                with code.new_blk(gen_method_def(m, with_cls_name = True)):
                                    output_fom_named_ret_vars(code, m)
                                    output_stmts(code, m.stmts)
                                    code += "return nullptr;"
                    for func in mod.funcs.itervalues():
                        if func.stmts is not None:
                            with code.new_blk(gen_func_def(func)):
                                output_fom_named_ret_vars(code, func)
                                output_stmts(code, func.stmts)
                                code += "return nullptr;"
                    for gv in mod.gvs.itervalues():
                        code += "%s gv_%s;" % (gen_tp_code(gv.tp), gv.name)

                    with code.new_blk("::ppgo::Exc::Ptr init()"):
                        code += "static bool inited = false;"
                        code += "if (inited) { return nullptr; } else { inited = true; }"
                        for mn in set(mod.iter_dep_mns()):
                            mnc = mncs[mn]
                            with code.new_blk(""):
                                code += "auto exc = ::ppgo::%s::init();" % mnc
                                code += "if (exc) { return exc; }"
                        for gv in mod.gvs.itervalues():
                            if gv.init_expr is not None:
                                code += "gv_%s = (%s);" % (gv.name, gen_expr_code(gv.init_expr))
                        if "init" in mod.funcs:
                            init_func = mod.funcs["init"]
                            assert not init_func.arg_defs and not init_func.ret_defs
                            code += "::std::tuple<> ret; return func_init(ret);"
                        else:
                            code += "return nullptr;"

    with Code(ppgoc_env.out_dir + "/prog-main.cpp") as code:
        code += '#include "ppgo.h"'
        with code.new_blk("namespace ppgo"):
            with code.new_blk("Exc::Ptr main()"):
                #内建模块没有显式import，需要提前初始化
                for mn, mnc in mncs.iteritems():
                    if mn.startswith("__builtins"):
                        with code.new_blk(""):
                            code += "auto exc = ::ppgo::%s::init(); if (exc) { return exc; }" % mnc
                with code.new_blk(""):
                    code += "auto exc = ::ppgo::%s::init(); if (exc) { return exc; }" % main_mnc
                with code.new_blk(""):
                    code += "::std::tuple<> ret;"
                    code += "return ::ppgo::%s::func_main(ret);" % main_mnc

def output_native_src():
    hfns = []
    for mod in ppgoc_mod.mods.itervalues():
        mnc = mncs[mod.name]
        for fn in os.listdir(mod.dir):
            if fn.endswith(".h") or fn.endswith(".cpp"):
                out_fn = mnc + "-" + fn
                if out_fn.endswith(".h"):
                    hfns.append(out_fn)
                with open(ppgoc_env.out_dir + "/" + out_fn, "w") as out_f:
                    print >> out_f, '#line 1 "PPGO_LIB/%s/%s"' % (mod.name, fn)
                    for line_idx, line in enumerate(open(mod.dir + "/" + fn)):
                        if re.match(r"^\s*#\s*pragma\s+ppgo\s+define-THIS_MOD\s*$", line):
                            print >> out_f, "#ifdef PPGO_THIS_MOD"
                            print >> out_f, "#error macro PPGO_THIS_MOD redefined"
                            print >> out_f, "#endif"
                            print >> out_f, "#define PPGO_THIS_MOD %s" % mnc
                            print >> out_f, "#line %d" % (line_idx + 2)
                        elif re.match(r"^\s*#\s*pragma\s+ppgo\s+undef-THIS_MOD\s*$", line):
                            print >> out_f, "#undef PPGO_THIS_MOD"
                        else:
                            out_f.write(line)
    return hfns

def output_conf_header():
    with Code(ppgoc_env.out_dir + "/_conf.h") as code:
        code += "#pragma once"
        #with ppgoc_env.out_confs

def cp_runtime():
    for fn in os.listdir(ppgoc_env.runtime_dir):
        shutil.copy(ppgoc_env.runtime_dir + "/" + fn, ppgoc_env.out_dir)

def make_deps():
    rc = os.system("make -C %s >/dev/null" % ppgoc_env.deps_dir)
    if rc != 0:
        sys.exit(rc)

def make_prog():
    with Code(ppgoc_env.out_dir + "/Makefile.def") as code:
        code += "PPGO_DIR := %s" % ppgoc_env.ppgo_dir
        assert main_mnc == os.path.basename(exe_file)
        code += "PPGO_MK_OUT := %s" % main_mnc
        code += "PPGO_OUT_OBJ_CACHE_DIR := %s" % ppgoc_env.out_obj_cache_dir
    rc = os.system("make -C %s fast >/dev/null" % ppgoc_env.out_dir)
    if rc != 0:
        sys.exit(rc)

def make_out_bin(out_bin):
    global exe_file
    if platform.system() not in ("Darwin", "Linux"):
        ppgoc_util.raise_bug()
    if os.path.exists(exe_file):
        shutil.copy(exe_file, out_bin)
        exe_file = out_bin
    else:
        ppgoc_util.exit("找不到可执行文件[%s]" % exe_file)

def run_prog(args_for_run):
    if platform.system() not in ("Darwin", "Linux"):
        ppgoc_util.raise_bug()
    if os.path.exists(exe_file):
        os.execv(exe_file, [exe_file] + args_for_run)
    else:
        ppgoc_util.exit("找不到可执行文件[%s]" % exe_file)

def output(out_bin, need_run, args_for_run):
    output_start_time = time.time()
    ppgoc_util.vlog("开始输出C++代码")

    gen_all_mncs()

    global main_mod, main_mnc

    main_mod = ppgoc_mod.mods[ppgoc_env.main_mn]
    main_mnc = gen_mnc(main_mod)

    shutil.rmtree(ppgoc_env.out_dir, True)
    os.makedirs(ppgoc_env.out_dir)

    global exe_file

    exe_file = "%s/%s" % (ppgoc_env.out_dir, main_mnc)

    native_header_fns = output_native_src()
    output_prog_h(native_header_fns)
    output_prog_cpp()
    output_conf_header()

    cp_runtime()

    ppgoc_util.vlog("C++代码输出完毕，耗时%.2f秒" % (time.time() - output_start_time))

    ppgoc_util.vlog("make deps")
    make_deps()

    make_start_time = time.time()
    ppgoc_util.vlog("开始执行make")
    make_prog()
    if out_bin is not None:
        make_out_bin(out_bin)
    ppgoc_util.vlog("make完毕，耗时%.2f秒" % (time.time() - make_start_time))

    ppgoc_util.output_all_warnings()

    if need_run:
        run_prog(args_for_run)
