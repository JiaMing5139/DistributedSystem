# muduo_study
从零用c++11实现muduo的核心功能
实现两个例子HttpEchoServer和Chat with protobuf

#### 参考书籍
```
Linux 多线程服务端编程
```

#### 编译
Reactor服务器
```
(Cmake Version >= 3.1.5)
Cmake CMakeLists.text
make
./muduo_study
```
ChatServer
```
(protobuf 3.12.2)
ChatServer
cd ChatServer
cmake CMakeList
make
./bin/ChatServer port
./bin/ChatClient ip port
```
## 笔记
## TcpConnection 

#### Q1:TcpConnection的状态设计（非阻塞）

event\state | closed |  closing | connected 
-|-|-|-
new connection | setState(Connected) | ❌ | ❌|
readable event|  ❌ | 1.读取数据到inputbuffer <br>2.调用onMessagecallback |1.读取数据到inputbuffer  <br>2.调用onMessagecallback|
send          | ❌ | ❌ |if(outputbuffer有数据){<br>添加数据到outputbuffer<br>} else{<br>尝试发送数据，若没有发送完，则添加到outputbufer中，并监听可写事件，若发送完毕调用writeCompleteCallback<br>}|
writeable event| ❌ | if(outputbuffer有数据) {<br>尝试发送outputbuffer中的数据，未发送的添加到outputbuffer中，若发送完毕，关闭监听可读事件,调用writeCompleteCallback，<br>继续shutdown<br>} |if(outputbuffer有数据){<br>尝试发送outputbuffer中的数据，未发送的添加到outputbuffer中，若发送完毕，关闭监听可读事件,调用writeCompleteCallback<br>}|
shutdown | ❌ | ❌ |if(outputbuffer有数据) {<br>setState(closing)<br>}else{调用shutdown}|
closed event(read == 0) | ❌ |if(outputbuffer有数据) {<br>丢弃数据<br>}<br>  在poll,和Tcpserver中析构资源<br>setState(closed) |if(outputbuffer有数据) {<br>丢弃数据<br>}<br> 在poll,和Tcpserver中析构资源<br>setState(closed) |


在关闭读到0，被动关闭连接时，如果网络库管理的输出缓冲区还有数据，是否需要继续发送出去？若发送时遇到sigpipe 再对相关资源进行析构。

#### Q2:TcpConnection和Channel的生命周期，在one loop per thread中析构正确？
```
（在实现中，Channel用shared_ptr管理)
TcpConnection的shared_ptr被Tcpserver中的TcpConnectionMap一直持有
Channel的shared_ptr被poll中的ChannelMap和TcpConnection中的成员一直持有

1.创建
New Connection事件触发时
在Tcpserver线程中，向TcpConnectionMap中添加新创建的TcpConnectionptr，并创建Channel。生命周期开始

2.析构
当关闭事件触发时(read = 0)
handlecloseEvent中 分别在IO线程中ChannelMap中erase该Channel，在TcpServer
线程中TcpConnectionMap中erase该Tcpconnection。

如何保证保证运行时自己不被析构？
poll中将channelptr一份到activedChannel中，保证处理事件时channel不会被析构。
TcpConnection必须位于handlecloseEvent中的最后一行，因为在调用TcpServer的closecallback之后，它自身已经被析构
所以在handlecloseEvente结束后 TcpConnection被析构，socket被关闭
在一次事件循环结束后Channel被析构
```


#### Q3:TcpConnection相关的临界区和race condition

## 定时器队列

#### Q1:为什么已经有了Timerfd，还要用TimerQueue去维护定时器的触发？
```
减少文件描述符的使用，若不用timerQueue，一秒内创建1000个定时器，则需要向epoll/poll注册1000个可读事件
而使用TimerQueue时，只维护一个最早的触发的fd,当TimerQueue中的timerfd事件触发时，触发后将一秒内的定时
器事件全部手动触发。这在每一秒内只用维护一个fd，一分钟最多60个。

相关实现:
Timer/TimerStamp:
    friend bool operator < (const Timestamp & ,const Timestamp &);
    friend bool operator == (const Timestamp & ,const Timestamp &);
    
Timer/TimerQueueBase
```     
#### Q2:TimerQueue中维护Timer的数据结构？
```
数据结构要能高效的找出最早触发的一批定时器。而且需要能快速的移除和添加定时器。
线性表：查找时间复杂度是O(N)
优先队列：一般用堆实现，查找时间复杂度O(LogN），因为二叉堆实现一般是用连续的内存是实现，
所以可以较好的利用局部性原理的缓存，效率高
二叉搜索树：红黑树实现，查找时间复杂度O(LogN) ，但二叉树一般是不连续空间，无法利用局部性原理，
效率比优先队列低。

具体实现：
数据结构（红黑树）: Timer/TimerQueue: set<pair<Timerstamp,shared_ptr<Timer>>>
定时器查找：Timer/TimerQueueBase : handleEvent
定时器添加: Timer/TimerQueueBase : addTimer

```
#### Q3：Timer的线程安全性，Timer属于EventLoop，而EventLoop中的成员会被多个线程看到，如果在多个线程向TimerQueue添加如何保证安全？

#### Q4: 如何实现对指定Timer的删除？

```

```


## 异步日志 




# Benchmark
测试Http Echo Serve <br>
测试工具 ab <br>
测试机器:CPU 2核， 内存2G
#### 无LOG 禁用Nagle
并发数\线程数 | 1 |  2 | 4 
-|-|-|-
50| 16734 | 19678 |20331 |
200| 15532| 22138 |21451 |

#### 无LOG 开启Nagle
并发数\线程数 | 1 |  2 | 4 
-|-|-|-
50| 13417 | 15339 |14113 |
200| 13266 | 14527 | 15014|

#### 有LOG
并发数\线程数 | 1 |  2 | 4 
-|-|-|-
50| 5713 | 6802 |5834 |
200| 7378| 6873 |6210 |

#### 优化LOG,对时间进行缓冲，尽量少调用localtime
并发数\线程数 | 1 |  2 | 4 
-|-|-|-
50| 9033 | 14363 |12409 |
200| 9560| 13164 |12728 |


