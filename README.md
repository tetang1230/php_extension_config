《config》扩展是实现将配置选项加载到内存中，先来看一个简单的config.ini：

;参数值长度不能超过255

[define]
;============================服务中心域名======================
;消息转发服务器节点一
MSG_SERVER_0            =       http://msg0.10000.cn/
;用户服务中心网址
USER_SERVICE            =       http://user.service.10000.cn/
;消息服务中心网址
MSG_SERVICE             =       http://msg.service.10000.cn/

[config]
max_upload_size          =      2048
login_redirect_url          =     http://login.service.10000.cn/

语法格式：
“;”开头表示注释
[define]下面的选项，表示定义为宏，等同于define(key, value)，客户端调用方式：echo MSG_SERVER_0
[config]下面的选项，表示定义为key-value形式的链表，客户端调用方式：echo config('max_upload_size')

建议使用场景：
1、多个项目中需要调用共用的配置，使用config来管理，避免一式多份的拷贝维护。
2、配置选项很多，使用config加载入内存，可以避免PHP每次加载配置文件进行解释执行。



一、安装步骤（centos）
1.安装PHP的开发环境
yum -y install php-devel

2.解压扩展包
tar zxf config.tar.gz

3.进入目录，编译安装
cd config
phpize
./configure --with-php-config=/usr/bin/php-config
make && make install

4.配置php.ini
末尾行新增
extension = config.so
[config]
config.path = 配置文件绝对路径

扩展下载：
注：目前仅测试编译PHP5.3.3，其他版本未测试编译