# Boogie

Chat application written in Qt using QTcpServer and QSslSocket.

## About:

Chat application with support for group chats and sending images.

## :information_source: Usage:

Server:
Command line application started with optional command line argument representing port number. If not specified, port 10000 is used.

Client:
On first page, enter address of server and port on which to connect. After that enter username and password. If you are logging in first time with that username, account is automatically created with entered password. 
You can add new contacts or create chat groups with your contacts. Sending pictures is supported.

#### :exclamation: NOTE:

Current encryption certificates are bound to "localhost" address. Connecting to any other address(including 127.0.0.1) will fail due to certificates(this is wanted behaviour). If you wish, you can generate new certificates using [this](http://www.infidigm.net/articles/qsslsocket_for_ssl_beginners/) tutorial which was used to generate current ones. 

### :blue_book: Requirements:

```Qt 5```

```C++14```



### :computer: Authors:

**Natalija Jovanović**: [github](https://github.com/JovanovicNatalija)

**Nikola MIlovanović**: [github](https://github.com/Milovanovic15141) <nikolamilovanovic94@gmail.com>

**Vuk Novaković**: [github](https://github.com/wdtbd) [linkedin](https://www.linkedin.com/in/vuk-novakovic/)

