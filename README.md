# HideDriver
Hide Driver，win7*64
- 先通过EtwWriteString找MiProcessLoaderEntry函数
(first using EtwWriteString find for MiProcessLoaderEntry funciton)
- 用MiProcessLoaderEntry移除DriverObject->DriverSection（直接断链会遭遇PG）
(use MiProcessLoaderEntry remove DriverObject->DriverSection dont straight set  DriverObject->DriverSection. u got be BSOD 109)
- 然后抹去Driver特征，有个问题，不能在DriverEntry中抹除驱动特征，所以开了个线程，当发现驱动加载完毕的时候就去抹除特征。
(remove driver information)
