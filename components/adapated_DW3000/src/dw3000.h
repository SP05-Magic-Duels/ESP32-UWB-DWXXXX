#pragma once

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "dw3000_uart.h"
#include "dw3000_port.h"
#include "dw3000_device_api.h"
#include "dw3000_shared_functions.h"

#define _BV(n) (1 << n) // sets 1 at position of BIT "n"
#define __INLINE inline

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_ */
