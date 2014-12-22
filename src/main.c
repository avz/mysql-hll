#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include "../deps/hll/src/hll.h"

static int _add_args_to_hll(struct HLL *hll, const UDF_ARGS *args, int start) {
	char buf[64];
	size_t len;
	int i;

	for(i = start; i < args->arg_count; i++) {
		if(!args->args[i]) /* argument is NULL */
			continue;

		switch(args->arg_type[i]) {
			case DECIMAL_RESULT:
			case STRING_RESULT:
				hll_add(hll, args->args[i], args->lengths[i]);
			break;
			case INT_RESULT:
				len = (size_t)snprintf(buf, sizeof(buf), "%lld", *((long long*)args->args[i]));
				hll_add(hll, buf, len);
			break;
			case REAL_RESULT:
				len = (size_t)snprintf(buf, sizeof(buf), "%f", *((double*)args->args[i]));
				hll_add(hll, buf, len);
			break;
			default:
				return -1;
		}
	}

	return 0;
};

my_bool HLL_COUNT_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	if(args->arg_count != 1) {
		strcpy(message, "This function takes exactly 1 argument");
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;
	initid->maybe_null = 1;

	return 0;
}

double HLL_COUNT(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	struct HLL hll;
	double count = 3;

	if(!args->args[0]) {
		*is_null = 1;
		return 0;
	}

	if(hll_load(&hll, args->args[0], (size_t)args->lengths[0]) == -1) {
		*error = 1;
		return 0;
	}

	count = hll_count(&hll);

	hll_destroy(&hll);

	return count;
}

my_bool HLL_CREATE_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	if(args->arg_count < 1) {
		strcpy(message, "This function takes at least 1 argument");
		return 1;
	}

	args->arg_type[0] = INT_RESULT;
	initid->maybe_null = 1;
	initid->max_length = 1 << 24;
	initid->ptr = NULL;

	return 0;
}

void HLL_CREATE_deinit(UDF_INIT *initid) {
	if(initid->ptr) {
		hll_destroy((struct HLL*)initid->ptr);
		free(initid->ptr);
	}
}

char *HLL_CREATE(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length,
	char *is_null, char *error)
{
	long long bits;
	struct HLL *hll;

	if(!args->args[0]) {
		*error = 1;
		return NULL;
	}

	bits = *((long long *)args->args[0]);

	if(bits <= 0 || bits > 255) {
		*error = 1;
		return NULL;
	}

	hll = malloc(sizeof(*hll));

	if(hll_init(hll, (uint8_t)bits) != 0) {
		free(hll);
		*error = 1;
		return NULL;
	}

	if(_add_args_to_hll(hll, args, 1) != 0) {
		*error = 1;
		return NULL;
	}

	initid->ptr = (char *)hll;

	*length = hll->size;
	return (char *)hll->registers;
}

my_bool HLL_ADD_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	if(args->arg_count < 2) {
		strcpy(message, "This function takes at least 2 arguments");
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;
	initid->maybe_null = 1;
	initid->max_length = 1 << 24;
	initid->ptr = NULL;

	return 0;
}

void HLL_ADD_deinit(UDF_INIT *initid) {
	if(initid->ptr) {
		hll_destroy((struct HLL*)initid->ptr);
		free(initid->ptr);
	}
}

char *HLL_ADD(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length,
	char *is_null, char *error)
{
	struct HLL *hll;

	if(!args->args[0]) {
		*error = 1;
		return NULL;
	}

	hll = malloc(sizeof(*hll));

	if(hll_load(hll, args->args[0], args->lengths[0]) != 0) {
		free(hll);
		*error = 1;
		return NULL;
	}

	if(_add_args_to_hll(hll, args, 1) != 0) {
		*error = 1;
		return NULL;
	}

	initid->ptr = (char *)hll;

	*length = hll->size;
	return (char *)hll->registers;
}
