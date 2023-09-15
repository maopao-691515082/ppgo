#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct ClsBase<PPGO_THIS_MOD::cls_BufReader>
{
    ::lom::io::BufReader::Ptr na_br;
};

template <>
struct ClsBase<PPGO_THIS_MOD::cls_BufWriter>
{
    ::lom::io::BufWriter::Ptr na_bw;
};

}

#pragma ppgo undef-THIS_MOD
