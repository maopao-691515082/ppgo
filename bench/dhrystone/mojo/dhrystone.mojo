import time
from memory.unsafe import Pointer
from utils.vector import DynamicVector

struct Record:

    var PtrComp : Int
    var Discr : Int
    var EnumComp : Int
    var IntComp : Int
    var StringComp : String

    fn __init__(inout self):
        self.PtrComp = 0
        self.Discr = 0
        self.EnumComp = 0
        self.IntComp = 0
        self.StringComp = ""
    fn __copyinit__(inout self, other : Self):
        self.PtrComp = other.PtrComp
        self.Discr = other.Discr
        self.EnumComp = other.EnumComp
        self.IntComp = other.IntComp
        self.StringComp = other.StringComp

var LOOPS = 50000000

var Ident1 = 1
var Ident2 = 2
var Ident3 = 3
var Ident4 = 4
var Ident5 = 5

var IntGlob = 0
var BoolGlob = False
var Char1Glob = 0
var Char2Glob = 0
var Array1Glob = DynamicVector[Int]()
var Array2Glob = DynamicVector[Int]()

var PtrGlb = Record()
var PtrGlbNext = Record()

fn Func3(EnumParIn : Int) -> Bool:
    let EnumLoc = EnumParIn
    if EnumLoc == Ident3:
        return True
    return False

fn Func2(StrParI1 : String, StrParI2 : String) -> Bool:
    var IntLoc = 1
    var CharLoc = 0
    while IntLoc <= 1:
        if Func1(ord(StrParI1[IntLoc]), ord(StrParI2[IntLoc + 1])) == Ident1:
            CharLoc = 65
            IntLoc += 1
    if CharLoc >= 87 and CharLoc <= 90:
        IntLoc = 7
    if CharLoc == 88:
        return True
    else:
        #if StrParI1 > StrParI2:
        if StrParI1 != StrParI2:
            IntLoc += 7
            return True
        else:
            return False

fn Func1(CharPar1 : Int, CharPar2 : Int) -> Int:
    let CharLoc1 = CharPar1
    let CharLoc2 = CharLoc1
    if CharLoc2 != CharPar2:
        return Ident1
    else:
        return Ident2

fn Proc8(inout Array1Par : DynamicVector[Int], inout Array2Par : DynamicVector[Int], IntParI1 : Int, IntParI2 : Int):
    let IntLoc = IntParI1 + 5
    Array1Par[IntLoc] = IntParI2
    Array1Par[IntLoc + 1] = Array1Par[IntLoc]
    Array1Par[IntLoc + 30] = IntLoc
    for IntIndex in range(IntLoc, IntLoc + 2):
        Array2Par[IntLoc * 51 + IntIndex] = IntLoc
    Array2Par[IntLoc * 51 + IntLoc - 1] += 1
    Array2Par[(IntLoc + 20) * 51 + IntLoc] = Array1Par[IntLoc]
    IntGlob = 5

fn Proc7(IntParI1 : Int, IntParI2 : Int) -> Int:
    let IntLoc = IntParI1 + 2
    let IntParOut = IntParI2 + IntLoc
    return IntParOut

fn Proc6(EnumParIn : Int) -> Int:
    var EnumParOut = EnumParIn
    if not Func3(EnumParIn):
        EnumParOut = Ident4
    if not (EnumParIn == Ident1):
        EnumParOut = Ident1
    elif EnumParIn == Ident2:
        if IntGlob > 100:
            EnumParOut = Ident1
        else:
            EnumParOut = Ident4
    elif EnumParIn == Ident3:
        EnumParOut = Ident2
    elif EnumParIn == Ident4:
        pass
    elif EnumParIn == Ident5:
        EnumParOut = Ident3
    return EnumParOut

fn Proc5():
    Char1Glob = 65
    BoolGlob = False

fn Proc4():
    var BoolLoc = Char1Glob == 65
    BoolLoc = BoolLoc or BoolGlob
    Char2Glob = 66

fn Proc3(inout PtrParOut : Int) -> Int:
    if Pointer[Record].address_of(PtrGlb) != Pointer[Record].get_null():
        PtrParOut = PtrGlb.PtrComp
    else:
        IntGlob = 100
    PtrGlb.IntComp = Proc7(10, IntGlob)
    return PtrParOut

fn Proc2(inout IntParIO : Int) -> Int:
    var IntLoc = IntParIO + 10
    var EnumLoc = 0
    while True:
        if Char1Glob == 65:
            IntLoc -= 1
            IntParIO = IntLoc - IntGlob
            EnumLoc = Ident1
        if EnumLoc == Ident1:
            break
    return IntParIO

fn Proc1(inout PtrParIn : Record, inout NextRecord : Record):
    NextRecord = PtrGlb
    PtrParIn.IntComp = 5
    NextRecord.IntComp = PtrParIn.IntComp
    NextRecord.PtrComp = PtrParIn.PtrComp
    NextRecord.PtrComp = Proc3(NextRecord.PtrComp)
    if NextRecord.Discr == Ident1:
        NextRecord.IntComp = 6
        NextRecord.EnumComp = Proc6(PtrParIn.EnumComp)
        NextRecord.PtrComp = PtrGlb.PtrComp
        NextRecord.IntComp = Proc7(NextRecord.IntComp, 10)
    else:
        PtrParIn = NextRecord

fn Proc0():
    PtrGlb.PtrComp = 1
    PtrGlb.Discr = Ident1
    PtrGlb.EnumComp = Ident3
    PtrGlb.IntComp = 40
    PtrGlb.StringComp = "DHRYSTONE PROGRAM, SOME STRING"
    let String1Loc = "DHRYSTONE PROGRAM, 1'ST STRING"
    Array2Glob[8 * 51 + 7] = 10

    for i in range(LOOPS):
        Proc5()
        Proc4()
        var IntLoc1 = 2
        var IntLoc2 = 3
        let String2Loc = "DHRYSTONE PROGRAM, 2'ND STRING"
        var EnumLoc = Ident2
        BoolGlob = not Func2(String1Loc, String2Loc)
        var IntLoc3 = 0
        #while IntLoc1 < IntLoc2:
        #    IntLoc3 = 5 * IntLoc1 - IntLoc2
        #    IntLoc3 = Proc7(IntLoc1, IntLoc2)
        #    IntLoc1 += 1
        IntLoc3 = 5 * IntLoc1 - IntLoc2
        IntLoc3 = Proc7(IntLoc1, IntLoc2)
        IntLoc1 += 1
        Proc8(Array1Glob, Array2Glob, IntLoc1, IntLoc3)
        #PtrGlb = Proc1(PtrGlb)
        if PtrGlb.PtrComp == 0:
            Proc1(PtrGlb, PtrGlb)
        else:
            Proc1(PtrGlb, PtrGlbNext)
        var CharIndex = 65
        while CharIndex <= Char2Glob:
            if EnumLoc == Func1(CharIndex, 67):
                EnumLoc = Proc6(Ident1)
            CharIndex += 1
        IntLoc3 = IntLoc2 * IntLoc1
        IntLoc2 = (IntLoc3 / IntLoc1).to_int()
        IntLoc2 = 7 * (IntLoc3 - IntLoc2) - IntLoc1
        IntLoc1 = Proc2(IntLoc1)

fn main():
    Array1Glob.resize(51)
    Array2Glob.resize(51 * 51)
    let ts = time.now()
    Proc0()
    let tm = (time.now() - ts) / 1e9
    print("Time used:", tm, "sec")
    print("This machine benchmarks at", LOOPS / tm, "MojoStones/second")
