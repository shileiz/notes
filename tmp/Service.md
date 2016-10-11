*  参考： docs/reference/android/app/Service.html#LocalServiceSample
*  同一进程使用 Service 时很简单，clients 只需把从 Service 得到的 IBinder 强转为 Service 暴露的接口类即可。

> When used in this way, by assuming the components are in the same process, you can greatly simplify the interaction between them:
> clients of the service can simply cast the IBinder they receive from it to a concrete class published by the service. 

* 