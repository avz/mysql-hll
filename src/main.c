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

static int _merge_arg_to_hll(struct HLL *hll, enum Item_result type, const char *buf, size_t size) {
	struct HLL tmp;

	if(!buf) /* NULL arg */
		return 0;

	if(type != STRING_RESULT) {
		return -1;
	}

	if(hll_load(&tmp, buf, size) != 0) {
		return -1;
	}

	if(hll_merge(hll, &tmp) != 0) {
		hll_destroy(&tmp);
		return -1;
	}

	hll_destroy(&tmp);

	return 0;
}

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
		hll_destroy(hll);
		free(hll);
		*error = 1;
		return NULL;
	}

	initid->ptr = (char *)hll;

	*length = hll->size;
	return (char *)hll->registers;
}

my_bool HLL_MERGE_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	if(args->arg_count < 2) {
		strcpy(message, "This function takes at least 2 arguments");
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;
	args->arg_type[1] = STRING_RESULT;
	initid->maybe_null = 1;
	initid->max_length = 1 << 24;
	initid->ptr = NULL;

	return 0;
}

void HLL_MERGE_deinit(UDF_INIT *initid) {
	if(initid->ptr) {
		hll_destroy((struct HLL*)initid->ptr);
		free(initid->ptr);
	}
}

char *HLL_MERGE(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length,
	char *is_null, char *error)
{
	struct HLL *hll;
	int i;

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

	for(i = 0; i < args->arg_count; i++) {
		if(_merge_arg_to_hll(hll, args->arg_type[i], args->args[i], args->lengths[i]) != 0) {
			hll_destroy(hll);
			free(hll);

			*error = 1;
			return NULL;
		}
	}

	initid->ptr = (char *)hll;

	*length = hll->size;
	return (char *)hll->registers;
}

my_bool HLL_GROUP_COUNT_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
	if(args->arg_count != 1) {
		strcpy(message, "This function takes exactly 1 argument");
		return 1;
	}

	args->arg_type[0] = STRING_RESULT;
	initid->maybe_null = 1;
	initid->ptr = NULL;

	return 0;
}

void HLL_GROUP_COUNT_deinit(UDF_INIT *initid) {
	if(initid->ptr) {
		hll_destroy((struct HLL*)initid->ptr);
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

void HLL_GROUP_COUNT_clear(UDF_INIT *initid, char *is_null, char *error) {
	HLL_GROUP_COUNT_deinit(initid);
}

void HLL_GROUP_COUNT_add(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
	struct HLL *hll;

	if(!args->args[0])
		return;

	if(initid->ptr) {
		hll = (struct HLL *)initid->ptr;

		if(_merge_arg_to_hll(hll, args->arg_type[0], args->args[0], args->lengths[0]) != 0) {
			HLL_GROUP_COUNT_deinit(initid);

			*error = 1;
			return;
		}
	} else {
		hll = malloc(sizeof(*hll));

		if(hll_load(hll, args->args[0], args->lengths[0]) != 0) {
			free(hll);
			*error = 1;
			return;
		}

		initid->ptr = (char *)hll;
		return;
	}
}

double HLL_GROUP_COUNT(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	if(!initid->ptr) {
		*is_null = 1;
		return 0;
	}

	return hll_count((struct HLL *)initid->ptr);
}