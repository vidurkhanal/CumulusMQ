#ifndef ENV_H
#define ENV_H

#include "storage.h"

struct Env {
  StorageType storage_type;
};

void buildEnv(Env *env);

#endif
