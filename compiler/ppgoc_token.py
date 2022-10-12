#coding=utf8

import re, math, copy
import ppgoc_util

_TOKEN_RE = re.compile(
    r"""(\d+\.?\d*[eE][+-]?\w+|"""
    r"""\.\d+[eE][+-]?\w+|"""
    r"""\d+\.\w*|"""
    r"""\.\d\w*)|"""
    r"""(0[xX][0-9A-Fa-f]+\.?[0-9A-Fa-f]*[pP][+-]?\w+|"""
    r"""0[xX]\.[0-9A-Fa-f]+[pP][+-]?\w+|"""
    r"""0[xX][0-9A-Fa-f]+\.\w*|"""
    r"""0[xX]\.[0-9A-Fa-f]\w*)|"""

    r"""(!==|===|!=|==|<<=|<<|<=|>>=|>>|>=|\.\.|[-%^&*+|/]=|&&|\|\||\+\+|--|\W)|"""

    r"""(\d\w*)|"""

    r"""([a-zA-Z_]\w*)"""
)

ASSIGN_SYM_SET = set(["=", "%=", "^=", "&=", "*=", "-=", "+=", "|=", "/=", "<<=", ">>="])
INC_DEC_SYM_SET = set(["++", "--"])
BINOCULAR_OP_SYM_SET = set([
    "%", "^", "&", "*", "-", "+", "|", "<", ">", "/", "!=", "==", "<<", "<=", ">>", ">=", "&&", "||",
])

_SYM_SET = (
    set("""~!%^&*()-+|{}[]:;"'<,>.?/""") |
    set(["!=", "==", "<<", "<=", ">>", ">=", "&&", "||", ".."]) |
    ASSIGN_SYM_SET | INC_DEC_SYM_SET
)

_RESERVED_WORD_SET = set(
    ["import", "public", "final", "class", "interface", "func", "var", "auto",
     "bool", "byte", "int", "uint", "float", "string", "any",
     "nil", "true", "false", "this",
     "for", "while", "if", "else", "return", "break", "continue", "with",
     "_"] +
    ["%s%d" % (iou, bc) for iou in "iu" for bc in (8, 16, 32, 64)] +
    ["float%d" % bc for bc in (32, 64)]
)

class _Token:
    def __init__(self, type, value, src_file, line_no, pos):
        self.id = ppgoc_util.new_id()

        self.type = type
        self.value = value
        self.src_file = src_file
        self.line_no = line_no
        self.pos = pos

        self._set_is_XXX()

        self._freeze()

    def _freeze(self):
        self.is_freezed = True

    def _unfreeze(self):
        assert self.is_freezed
        del self.__dict__["is_freezed"]

    def _set_is_XXX(self):
        class IsLiteral:
            def __init__(self, token):
                self.token = token
            def __nonzero__(self):
                return self.token.type.startswith("literal_")
            def __call__(self, type):
                assert type in ("nil", "bool", "byte", "int", "uint", "float", "str")
                return self and self.token.type == "literal_" + type
        self.is_literal = IsLiteral(self)

        class IsSym:
            def __init__(self, token):
                self.token = token
            def __nonzero__(self):
                return self.token.type == "sym"
            def __call__(self, sym):
                assert sym in _SYM_SET
                return self and self.token.value == sym
        self.is_sym = IsSym(self)

        class IsReserved:
            def __init__(self, token):
                self.token = token
            def __nonzero__(self):
                return self.token.type == "word" and self.token.value in _RESERVED_WORD_SET
            def __call__(self, word):
                assert word in _RESERVED_WORD_SET, str(word)
                return self and self.token.value == word
        self.is_reserved = IsReserved(self)
        self.is_name = self.type == "word" and self.value not in _RESERVED_WORD_SET

    def __str__(self):
        return """<token %r, %d, %d, %r>""" % (self.src_file, self.line_no, self.pos + 1, self.value)
    __repr__ = __str__

    def __setattr__(self, name, value):
        if self.__dict__.get("is_freezed", False):
            ppgoc_util.raise_bug()
        self.__dict__[name] = value

    def __delattr__(self, name):
        if self.__dict__.get("is_freezed", False):
            ppgoc_util.raise_bug()
        del self.__dict__[name]

    def copy_on_pos(self, t):
        return _Token(self.type, self.value, t.src_file, t.line_no, t.pos)

    def copy(self):
        return self.copy_on_pos(self)

    def syntax_err(self, msg = ""):
        ppgoc_util.exit("%s %s" % (self.pos_desc(), msg))

    def warning(self, msg):
        ppgoc_util.warning("%s %s" % (self.pos_desc(), msg))

    def pos_desc(self):
        return "文件[%s]行[%d]列[%d]" % (self.src_file, self.line_no, self.pos + 1)

