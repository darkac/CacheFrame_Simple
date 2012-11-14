CacheFrame_Simple
=================

A simple version of CacheFrame, using malloc/free to manipulate the memory when necessary.

Different from `CacheFrame_MemoryManager`, this version doesn't use a manually
written `MemoryManager` module to manage the memory operation. 
However, it perfomrs better in the scene of list intersection when
compared to the complicated counterpart version.

Notes:

1) This module is multi-thread safe.

2) The `lru` and `qtca` etc. here represent the name of the caching policy.

