#ifndef Z40E93959D74A919952A997120A9E2296
#define Z40E93959D74A919952A997120A9E2296

#include "main.h"

/**
 *  load settings from rcfile
 *
 *  \returns 0 if everything is ok.
 */
int
load_config_file(struct opt *opt);

/**
 * save current settings to the rcfile
 *
 *  \returns 0 if everything is ok.
 */
int
save_config_file(const struct opt *opt);

#endif
