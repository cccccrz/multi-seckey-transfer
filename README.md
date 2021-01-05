# 多端安全协议传输系统-qt驱动级

## 流程：

### 管理方：

管理人员通过QT界面的配置管理终端

- 配置数据库参数：数据库地址，数据库名，用户名，密码，端口
  - 用于连接数据库
- 配置服务器参数：服务器ID，端口号，最大接入网点数
  - 用于提供服务器接口

- 添加客户端网点信息：客户端（网点）编号，名称，描述，授权码，**状态**
  - **信息存入数据库，给客户端授权**



### 客户：

授权的客户端可以请求访问服务端，服务端**查询数据库认证**后，可以**提供服务**

> 查询数据库中是否存在该网点，允许服务

##### 1.秘钥协商

- 使用相同的算法生成秘钥

  - 客户端发送：随机字符串r1+消息认证码HMAC{(@serverid+clientid@),r1}

  - 服务端接受验证消息认证码后，证明r1完整，自己生成随机字符串r2，

  - 服务端 SHA1(r1+r2) 生成秘钥，并且将秘钥标志rv置0，发送给客户端

    >  秘钥信息：存入本地共享内存；存入数据库

- 客户端接受秘钥，读取秘钥标志rv，确认秘钥生成成功，记录秘钥信息

  > 存入共享内存

##### 2.秘钥校验

- 客户端从共享内存中读取秘钥信息
- 客户端组织应答信息：
  - r1 存放秘钥ID
  - authCode 存放 消息认证码
    - @serverid+clientid@ 与 seckey 进行哈希运算
- 服务端接受并校验，使用相同算法，回复应答信息
  - 秘钥从数据库中获取
- 客户端接受应答信息，rv=0 表示成功

###### 3.秘钥注销

- 客户端发送注销信息
- 服务根据 `clientID` 和 `serverID` 查找 秘钥信息 ，判断信息是否存在
- 删除数据库中 所有 秘钥信息  回复注销结果
  - 待完善，添加选择性删除
- 客户端收到回复，注销成功

###### 4.秘钥查看

- 客户端发送查看信息
- 服务根据 `clientID` 和 `serverID` 查找 秘钥信息 ，并存到队列中
- 服务端**循环发送**应答信息：`seckeyID`
  - 发送一个空包表示结束 且 `seckeyID=0`
  - 待补充 ， 可发送别的信息
- 客户端**循环接受**消息 打印 `seckeyID`
  - 接受到空包跳出循环：`seckeyID=0`
  - 待补充  ，可存储到本地
### 编译问题

gcc4.9 兼容

### 粘包问题：

ASN1编码字符串出错

- 指定字符串长度`strlen()` 不包含`'\0'` ,需要修改成 `strlen()+1`

## 模块划分

> #### 报文编码模块
>
> `class BaseASN1`												ASN1编解码类
>
> `class SequenceASN1:public BaseASN1`	  编解码封装类
>
> `class Codec:public SequenceASN1`			 编解码虚基类
>
> ------
>
> `struct RequestMsg`									请求数据结构体
>
> `class RequestCodec:public Codec`  	 请求数据编解解码类
>
> `struct RespondMsg`									应答数据结构体
>
> `class RespondCodec:public Codec`	   应答数据编解解码类 
>
> ------
>
> `class CodecFactory`											    	编解码工厂基类
>
> `class RequestFactory:public CodecFactory`	  请求编解码生产工厂类
>
> `class RespondFactory:public CodecFactory`	  应答编解码生产工厂类
>
> ------
>
> #### 共享内存操作模块
>
> `class ShareMemory`			共享内存类 -- 封装共享内存方法
>
> `struct NodeSHMInfo`		  秘钥信息结构体
>
> `class SeckeyShm`				存储秘钥类 -- 操作共享内存，存储秘钥信息
>
> ------
>
> #### 网络通信模块
>
> `class TCPSocket`			套接字通信类
>
> `class TcpServer`			TCP监听连接类
>
> ------
>
> `struct ClientInfo`				客户端本地信息结构体
>
> `class ClientOperation`		客户端操作类
>
> `struct ServerInfo`				服务端本地信息类
>
> `class ServerOperation`		服务端操作类
>
> ------
>
> #### 数据库库操作模块
>
> `class OCCIOP`			数据库操作类  -- 获取、更新秘钥ID，检验客户端合法性
>
> ------
>
> #### 日志打印模块
>
> `class ItcastLog`			日志类

## QT模块：

待完成


### 有需要请联系：

**QQ**：`309151115`

**微信**：`tty309`

