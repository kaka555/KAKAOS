#ifndef _SHELL_FUN_H
#define _SHELL_FUN_H

#include "shell_debug.h"
#include <ka_configuration.h>

#if CONFIG_SHELL_EN

//add corresponding function here
void test(int argc, char const *argv[]);

void shell_version(int argc, char const *argv[]);

#if CONFIG_TIME_EN
void shell_time(int argc, char const *argv[]);
#endif

void shell_memory(int argc, char const *argv[]);

void shell_clear(int argc, char const *argv[]);

void shell_echo(int argc, char const *argv[]);

#if CONFIG_SHELL_DEBUG_EN && CONFIG_SHELL_EN
struct set_variable{
	char *shell_v_name;
	Shell_V_Type shell_v_type;
	void *data_ptr;
};
void shell_set(int argc, char const *argv[]);
void shell_addr(int argc, char const *argv[]);
#endif

void shell_TCB_check(int argc, char const *argv[]);

void shell_reboot(int argc, char const *argv[]);

#if CONFIG_MODULE
void shell_module(int argc, char const *argv[]);
void shell_remove_module(int argc, char const *argv[]);
#endif

#if CONFIG_POWER_MANAGEMENT
void shell_sleep(int argc, char const *argv[]);
void shell_shutdown(int argc, char const *argv[]);
#endif

#if CONFIG_CPU_USE_RATE_CALCULATION
void cpu_rate(int argc, char const *argv[]);
#endif

#endif

#endif
