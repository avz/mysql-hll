#include <string.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include "../deps/hll/src/hll.h"

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
