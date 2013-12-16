ebgn
====
BGN is a general distributed compulation platform in C language.

Based on distributed module technology, BGN provides elastic module slots, e.g., Distributed File System module(HsDFS),
NoSQL BigTable(HsBGT), Random access File System(RFS), etc.

BGN organize a network and each node in the network serves as both server and client. Generally one ip and one port
can provide the multiple service/modules communication in the network.

BGN support several computation models and their combination: master-slave, master-pool + slave-pool, cross-connection,
workflow, no-central, etc.

BGN is driven by task mechanism where a task is subtask list which looks like as a RPC --- communication is tranparent to user! 

BGN support process pool, thread pool, module pool, memory pool etc.

According to BGN communication protocol, any external application can communicate with BGN.

BGN is also a powerful PAAS.

Usage
<TODO>

AUTHORS

BGN was written by Chaoyong Zhou (bgnvendor at gmail) and is released under the GPLv2 license and commercial license.
