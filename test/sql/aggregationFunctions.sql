CREATE TEMPORARY TABLE `hllTest_uids_int`(uid INTEGER UNSIGNED);
INSERT INTO `hllTest_uids_int` VALUES
	(1), (2), (2), (4), (3), (4), (5), (4), (2), (5), (1)
;

SELECT '2-01. HLL_GROUP_CREATE(int)', IF(CRC32(HLL_GROUP_CREATE(16, uid)) = 3143950436, 'OK', 'FAIL') FROM `hllTest_uids_int`;
SELECT '2-02. HLL_GROUP_CREATE(int)', IF(HLL_COUNT(HLL_GROUP_CREATE(13, uid)) = 5, 'OK', 'FAIL') FROM `hllTest_uids_int`;
SELECT '2-03. HLL_GROUP_CREATE(int)', IF(HLL_COUNT(HLL_GROUP_CREATE(13, uid)) = 5, 'OK', 'FAIL') FROM `hllTest_uids_int`;
SELECT '2-04. HLL_COUNT_DISTINCT(int)', IF(HLL_COUNT_DISTINCT(uid) = 5, 'OK', 'FAIL') FROM `hllTest_uids_int`;

CREATE TEMPORARY TABLE `hllTest_uids_str`(uid VARCHAR(16));
INSERT INTO `hllTest_uids_str` VALUES
	('a'), ('b'), ('b'), ('d'), ('c'), ('d'), ('e'), ('d'), ('b'), ('e'), ('a')
;

SELECT '2-05. HLL_GROUP_CREATE(string)', IF(CRC32(HLL_GROUP_CREATE(16, uid)) = 1170795046, 'OK', 'FAIL') FROM `hllTest_uids_str`;
SELECT '2-06. HLL_GROUP_CREATE(string)', IF(HLL_COUNT(HLL_GROUP_CREATE(13, uid)) = 5, 'OK', 'FAIL') FROM `hllTest_uids_str`;
SELECT '2-07. HLL_GROUP_CREATE(string)', IF(HLL_COUNT(HLL_GROUP_CREATE(13, uid)) = 5, 'OK', 'FAIL') FROM `hllTest_uids_str`;
SELECT '2-08. HLL_COUNT_DISTINCT(string)', IF(HLL_COUNT_DISTINCT(uid) = 5, 'OK', 'FAIL') FROM `hllTest_uids_str`;
