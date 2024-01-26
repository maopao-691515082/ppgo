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
    - 数据的聚合节点使用`GoSlice<Str>`而不是`ZList`，
      可看做是介于`AVLMap<Str, Str>`和`zkv::DB`的内存版二者中间的均衡形式
        - 思路是在字符串比较长的时候避免`ZList`下的写拷贝和解析查找，但实际效果并不算理想，
          先放在`experimental`里
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
