#coding=utf8

import ppgoc_util, ppgoc_mod, ppgoc_token

BASE_TYPES = (["bool", "byte", "int", "uint", "float", "string", "any"] +
    ["%s%d" % (iou, bc) for iou in "iu" for bc in (8, 16, 32, 64)] +
    ["float%d" % bc for bc in (32, 64)])

class Type:
    def __init__(self, t, name, mn = None):
        self.t = t
        self.name = name
        self.mn = mn

        self.vec_elem_tp = None
        self.map_kv_tp = None
        self.func_arg_ret_tps = None
        self.multi_tps = None

        self.is_checked = False

        self._set_is_XXX()

    def _set_is_XXX(self):
        self.is_vec = self.vec_elem_tp is not None
        self.is_map = self.map_kv_tp is not None
        self.is_func = self.func_arg_ret_tps is not None
        self.is_multi = self.multi_tps is not None
        self.is_nil = self.t.is_reserved("nil")
        self.is_coi_type = self.t.is_name
        self.is_obj_type = self.is_nil or self.is_coi_type
        self.is_bool_type = self.t.is_reserved("bool")
        self.is_str_type = self.t.is_reserved("string")
        self.is_any = self.t.is_reserved("any")
        self.is_integer_type = (
            self.t.is_reserved and self.name in (
                ["byte", "int", "uint"] + ["%s%d" % (iou, bc) for iou in "iu" for bc in (8, 16, 32, 64)]))
        self.is_unsigned_integer_type = (
            self.is_integer_type and (self.name[0] == "u" or self.name == "byte"))
        self.is_byte_type = self.t.is_reserved("byte")
        self.is_float_type = self.t.is_reserved and self.name in ("float", "float32", "float64")
        self.is_number_type = self.is_integer_type or self.is_float_type
        self.can_inc_dec = self.is_integer_type
        self.is_base_type = self.t.is_reserved and self.t.value in BASE_TYPES

    def __repr__(self):
        if self.vec_elem_tp is not None:
            return "[]%s" % self.vec_elem_tp
        if self.map_kv_tp is not None:
            return "[%s]%s" % self.map_kv_tp
        if self.func_arg_ret_tps is not None:
            atps, rtps = self.func_arg_ret_tps
            return "func (%s) (%s)" % (", ".join(str(tp) for tp in atps), ", ".join(str(tp) for tp in rtps))
        if self.multi_tps is not None:
            return "multi-tps%r" % self.multi_tps

        return ("" if self.mn is None else self.mn + ".") + self.name

    __str__ = __repr__

    def _freeze(self):
        self.is_freezed = True

    def _unfreeze(self):
        assert self.is_freezed
        del self.__dict__["is_freezed"]

    def __setattr__(self, name, value):
        if self.__dict__.get("is_freezed", False):
            ppgoc_util.raise_bug()
        self.__dict__[name] = value

    def __delattr__(self, name):
        if self.__dict__.get("is_freezed", False):
            ppgoc_util.raise_bug()
        del self.__dict__[name]

    def _set_is_checked(self):
        self.is_checked = True
        self._freeze()

    def __eq__(self, other):
        if self.is_vec or other.is_vec:
            return self.is_vec and other.is_vec and self.vec_elem_tp == other.vec_elem_tp
        if self.is_map or other.is_map:
            return self.is_map and other.is_map and self.map_kv_tp == other.map_kv_tp
        if self.is_func or other.is_func:
            if not (self.is_func and other.is_func):
                return False
            atps, rtps = self.func_arg_ret_tps
            oatps, ortps = other.func_arg_ret_tps
            if len(atps) != len(oatps) or len(rtps) != len(ortps):
                return False
            for tp, otp in zip(atps, oatps):
                if tp != otp:
                    return False
            for tp, otp in zip(rtps, ortps):
                if tp != otp:
                    return False
            return True
        if self.is_multi or other.is_multi:
            return False
        return (self.name == other.name and self.mn == other.mn)

    def __ne__(self, other):
        return not self == other

    def __hash__(self):
        return hash(str(self))

    def check(self, mod):
        if not self.is_checked:
            self._check(mod)
            self._set_is_checked()

    def _check(self, mod):
        if self.is_vec:
            self.vec_elem_tp.check(mod)
            return
        if self.is_map:
            ktp, vtp = self.map_kv_tp
            ktp.check(mod)
            if not ktp.can_be_map_key():
                ktp.t.syntax_err("类型'%s'不能作为map的key" % ktp)
            vtp.check(mod)
            return
        if self.is_func:
            atps, rtps = self.func_arg_ret_tps
            for tp in atps:
                tp.check(mod)
            for tp in rtps:
                tp.check(mod)
            return

        assert not self.is_multi

        if self.t.is_reserved:
            return

        find_path = (mod, ppgoc_mod.builtins_mod) if self.mn is None else (ppgoc_mod.mods[self.mn],)
        for m in find_path:
            coi = m.get_coi(self)
            if coi is not None:
                if m is not mod:
                    #非当前模块，检查权限
                    if not ppgoc_mod.is_public(coi):
                        if self.mn is None:
                            #引用了一个内建模块的私有成员，应continue，继续跳到for的else报找不到类型
                            continue
                        self.t.syntax_err("无法使用类型'%s'：没有权限" % self)
                self.mn = m.name
                break
        else:
            self.t.syntax_err("找不到类型'%s'" % self)

    def can_convert_from(self, tp):
        assert self.is_checked and self.is_freezed
        assert tp.is_checked and tp.is_freezed
        assert not self.is_nil

        if self.is_multi or tp.is_multi:
            return False

        if self == tp:
            #完全一样
            return True
        if self.is_any:
            #函数类型之外的其他都可以转any
            return not tp.is_func
        if self.is_obj_type:
            if tp.is_nil:
                #允许nil直接赋值给任何接口或对象类型
                return True
            #目标类型为接口或类，只有接口可以作为转换目标
            coi = self.get_coi()
            if coi.is_intf and tp.is_obj_type:
                from_coi = tp.get_coi()
                #若self是接口，则检查其他对象或接口到接口的转换
                return coi.can_convert_from(from_coi)

        #其余情况都不行
        return False

    def can_force_convert_from(self, tp):
        if self.can_convert_from(tp):
            #能隐式转换，则也能强制转换
            return True

        #number类型之间可以强转
        if self.is_number_type and tp.is_number_type:
            return True

        #其余情况不能强转
        return False

    def can_assert_type_from(self, tp):
        assert not self.is_multi
        assert not self.can_force_convert_from(tp)

        if tp.is_any:
            #any可以断言至任何下层类型
            return not self.is_func

        #tp不是any，则必须是接口到其他接口或类的断言
        if tp.is_coi_type:
            coi = tp.get_coi()
            if coi.is_intf and self.is_coi_type:
                #不能检查兼容性，因为有可能是两个无关的intf的断言转换，依赖下层的实际对象
                return True

        return False

    def get_coi(self):
        assert self.is_coi_type and self.mn is not None
        m = ppgoc_mod.mods[self.mn]
        coi = m.get_coi(self)
        assert coi is not None
        return coi

    def can_be_map_key(self):
        assert self.is_checked and self.is_freezed
        if self.is_bool_type or self.is_number_type or self.is_str_type:
            return True
        if self.is_coi_type:
            coi = self.get_coi()
            #public func cmp(other THIS_COI_TYPE) int
            if "cmp" in coi.methods:
                m = coi.methods["cmp"]
                if ppgoc_mod.is_public(m) and len(m.arg_defs) == 1 and len(m.ret_defs) == 1:
                    _, arg_tp = m.arg_defs.value_at(0)
                    #参数类型这时候可能还没check，特殊判断一下是不是当前的coi
                    if (arg_tp.t.is_name and arg_tp.name == coi.name and
                        (arg_tp.mn is None or arg_tp.mn == coi.mod.name)):
                        _, ret_tp = m.ret_defs.value_at(0)
                        if ret_tp.t.is_reserved("int"):
                            return True
        return False

