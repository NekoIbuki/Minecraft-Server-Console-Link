#Minecraft Server Console Link  

This is a convenient tool for managing MC servers, mainly used to output and input information on the MC server side, highlighting commands, errors, player chat information, join information, etc. with color  

(convenient for accessing other software, web pages)  

这是一个方便管理MC服务器的工具,主要用于输出和输入MC服务端的信息用颜色强调出了命令,报错,玩家聊天信息,加入信息等(方便用于接入其它软件,网页的工具)  

At present, it only has an output function, which is output through the OUTMSGLink_DLL interface  

目前只具备输出功能,通过OUTMSGLink_DLL的接口进行输出  

Calling the IOutputMessage class uses GetGlobalOutputMessage  

调用IOutputMessage类使用GetGlobalOutputMessage方法即可获取输出信息调用IOutputMessage类使用GetGlobalOutputMessage方法即可获取输出信息  

GetPChat() to get player chat information  

GetPChat()获取玩家聊天信息  

GetPCONN() to get information about players joining the server  

GetPCONN()获取玩家加入服务器的信息  

GetPCMD() to get information about the player executing the command  

GetPCMD()获取玩家执行命令的信息  

GetBCMD() to get the information about the command box executing the command  

GetBCMD()获取命令方块执行命令的信息  

GetSERROR() to obtain the error information in the server console  

GetSERROR()获取服务器控制台报错信息  

La información anterior se almacenará en el archivo en el directorio raíz del proyecto.  
La herramienta se inicia configurando la ruta de la carpeta del lado servidor del server_config.txt y se inicia la configuración de inicio del lado servidor  
以上信息会存储在项目根目录下的文件内.  
通过配置server_config.txt的服务端文件夹路径,服务端启动配置启动该工具  

(Currently in the development stage, many functions are not perfected, this is my first project, I hope you will guide me)  

(目前处于开发阶段很多功能没有完善,这是我的第一个项目希望各位指导)
