版本号：v1.0
============
ScreenCoordsService 采用本地代码封装了系统内核工具 /system/bin/getevent，解析了 Linux 多点触控协议，将触摸事件封装成 MotionEvent 对象，回调监听对象 OnScreenTouchListener。<br>

TODO:
=====
1、生产者、消费者问题；<br>
2、异步控制；<br>

Bug:
====
1、OnScreenTouchListener 在使用 MotionEvent 的过程中，可能会抛出 java.lang.IllegalArgumentException: pointerIndex out of range 异常；根据日志观察，应该是异步导致的问题。<br>

参考链接：
==========
1、Linux多点触控协议及编码：<br>
https://www.kernel.org/doc/Documentation/input/multi-touch-protocol.txt<br>
https://www.kernel.org/doc/Documentation/input/event-codes.txt<br>
2、Google linux/input.h 第一条返回结果：<br>
http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/include/linux/input.h<br>
如果你使用 Ubuntu 或其他 Linux 开发环境，可以在 /usr/include/linux/ 下找到：<br>
cd /usr/include/linux<br>
find input.h<br>
注意！使用 android ndk-build 时，如果编译不通过并提示常量未定义，可以查看 &lt;ndk&gt;/platform/.../usr/include/linux/input.h 是否定义了相应的宏；如果没有，指定具体头文件，比如<br>
\#include &lt;/usr/include/linux/input.h&gt;     // Ubuntu 12.04 LTS<br>
感谢：
======
http://www.pocketmagic.net/2012/04/injecting-events-programatically-on-android/#.UbQK0KKafGk<br>
http://code.google.com/p/android-event-injector/<br>