class TokenList:
    def __init__(self, src_file):
        self.src_file = src_file
        self.l = []
        self.i = 0

    def __nonzero__(self):
        return self.i < len(self.l)

    def __iter__(self):
        for i in xrange(self.i, len(self.l)):
            yield self.l[i]

    def copy(self):
        return copy.deepcopy(self)

    def peek(self, start_idx = 0):
        try:
            return self.l[self.i + start_idx]
        except IndexError:
            ppgoc_util.exit("文件[%s]代码意外结束" % self.src_file)

    def peek_name(self):
        t = self.peek()
        if not t.is_name:
            t.syntax_err("需要标识符")
        return t.value

    def revert(self, i = None):
        if i is None:
            assert self.i > 0
            self.i -= 1
        else:
            assert 0 <= i <= self.i < len(self.l)
            self.i = i

    def pop(self):
        t = self.peek()
        self.i += 1
        return t

    def pop_sym(self, sym = None):
        t = self.pop()
        if not t.is_sym:
            if sym is None:
                t.syntax_err("需要符号")
            else:
                t.syntax_err("需要符号'%s'" % sym)
        if sym is None:
            return t, t.value
        if t.value != sym:
            t.syntax_err("需要'%s'" % sym)
        return t

    def pop_name(self):
        t = self.pop()
        if not t.is_name:
            t.syntax_err("需要标识符")
        return t, t.value

    def append(self, t):
        assert isinstance(t, _Token)
        self.l.append(t)

def _syntax_err(src_file, line_no, pos, msg):
    ppgoc_util.exit("文件[%s]行[%d]列[%d] %s" % (src_file, line_no, pos + 1, msg))

def _syntax_warning(src_file, line_no, pos, msg):
    ppgoc_util.warning("文件[%s]行[%d]列[%d] %s" % (src_file, line_no, pos + 1, msg))

def _get_escape_char(s, src_file, line_no, pos):
    if s[0] in "abfnrtv":
        return eval("'\\" + s[0] + "'"), s[1 :]

    if s[0] in ("\\", "'", '"'):
        return s[0], s[1 :]

    if s[0] >= "0" and s[0] <= "7":
        for k in s[: 3], s[: 2], s[0]:
            try:
                i = int(k, 8)
                break
            except ValueError:
                pass
        if i > 255:
            _syntax_err(src_file, line_no, pos, "八进制换码序列值过大[\\%s]" % k)
        return chr(i), s[len(k) :]

    if s[0] == "x":
        if len(s) < 3:
            _syntax_err(src_file, line_no, pos, "十六进制换码序列长度不够")
        try:
            i = int(s[1 : 3], 16)
        except ValueError:
            _syntax_err(src_file, line_no, pos, "十六进制换码序列值错误[\\%s]" % s[: 3])
        return chr(i), s[3 :]

    _syntax_err(src_file, line_no, pos, "非法的转义字符[%s]" % s[0])

def _parse_str(s, src_file, line_no, pos):
    s_len = len(s)
    quota = s[0]
    s = s[1 :]

    l = []

    while s:
        c = s[0]
        s = s[1 :]
        if c == quota:
            break
        if c == "\\":
            if s == "":
                _syntax_err(src_file, line_no, pos, "字符串在转义处结束")
            c, s = _get_escape_char(s, src_file, line_no, pos)
        l.append(c)
    else:
        _syntax_err(src_file, line_no, pos, "字符串不完整")

    return "".join(l), s_len - len(s)

def _parse_token(mod_name, src_file, line_no, line, pos):
    s = line[pos :]
    m = _TOKEN_RE.match(s)
    if m is None:
        _syntax_err(src_file, line_no, pos, "")

    f, hex_f, sym, i, w = m.groups()

    if f is not None or hex_f is not None:
        if f is None:
            f = hex_f
        try:
            value = float(f) if hex_f is None else float.fromhex(f)
            if math.isnan(value) or math.isinf(value):
                raise ValueError
        except ValueError:
            _syntax_err(src_file, line_no, pos, "非法的float字面量'%s'" % f)
        return _Token("literal_float", value, src_file, line_no, pos), len(f)

    if sym is not None:
        if sym in ("'", '"'):
            value, token_len = _parse_str(s, src_file, line_no, pos)
            if sym == '"':
                return _Token("literal_str", value, src_file, line_no, pos), token_len
            if len(value) != 1:
                _syntax_err(src_file, line_no, pos, "byte字面量长度必须为1")
            return _Token("literal_byte", ord(value), src_file, line_no, pos), token_len

        if sym not in _SYM_SET:
            _syntax_err(src_file, line_no, pos, "非法的符号'%r'" % sym)
        return _Token("sym", sym, src_file, line_no, pos), len(sym)

    if i is not None:
        try:
            if i[-1].upper() == "U":
                value = int(i[: -1], 0)
                if value >= 2 ** 64:
                    _syntax_err(src_file, line_no, pos, "过大的uint字面量'%s'" % i)
                type = "uint"
            else:
                value = int(i, 0)
                if value >= 2 ** 63:
                    _syntax_err(src_file, line_no, pos, "过大的int字面量'%s'" % i)
                type = "int"
        except ValueError:
            _syntax_err(src_file, line_no, pos, "非法的整数字面量'%s'" % i)
        return _Token("literal_" + type, value, src_file, line_no, pos), len(i)

    if w is not None:
        if w in ("true", "false"):
            return _Token("literal_bool", w, src_file, line_no, pos), len(w)
        if w == "nil":
            return _Token("literal_nil", w, src_file, line_no, pos), len(w)
        return _Token("word", w, src_file, line_no, pos), len(w)

    ppgoc_util.raise_bug()

