#ifndef COROUTINE_CPP_H_
#define COROUTINE_CPP_H_

#include "coroutine/coroutine.h"

#undef ccrEndContext
#define ccrEndContext(x) } *x = reinterpret_cast<ccrContextTag *>(*ccrParam)

#undef ccrBegin
#define ccrBegin(x) if(!x) {x= reinterpret_cast<ccrContextTag *>(*ccrParam=malloc(sizeof(*x))); x->ccrLine=0;}\
                    if(x) switch(x->ccrLine) { case 0:;

#endif
