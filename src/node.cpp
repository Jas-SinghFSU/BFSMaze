#include "node.h"

node::node()
{
    next = nullptr;
}

node::node(int _i, int _j)
{
    i = _i;
    j = _j;
    next = nullptr;
}

node::~node()
{
    //dtor
}
