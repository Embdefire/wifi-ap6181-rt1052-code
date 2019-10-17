#include "tcpecho.h"

#include "lwip/opt.h"
#include <lwip/sockets.h>

#include "lwip/sys.h"
#include "lwip/api.h"
#include "fsl_debug_console.h"
#include "appliance.h"

static void 
tcpecho_thread(void *arg)
{

}
/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
  sys_thread_new("tcpecho_thread", tcpecho_thread, NULL, 512, 8);
}
/*-----------------------------------------------------------------------------------*/


