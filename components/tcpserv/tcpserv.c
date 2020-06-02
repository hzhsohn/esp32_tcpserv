#include "tcpserv.h"

int g_tcps_port=0;
int g_tcps_tmp_fd=0;
//客户端SOCKET
int g_tcps_clientFD=0;

static struct sockaddr_in tcps_fd;
PF_TCPSERV_READ * g_trans_tcps_recv;

int tcps_creat_socket_server(in_port_t in_port, in_addr_t in_addr)
{
	int socket_fd, on;
	//struct timeval timeout = {10,0};

	tcps_fd.sin_family = AF_INET;
	tcps_fd.sin_port = in_port;
	tcps_fd.sin_addr.s_addr = in_addr;

	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0))<0) {
		perror("listen socket uninit\n");
		return -1;
	}
	on=1;
	//setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int) );
	//CALIB_DEBUG("on %x\n", on);
	if((bind(socket_fd, (struct sockaddr *)&tcps_fd, sizeof(tcps_fd)))<0) {
		perror("cannot bind srv socket\n");
		return -1;
	}

	if(listen(socket_fd, 1)<0) {
		perror("cannot listen");
		close(socket_fd);
		return -1;
	}
	return socket_fd;
}

void app_tcpserv_task_accept()
{
	//printf("app_websocket_task_accept\n");

	//---------------------
	struct sockaddr_in  client;
	int client_size=sizeof(client);
	//---------------------
	int socket_fd = tcps_creat_socket_server(htons(g_tcps_port),htonl(INADDR_ANY));
	//----------------------
	if( socket_fd >= 0 ){
		for(;;){
				g_tcps_tmp_fd=accept(socket_fd,(struct sockaddr*)&client,(socklen_t*)&client_size);
				if(g_tcps_tmp_fd>0L){
							//---------------------
							printf("app_tcps_task_accept g_tcps_tmp_fd=%d \n",g_tcps_tmp_fd);
							
							if(g_tcps_clientFD>0)
							{
								//如果连接中先断开
								close(g_tcps_clientFD);
							}
							g_tcps_clientFD=g_tcps_tmp_fd;
				}
				else
				{
						ESP_LOGI("tcp serv","accept error restart...");
						esp_restart();
				}
			}
	}

	vTaskDelete(NULL); 
}
void app_tcpserv_task_recv()
{
	//printf("app_websocket_task_recv\n");

	int len=0;
	char recv_buf[128];
	for(;;)
	{
		if(g_tcps_clientFD>0)
		{
				//recv会阻塞
				len = recv( g_tcps_clientFD, recv_buf, sizeof(recv_buf), 0 );
				if(len>0)
				{
					g_trans_tcps_recv(recv_buf, len);
				}if(-1==len)
				{
					close(g_tcps_clientFD);
					g_tcps_clientFD=0;
				}				
		}
		else
		{
			//延时
			vTaskDelay(100 / portTICK_PERIOD_MS);      
		}
	}

	vTaskDelete(NULL); 
}

void  tcpserv_init(PF_TCPSERV_READ*pf, int port)
{
	if(port>0)
	{
		g_trans_tcps_recv=pf;
		g_tcps_port=port;
	}
	else
	{
		ESP_LOGI("websocket","Failure to initizal port=%d",port);
	}
}

void  tcpserv_send(char* buf,int len)
{
	if(g_tcps_clientFD>0)
	{
		write(g_tcps_clientFD,buf,len);
	}
}