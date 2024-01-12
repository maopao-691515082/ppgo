#pragma once

#include "../_util.h"

namespace lom
{

namespace ordered_kv
{

namespace zkv
{

namespace experimental
{

/*
lite版本：
    - 纯内存的KV表结构，即不支持持久化
    - 数据的聚合节点使用`GoSlice<Str>`而不是`ZList`，牺牲内存来提升性能
        - 可看做是介于`AVLMap<Str, Str>`和`zkv::DB`的内存版二者中间的均衡形式
*/
class DBLite : public DBBase
{
public:

    typedef std::shared_ptr<DBLite> Ptr;

    typedef ::lom::immut::AVL<Str, ::lom::GoSlice<Str>> ZMap;

    //获取底层的数据
    virtual ZMap RawZMap() = 0;

    static LOM_ERR Open(Ptr &db);
};

}

}

}

}
