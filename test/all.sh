#!/bin/sh

wd=$(dirname "$0")

(
	echo "SELECT '--- Basic functions ---';";
	cat "$wd/sql/basicFunction.sql";
	echo "SELECT '';";
	echo "SELECT '--- Aggregation functions ---';"
) | mysql -N $@
