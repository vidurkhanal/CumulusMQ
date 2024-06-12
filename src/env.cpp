#include <cumulusmq/conn.h>
#include <cumulusmq/env.h>

void buildEnv(Env *env) {
  const char *storage_type = getenv("CUMULUS_STORAGE_TYPE");
  env->storage_type = parseStorageType(storage_type);
}
