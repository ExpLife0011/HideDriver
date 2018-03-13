# HideDriver
Hide Driver，win7*64
- 先通过EtwWriteString找MiProcessLoaderEntry函数
- 用MiProcessLoaderEntry移除DriverObject->DriverSection（直接断链会遭遇PG）
- 然后抹去Driver特征，有个问题，不能在DriverEntry中抹除驱动特征，所以开了个线程，当发现驱动加载完毕的时候就去抹除特征。