for tp in BASE_TYPES + ["nil"]:
    exec '%s_TYPE = Type(ppgoc_token.make_fake_token_reserved("%s"), "%s")' % (tp.upper(), tp, tp)
    exec "%s_TYPE._set_is_checked()" % tp.upper()
del tp
STR_TYPE = STRING_TYPE

def parse_tp(tl, dep_mns):
    t = tl.pop()

    if t.is_sym("["):
        if tl.peek().is_sym("]"):
            tl.pop_sym("]")
            elem_tp = parse_tp(tl, dep_mns)
            tp = Type(t, "[]")
            tp.vec_elem_tp = elem_tp
        else:
            ktp = parse_tp(tl, dep_mns)
            tl.pop_sym("]")
            vtp = parse_tp(tl, dep_mns)
            tp = Type(t, "[map")
            tp.map_kv_tp = ktp, vtp
        tp._set_is_XXX()
        return tp

    if t.is_reserved and t.value in BASE_TYPES:
        return Type(t, t.value)

    if t.is_name:
        if t.value in dep_mns:
            tl.pop_sym(".")
            name_t, name = tl.pop_name()
            return Type(name_t, name, mn = dep_mns[t.value])
        return Type(t, t.value)

    t.syntax_err()

def from_cls(cls):
    t = ppgoc_token.make_fake_token_name(cls.name)
    tp = Type(t, cls.name, mn = cls.mod.name)
    tp._set_is_checked()
    assert tp.get_coi() is cls
    return tp

def make_multi(tps):
    assert isinstance(tps, list)
    t = ppgoc_token.make_fake_token_reserved("multi-tps")
    tp = Type(t, "multi-tps")
    tp.multi_tps = tps
    tp._set_is_XXX()
    tp._set_is_checked()
    return tp
