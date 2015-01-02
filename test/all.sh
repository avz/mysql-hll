#!/bin/sh

wd=$(dirname "$0")

catSql() {
	echo "SELECT '--- Basic functions ---';"
	cat "$wd/sql/basicFunctions.sql"
	echo "SELECT '';"
	echo "SELECT '--- Aggregation functions ---';"
	cat "$wd/sql/aggregationFunctions.sql"
}

result=`catSql | mysql -N $@` || exit 1

echo "$result"
echo
echo '--- Result ---'

success=`echo "$result" | grep -c OK`
failed=`echo "$result" | grep -c FAIL`

if [ "$failed" -gt 0 ]; then
	echo Failed tests: $failed / $(($success + $failed))
	exit 2
else
	echo Everything is OK "($success tests)"
fi
