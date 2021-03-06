/**********

Copyright 2017 Xilinx, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

**********/

/*

CREATE TABLE lineitem ( L_ORDERKEY    INTEGER NOT NULL,
1                             L_PARTKEY     INTEGER NOT NULL,
2                             L_SUPPKEY     INTEGER NOT NULL,
3                             L_LINENUMBER  INTEGER NOT NULL,
4                             L_QUANTITY    DECIMAL(15,2) NOT NULL,
5                             L_EXTENDEDPRICE  DECIMAL(15,2) NOT NULL,
6                             L_DISCOUNT    DECIMAL(15,2) NOT NULL,
7                             L_TAX         DECIMAL(15,2) NOT NULL,
8                             L_RETURNFLAG  CHAR(1) NOT NULL,
9                             L_LINESTATUS  CHAR(1) NOT NULL,
10                             L_SHIPDATE    DATE NOT NULL,
11                             L_COMMITDATE  DATE NOT NULL,
12                             L_RECEIPTDATE DATE NOT NULL,
13                             L_SHIPINSTRUCT CHAR(25) NOT NULL,
14                             L_SHIPMODE     CHAR(10) NOT NULL,
15                             L_COMMENT      VARCHAR(44) NOT NULL);

select
	sum(l_extendedprice * l_discount) as revenue
from
	lineitem
where
	l_shipdate >= date '1994-01-01'
	and l_shipdate < date '1994-01-01' + interval '1' year
	and l_discount between 0.06 - 0.01 and 0.06 + 0.01
	and l_quantity < 24;

*/
#include <string.h>
#include "sqldecimal.h"
#include "sqldate.h"
#include "sqldefs.h"
#include "sqlq6defs.h"

SqlDec17P2D SQLEVAL(sqlq6procunit,SQLKERNEL_ID)(hls::stream<SqlDefs::RdDdrWord> &bufStream,
                          unsigned char block_num,
                          unsigned char noOfProcUnitBlocks)
{
#pragma HLS inline
  
  SqlDec17P2D revenue;
  revenue.fixedBin = SqlDec17P2D::Decimal(0);

  // inbuf has now following elements in the order
  // 4  L_QUANTITY    DECIMAL(15,2) NOT NULL,
  // 5  L_EXTENDEDPRICE  DECIMAL(15,2) NOT NULL,
  // 6  L_DISCOUNT    DECIMAL(15,2) NOT NULL,
  // 10  L_SHIPDATE    DATE NOT NULL,
  SqlDec17P2D l_quantity;
  SqlDec17P2D l_extendedprice;
  SqlDec17P2D l_discount;

  SqlDefs::RdTracker rdTracker;
  rdTracker.curChIdx = 0;
  rdTracker.curIdx = 0;  
  
  SqlDefs::readChar(bufStream,rdTracker);
  unsigned int noOfRows = SqlDefs::readChar(bufStream,rdTracker);
  noOfRows = (noOfRows<<8) + SqlDefs::readChar(bufStream,rdTracker);
  noOfRows = (noOfRows<<8) + SqlDefs::readChar(bufStream,rdTracker);
  // procUnitBlock[0]; reserved for other blocks
  
  if (block_num >= noOfProcUnitBlocks)
      noOfRows = 0;

  SqlDate l_shipdate;  

  //processing byte by byte
 LOOP_PU_ROWS: for (unsigned int i =0; i<SqlQ6Defs::ProcUnitBlockMaxRows; i++) {
    //#pragma HLS PIPELINE OFF
    if (i == noOfRows) break;
    l_quantity.SQLEVAL(decToBin,SQLKERNEL_ID)(bufStream,rdTracker);
    l_extendedprice.SQLEVAL(decToBin,SQLKERNEL_ID)(bufStream,rdTracker);
    l_discount.SQLEVAL(decToBin,SQLKERNEL_ID)(bufStream,rdTracker);
    l_shipdate.SQLEVAL(toBin,SQLKERNEL_ID)(bufStream,rdTracker);
    char ch = SqlDefs::readChar(bufStream,rdTracker);
#ifdef HLS_DEBUG
    if (ch != '\n') { // end of row reached, new line is row marker
      printf("Error: row end marker %c is not new line\n",ch);
    }
#endif
    if ( l_shipdate.year >= SqlDate::Year(1994)) {// l_shipdate >= date '1994-01-01'
      if (l_shipdate.year < SqlDate::Year(1995)) { // and l_shipdate < date '1994-01-01' + interval '1' year
        if (l_quantity.fixedBin < SqlDec17P2D::Decimal(2400)) {  // and l_quantity < 24;
          if (SqlDec17P2D::Decimal(5) <= l_discount.fixedBin && l_discount.fixedBin <= SqlDec17P2D::Decimal(7)) { // and l_discount between 0.06 - 0.01 and 0.06 + 0.01
            revenue.fixedBin = revenue.fixedBin + (l_extendedprice.fixedBin * l_discount.fixedBin); // take care of decimals
          }
        }
      }
    }

  }
  
  //Flush only during HLS SIM to avoid warnign clutter
  LOOP_INFLSH: for (unsigned int ml=rdTracker.curIdx; ml<(SqlQ6Defs::ProcUnitBlockSize/SqlDefs::RdDdrWordSize); ml++) {
#pragma HLS LOOP_TRIPCOUNT min=128 max=128 avg=128
#pragma HLS PIPELINE
    bufStream.read();
  }
  
  return revenue;
  
}
