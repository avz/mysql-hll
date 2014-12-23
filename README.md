# HyperLogLog UDF for MySQL (work in progress)

### Examples

```sql
CREATE TABLE `uniqueUsersPerDay`(`day` DATE PRIMARY KEY, `hll` BLOB);
```
```sql
SET @bits = 14; -- set storage size to 2^@bits bytes (standard error 0.81%)

SET @userId = 1;
INSERT INTO `uniqueUsersPerDay`(`day`, `hll`)
	VALUES(CURDATE(), HLL_CREATE(@bits, @userId))
	ON DUPLICATE KEY UPDATE `hll` = HLL_ADD(`hll`, @userId)
;

SET @userId = 2;
INSERT INTO `uniqueUsersPerDay`(`day`, `hll`)
	VALUES(CURDATE(), HLL_CREATE(@bits, @userId))
	ON DUPLICATE KEY UPDATE `hll` = HLL_ADD(`hll`, @userId)
;

SET @userId = 2;
INSERT INTO `uniqueUsersPerDay`(`day`, `hll`)
	VALUES(CURDATE(), HLL_CREATE(@bits, @userId))
	ON DUPLICATE KEY UPDATE `hll` = HLL_ADD(`hll`, @userId)
;
```
```sql
SELECT HLL_COUNT(`hll`) AS `dau` FROM `uniqueUsersPerDay` WHERE `day` = CURDATE();
```
```
+-------------------+
| dau               |
+-------------------+
| 2.000122080247517 |
+-------------------+
1 row in set (0,00 sec)
```

### API

#### Basic functions

##### `blob HLL_CREATE(bits[, key1[, key2[, ...]]])` - implemented
##### `blob HLL_ADD(blob, key1[, key2[, ...]])` - implemented
##### `double HLL_COUNT(blob)` - implemented
##### `blob HLL_MERGE(blob1[, blob2[, ...]])` - implemented

#### Aggregation functions

##### `blob HLL_GROUP_CREATE(bits, *keys)`
##### `double HLL_GROUP_COUNT(*blobs)`
##### `blob HLL_GROUP_MERGE(*blobs)`
