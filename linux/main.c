#include "netfact.h"
#include "fact.h"
#include "psmlmagic.h"

int main(int argc, char** argv)
{
  if(argc < 3) {
    printf("Usage: %s mac_address interface\n",argv[0]);
    return 1;
  }

  unsigned int val;
  unsigned char dest_mac[ETH_ALEN];
  get_mac(argv[1],dest_mac);
  net_handle recv_hnd,send_hnd;

  /*init handles*/
  initRecv(&recv_hnd, argv[2]);
  initSend(&send_hnd, argv[2], dest_mac);

  val = START_CODE;
  sendFrame(&send_hnd, (void *) &val, sizeof(val));
  //turn on the trace
  PSML_MAGIC(PSML_MAGIC_TSTART);

  /*listen for values, compute factorial and send response*/
  while(1) {
    PSML_MAGIC(PSML_MAGIC_MARK0);
    recvFrame(&recv_hnd,(void *) &val, sizeof(val));
    PSML_MAGIC(PSML_MAGIC_MARK0);
    if(val == EXIT_CODE) /*stop if we receive the exit code*/
      break;
    PSML_MAGIC(PSML_MAGIC_MARK0);
    val = fact(val);
    PSML_MAGIC(PSML_MAGIC_MARK0);
    sendFrame(&send_hnd, (void *) &val, sizeof(val));
    PSML_MAGIC(PSML_MAGIC_MARK0);
  }

  PSML_MAGIC(PSML_MAGIC_TSTOP);
  //turn off the trace
  /*cleanup handles*/
  closeHandle(&recv_hnd);
  closeHandle(&send_hnd);
  
  return 0;
}
