/* Copyright 2011 Boston University. All rights reserved. */

/* Redistribution and use in source and binary forms, with or without modification, are */
/* permitted provided that the following conditions are met: */

/*    1. Redistributions of source code must retain the above copyright notice, this list of */
/*       conditions and the following disclaimer. */

/*    2. Redistributions in binary form must reproduce the above copyright notice, this list */
/*       of conditions and the following disclaimer in the documentation and/or other materials */
/*       provided with the distribution. */

/* THIS SOFTWARE IS PROVIDED BY BOSTON UNIVERSITY ``AS IS'' AND ANY EXPRESS OR IMPLIED */
/* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND */
/* FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BOSTON UNIVERSITY OR */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR */
/* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR */
/* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF */
/* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* The views and conclusions contained in the software and documentation are those of the */
/* authors and should not be interpreted as representing official policies, either expressed */
/* or implied, of Boston University */

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
