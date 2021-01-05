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

##### 1.秘钥协商

- 使用相同的算法生成秘钥

  - 客户端发送：随机字符串r1+消息认证码HMAC{(@serverid+clientid@),r1}

  - 服务端接受验证消息认证码后，证明r1完整，自己生成随机字符串r2，

  - 服务端 SHA1(r1+r2) 生成秘钥，并且将秘钥标志rv置0，发送给客户端

    >  网点对应**秘钥信息存入数据库**

- 客户端接受秘钥，读取秘钥标志rv，确认秘钥生成成功，记录秘钥信息

##### 2.秘钥校验

###### 3.秘钥注销

###### 4.秘钥查看

- 根据日期查看
- 根据网点查看

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

待完成### 有需要请联系：

**QQ**：`309151115`

**微信**：`tty309`

