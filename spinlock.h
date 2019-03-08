#ifndef _SPINLOCK_H_INCLUDED_
#define _SPINLOCK_H_INCLUDED_

#include "typedef.h"

void spin_lock(int *locked);
void spin_unlock(int *locked);

#endif
