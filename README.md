# HyperLogLog UDF for MySQL (work in progress)

### API

#### Basic functions

##### `blob HLL_CREATE(bits)`
##### `blob HLL_ADD(blob, key1[, key2[, ...]])`
##### `double HLL_COUNT(blob)`
##### `blob HLL_MERGE(blob1[, blob2[, ...]])`

#### Aggregation functions

##### `double HLL_GROUP_COUNT(*blobs)`
##### `blob HLL_GROUP_MERGE(*blobs)`
