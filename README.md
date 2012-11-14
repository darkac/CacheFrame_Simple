CacheFrame_Simple
=================

A simple version of CacheFrame, using malloc/free to manipulate the memory when necessary.

Different from CacheFrame_MemoryManager, this version doesn't use a manually
written `MemoryManager` module to manage the memory operation. 

Nevertheless, it perfomrs better in the scene of list intersection when
compared to the complicated counterpart version.

Note: This module is multi-thread safe.


