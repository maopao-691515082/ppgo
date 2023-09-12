#coding=utf8

import ppgoc_expr, ppgoc_token, ppgoc_util, ppgoc_type

class Stmt:
    def __init__(self, type, **kw_arg):
        self.type = type
        for k, v in kw_arg.iteritems():
            setattr(self, k, v)

class StmtList(list):
    def __init__(self, vars):
        list.__init__(self)
        self.vars = vars

'''
独立记录已定义但仍在解析其初始化表达式的变量名栈，用于变量名冲突检测
基本都是闭包的检测问题
例：
    ```
    var x = func () int {
        var x = 123; //需要检测出这里的x和上面重名
        return x;
    }();
    ```
ieup = init-expr unparsed
元素为名字列表
'''
ieup_var_names_stk = []

class Parser:
    def __init__(self, tl, mod, dep_mns, cls, fom):
        self.tl = tl
        self.mod = mod
        self.dep_mns = dep_mns
        self.cls = cls
        self.fom = fom

        self.fn = self.fom.fn
        self.expr_parser = ppgoc_expr.Parser(tl, mod, self.fn, dep_mns, cls, fom)

    def parse(self, vars_stk, loop_deep):
        assert vars_stk
        stmts = StmtList(vars_stk[-1])

        while True:
            if self.tl.peek().is_sym("}"):
                break

            t = self.tl.pop()

            if t.is_sym(";"):
                t.warning("空语句")
                continue

            if t.is_reserved("for"):
                self.tl.pop_sym("(")
                t = self.tl.pop()
                if not t.is_reserved("var"):
                    t.syntax_err("for语句必须用'var'独立定义变量")
                var_t = t
                names = []
                while True:
                    if self.tl.peek().is_reserved("_"):
                        names.append((self.tl.pop(), "_.%d" % ppgoc_util.new_id()))
                    else:
                        names.append(self.tl.pop_name())
                    if not self.tl.peek().is_sym(","):
                        break
                    self.tl.pop_sym(",")
                new_vars = ppgoc_util.OrderedDict()
                for name_t, name in names:
                    for vars in vars_stk:
                        if name in vars:
                            name_t.syntax_err("与上层变量名冲突")
                    for var_names in ieup_var_names_stk:
                        if name in var_names:
                            name_t.syntax_err("与上层变量名冲突")
                    new_vars[name] = None
                assert new_vars
                self.tl.pop_sym(":")
                t = self.tl.peek()
                ieup_var_names_stk.append(list(new_vars))
                e = self.expr_parser.parse(vars_stk, None)
                ieup_var_names_stk.pop()
                is_range = self.tl.peek().is_sym("..")
                if is_range:
                    self.tl.pop_sym("..")
                    ea = e
                    e = None
                    ieup_var_names_stk.append(list(new_vars))
                    eb = self.expr_parser.parse(vars_stk, None)
                    ieup_var_names_stk.pop()
                    if ea.tp != eb.tp:
                        t.syntax_err("'..'两侧的表达式类型必须相同")
                    if not ea.tp.is_integer_type:
                        t.syntax_err("'..'两侧的表达式必须是整数类型")
                    if len(names) != 1:
                        var_t.syntax_err("循环变量数量错误")
                    new_vars[new_vars.key_at(0)] = ea.tp
                elif e.tp.is_vec or e.tp.is_vec_view:
                    if len(names) != 2:
                        var_t.syntax_err("循环变量数量错误")
                    new_vars[new_vars.key_at(0)] = ppgoc_type.INT_TYPE
                    new_vars[new_vars.key_at(1)] = e.tp.vec_elem_tp or e.tp.vec_view_elem_tp
                elif e.tp.is_map:
                    ktp, vtp = e.tp.map_kv_tp
                    if len(names) != 2:
                        var_t.syntax_err("循环变量数量错误")
                    new_vars[new_vars.key_at(0)] = ktp
                    new_vars[new_vars.key_at(1)] = vtp
                elif e.tp.is_set:
                    if len(names) != 1:
                        var_t.syntax_err("循环变量数量错误")
                    new_vars[new_vars.key_at(0)] = e.tp.set_elem_tp
                else:
                    t.syntax_err("类型'%s'不可用于遍历" % e.tp)
                self.tl.pop_sym(")")
                self.tl.pop_sym("{")
                valid_new_vars = ppgoc_util.OrderedDict()
                for name, tp in new_vars.iteritems():
                    if not name.startswith("_."):
                        valid_new_vars[name] = tp
                for_stmts = self.parse(vars_stk + (valid_new_vars,), loop_deep + 1)
                self.tl.pop_sym("}")
                if is_range:
                    stmts.append(Stmt(
                        "for..", var_name = new_vars.key_at(0), ep = (ea, eb), stmts = for_stmts))
                elif e.tp.is_vec  or e.tp.is_vec_view:
                    stmts.append(Stmt(
                        "for_vec", idx_var_name = new_vars.key_at(0), value_var_name = new_vars.key_at(1),
                        expr = e, stmts = for_stmts))
                elif e.tp.is_map:
                    stmts.append(Stmt(
                        "for_map", k_var_name = new_vars.key_at(0), v_var_name = new_vars.key_at(1),
                        expr = e, stmts = for_stmts))
                elif e.tp.is_set:
                    stmts.append(Stmt(
                        "for_set", var_name = new_vars.key_at(0), expr = e, stmts = for_stmts))
                else:
                    ppgoc_util.raise_bug()
                continue

            if t.is_reserved("var"):
                is_batch_def = self.tl.peek().is_sym("(")
                if is_batch_def:
                    self.tl.pop_sym("(")
                while True:
                    names = []
                    while True:
                        if self.tl.peek().is_reserved("_"):
                            names.append((self.tl.pop(), "_.%d" % ppgoc_util.new_id()))
                        else:
                            names.append(self.tl.pop_name())
                        if not self.tl.peek().is_sym(","):
                            break
                        self.tl.pop_sym(",")
                    assert names
                    new_vars = ppgoc_util.OrderedDict()
                    for name_t, name in names:
                        if name in vars_stk[-1]:
                            name_t.syntax_err("变量名重复定义")
                        for vars in vars_stk:
                            if name in vars:
                                name_t.syntax_err("与上层变量名冲突")
                        for var_names in ieup_var_names_stk:
                            if name in var_names:
                                name_t.syntax_err("与上层变量名冲突")
                        new_vars[name] = None
                    assert new_vars

                    if self.tl.peek().is_sym("="):
                        self.tl.pop_sym("=")
                        ieup_var_names_stk.append(list(new_vars))
                        e = self.expr_parser.parse(vars_stk, None)
                        ieup_var_names_stk.pop()
                        if e.tp.is_multi:
                            tps = e.tp.multi_tps
                        else:
                            tps = [e.tp]
                        if len(new_vars) < len(tps) or len(new_vars) > len(tps) + 2:
                            names[0][0].syntax_err("左值数量错误")
                        for i, tp in enumerate(tps):
                            new_vars[new_vars.key_at(i)] = tp
                        if len(new_vars) > len(tps):
                            #exception
                            new_vars[new_vars.key_at(len(tps))] = ppgoc_type.ANY_TYPE
                        if len(new_vars) > len(tps) + 1:
                            #traceback
                            new_vars[new_vars.key_at(len(tps) + 1)] = ppgoc_type.STR_TYPE
                    else:
                        tp = ppgoc_type.parse_tp(self.tl, self.dep_mns)
                        tp.check(self.mod)
                        for name in list(new_vars):
                            new_vars[name] = tp
                        e = None #init after type is not supported now
                    self.tl.pop_sym(";")

                    new_curr_vars = vars_stk[-1].copy()
                    for name, tp in new_vars.iteritems():
                        if not name.startswith("_."):
                            assert name not in new_curr_vars and tp is not None
                            new_curr_vars[name] = tp
                            stmts.append(Stmt("var", name = name, tp = tp))
                    vars_stk = vars_stk[: -1] + (new_curr_vars,)

                    if e is not None:
                        if (len(new_vars) == 1 and not new_vars.key_at(0).startswith("_.") and
                            not e.tp.is_multi):
                            stmt = stmts.pop()
                            assert stmt.name == new_vars.key_at(0) and stmt.tp == new_vars.value_at(0)
                            stmts.append(Stmt("var_with_init", name = stmt.name, tp = stmt.tp, expr = e))
                        else:
                            stmts.append(Stmt("var_init", new_vars = new_vars, expr = e))

                    if not is_batch_def:
                        break
                    if self.tl.peek().is_sym(")"):
                        self.tl.pop_sym(")")
                        break

                continue

            if t.is_reserved("return"):
                if len(self.fom.ret_defs) == 1 and self.fom.ret_defs.key_at(0) is None:
                    _, ret_tp = self.fom.ret_defs[None]
                    e = self.expr_parser.parse(vars_stk, ret_tp)
                else:
                    e = None
                self.tl.pop_sym(";")
                stmts.append(Stmt("return", expr = e))
                continue

            if t.is_reserved("while"):
                self.tl.pop_sym("(")
                e = self.expr_parser.parse(vars_stk, ppgoc_type.BOOL_TYPE)
                self.tl.pop_sym(")")
                self.tl.pop_sym("{")
                while_stmts = self.parse(vars_stk + (ppgoc_util.OrderedDict(),), loop_deep + 1)
                self.tl.pop_sym("}")
                stmts.append(Stmt("while", expr = e, stmts = while_stmts))
                continue

            if t.is_reserved("if"):
                if_expr_stmts_list = []
                else_stmts = None
                while True:
                    self.tl.pop_sym("(")
                    e = self.expr_parser.parse(vars_stk, ppgoc_type.BOOL_TYPE)
                    self.tl.pop_sym(")")
                    self.tl.pop_sym("{")
                    if_stmts = self.parse(vars_stk + (ppgoc_util.OrderedDict(),), loop_deep)
                    self.tl.pop_sym("}")
                    if_expr_stmts_list.append((e, if_stmts))
                    if not self.tl.peek().is_reserved("else"):
                        break
                    self.tl.pop()
                    if self.tl.peek().is_sym("{"):
                        self.tl.pop_sym("{")
                        else_stmts = self.parse(vars_stk + (ppgoc_util.OrderedDict(),), loop_deep)
                        self.tl.pop_sym("}")
                        break
                    if not self.tl.peek().is_reserved("if"):
                        self.tl.peek().syntax_err("需要'if'或'{'")
                    self.tl.pop()
                stmts.append(Stmt("if", if_expr_stmts_list = if_expr_stmts_list, else_stmts = else_stmts))
                continue

            if t.is_sym and t.value in ("++", "--"):
                lvalue = self.expr_parser.parse(vars_stk, None)
                if not (lvalue.is_lvalue and lvalue.tp.is_integer_type):
                    t.syntax_err("需要整数类型的左值表达式")
                self.tl.pop_sym(";")
                stmts.append(Stmt(t.value, lvalue = lvalue))
                continue

            if t.is_reserved and t.value in ("break", "continue"):
                if loop_deep == 0:
                    t.syntax_err("循环外的%s" % t.value)
                stmts.append(Stmt(t.value))
                self.tl.pop_sym(";")
                continue

            if t.is_reserved("with"):
                self.tl.pop_sym("(")
                e = self.expr_parser.parse(vars_stk, ppgoc_type.WITHABLE_TYPE)
                self.tl.pop_sym(")")
                self.tl.pop_sym("{")
                with_stmts = self.parse(vars_stk + (ppgoc_util.OrderedDict(),), loop_deep)
                self.tl.pop_sym("}")
                stmts.append(Stmt("with", expr = e, stmts = with_stmts))
                continue

            self.tl.revert()

            et = self.tl.peek()
            if self.tl.peek().is_reserved("_"):
                expr = None
                self.tl.pop()
            else:
                expr = self.expr_parser.parse(vars_stk, None)
            t, sym = self.tl.pop_sym()
            if expr is not None:
                if sym == ";":
                    #expr as stmt
                    stmts.append(Stmt("expr", expr = expr))
                    continue
                if sym in ppgoc_token.ASSIGN_SYM_SET and sym != "=":
                    #expr op= expr
                    lvalue = expr
                    assert sym.endswith("=")
                    op = sym[: -1]
                    if op == "+":
                        if not (lvalue.tp.is_str_type or lvalue.tp.is_number_type):
                            first_et.syntax_err("需要字符串或数值类型")
                    elif op in ("-", "*", "/", "%"):
                        if not lvalue.tp.is_number_type:
                            first_et.syntax_err("需要数值类型")
                    elif op in ("&", "|", "^", "<<", ">>"):
                        if not lvalue.tp.is_integer_type:
                            first_et.syntax_err("需要整数类型")
                    else:
                        ppgoc_util.raise_bug()
                    op_is_shift = op in ("<<", ">>")
                    t = self.tl.peek()
                    expr = self.expr_parser.parse(vars_stk, None if op_is_shift else lvalue.tp)
                    self.tl.pop_sym(";")
                    if op_is_shift:
                        if not expr.tp.is_integer_type:
                            t.syntax_err("移位运算右分量需要是整数类型")
                    stmts.append(Stmt("op=", op = op, lvalue = lvalue, expr = expr))
                    continue

            #assign
            first_et = et
            lvalues = []
            while True:
                if expr is not None and not expr.is_lvalue:
                    et.syntax_err("需要左值")
                if expr.tp.is_func:
                    et.syntax_err("不能赋值给闭包函数类型的变量")
                lvalues.append(expr)
                if sym != ",":
                    break
                et = self.tl.peek()
                if self.tl.peek().is_reserved("_"):
                    expr = None
                    self.tl.pop()
                else:
                    expr = self.expr_parser.parse(vars_stk, None)
                t, sym = self.tl.pop_sym()
            assert lvalues
            if sym != "=":
                t.syntax_err()

            expr = self.expr_parser.parse(vars_stk, None)
            self.tl.pop_sym(";")
            if expr.tp.is_multi:
                tps = expr.tp.multi_tps
            else:
                tps = [expr.tp]
            if len(lvalues) < len(tps) or len(lvalues) > len(tps) + 2:
                first_et.syntax_err("左值数量错误")
            for i, tp in enumerate(tps):
                lvalue = lvalues[i]
                if lvalue is not None and not lvalue.tp.can_convert_from(tp):
                    t, _ = lvalue.pos_info
                    t.syntax_err("类型'%s'无法隐式转换为类型'%s'" % (tp, lvalue.tp))
            if len(lvalues) > len(tps):
                lvalue = lvalues[len(tps)]
                if lvalue is not None and lvalue.tp != ppgoc_type.ANY_TYPE:
                    t, _ = lvalue.pos_info
                    t.syntax_err("异常需要类型'any'")
            if len(lvalues) > len(tps) + 1:
                lvalue = lvalues[len(tps) + 1]
                if lvalue is not None and lvalue.tp != ppgoc_type.STR_TYPE:
                    t, _ = lvalue.pos_info
                    t.syntax_err("Traceback信息需要类型'string'")
            stmts.append(Stmt("=", lvalues = lvalues, expr = expr))

        return stmts
