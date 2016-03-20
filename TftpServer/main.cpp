#include "TftpServer.h"
#include <Winsock2.h>
using namespace std;

int main()
{
	TftpServ serv(69);
	serv.start();
	return 0;
}