#pragma ppgo define-THIS_MOD

namespace ppgo
{

namespace PPGO_THIS_MOD
{

}

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_BufReader>
{
    ::lom::io::BufReader::Ptr br;
};

template <>
struct NativeAttrs<PPGO_THIS_MOD::cls_BufWriter>
{
    ::lom::io::BufWriter::Ptr bw;
};

}

#pragma ppgo undef-THIS_MOD