class _RawStr:
    def __init__(self, value, src_file, line_no, pos):
        self.value = value
        self.src_file = src_file
        self.line_no = line_no
        self.pos = pos

    def check(self):
        if "\t\n" in self.value or "\x20\n" in self.value:
            _syntax_warning(self.src_file, self.line_no, self.pos, "原始字符串含有空格或制表符结尾的行")

def _parse_line(mod_name, src_file, line_no, line, pos):
    token_list = []
    uncompleted_comment_start_pos = None
    raw_str = None

    while pos < len(line):
        while pos < len(line) and line[pos] in "\t\x20":
            pos += 1
        if pos >= len(line):
            break

        if line[pos : pos + 2] == "//":
            break
        if line[pos : pos + 2] == "/*":
            pos += 2
            comment_end_pos = line[pos :].find("*/")
            if comment_end_pos < 0:
                uncompleted_comment_start_pos = pos - 2
                break
            pos += comment_end_pos + 2
            continue
        if line[pos] == "`":
            raw_str = _RawStr("", src_file, line_no, pos)
            pos += 1
            raw_str_end_pos = line[pos :].find("`")
            if raw_str_end_pos < 0:
                raw_str.value += line[pos :] + "\n"
                break
            raw_str.value += line[pos : pos + raw_str_end_pos]
            raw_str.check()
            token_list.append(_Token("literal_str", raw_str.value, raw_str.src_file, raw_str.line_no, raw_str.pos))
            pos += raw_str_end_pos + 1
            raw_str = None
            continue

        token, token_len = _parse_token(mod_name, src_file, line_no, line, pos)
        token_list.append(token)
        pos += token_len

    return token_list, uncompleted_comment_start_pos, raw_str

def parse_token_list(mod_name, src_file):
    lines = ppgoc_util.open_src_file(src_file).read().splitlines()

    token_list = TokenList(src_file)
    in_comment = False
    raw_str = None
    for line_no, line in enumerate(lines):
        line_no += 1

        for pos, c in enumerate(line):
            assert c not in ("\r", "\n")
            if ord(c) < 32 and c not in ("\t",):
                _syntax_err(src_file, line_no, pos, "含有非法的ascii控制码‘%r’" % c)

        if in_comment:
            pos = line.find("*/")
            if pos < 0:
                continue
            pos += 2
            in_comment = False
        elif raw_str is not None:
            pos = line.find("`")
            if pos < 0:
                raw_str.value += line + "\n"
                continue
            raw_str.value += line[: pos]
            raw_str.check()
            token_list.append(_Token("literal_str", raw_str.value, raw_str.src_file, raw_str.line_no, raw_str.pos))
            pos += 1
            raw_str = None
        else:
            pos = 0

        line_tl, uncompleted_comment_start_pos, raw_str = _parse_line(mod_name, src_file, line_no, line, pos)
        for t in line_tl:
            token_list.append(t)
        if uncompleted_comment_start_pos is not None:
            assert raw_str is None
            in_comment = True

    if in_comment:
        _syntax_err(src_file, len(lines), len(lines[-1]), "存在未结束的块注释")
    if raw_str is not None:
        _syntax_err(src_file, len(lines), len(lines[-1]), "存在未结束的原始字符串")

    return token_list

def parse_tl_until_sym(tl, end_syms):
    brackets = {"(" : ")", "[" : "]", "{" : "}"}
    sub_tl = TokenList(tl.src_file)
    stk = []
    while True:
        t = tl.pop()
        sub_tl.append(t)
        if t.is_sym and t.value in end_syms and not stk:
            return sub_tl, t.value
        if t.is_sym and t.value in brackets:
            stk.append(t)
        if t.is_sym and t.value in brackets.values():
            if not (stk and stk[-1].is_sym and t.value == brackets.get(stk[-1].value)):
                t.syntax_err("未匹配的'%s'" % t.value)
            stk.pop()

def gen_empty_token_list(end_sym):
    token_list = TokenList("EMPTY")
    token_list.append(_Token("sym", end_sym, "EMPTY", -1, -1))
    return token_list

def is_valid_name(name):
    return re.match("^[a-zA-Z_]\w*$", name) is not None and name not in _RESERVED_WORD_SET

def make_fake_token_reserved(w):
    t = _Token("word", w, "<nil>", 0, -1)
    t._unfreeze()
    t.is_reserved = lambda r: r == w
    t.is_name = False
    t._freeze()
    return t

def make_fake_token_name(w):
    assert w not in _RESERVED_WORD_SET
    t = _Token("word", w, "<nil>", 0, -1)
    return t
