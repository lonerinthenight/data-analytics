Xilinx TPCH Query Elements
======================

This README file contains the following sections:
  1. OVERVIEW
  2. DESIGN FILE HIERARCHY  
  3. COMPILATION AND EXECUTION

## 1. OVERVIEW
This directory provides sample kernel implementation for Query 6 and Query 1 on TPCH data tables. It can be run on F1 instances to produce high quality TPCH results.

## 2. DESIGN FILE HIERARCHY

Files                 | Description
----------------------|----------------------------------------------------------------------------
src/sqlq6tpch.cpp     | TPCH Query 6 Kernel with top level HLS unit
src/sqlq6procunit.cpp | TPCH Query 6 Kernel for one HLS processing unit
src/sqlq1tpch.cpp     | TPCH Query 1 Kernel with top level HLS unit
src/sqlq1procunit.cpp | TPCH Query 1 Kernel for one HLS processing unit
src/sqlpageloop.cpp   | Kernel with loopback logic for testing the data flow
src/sqldate.cpp       | Sqldate library to handle SQL data formats directly into the FPGA logic
src/sqltbl.cpp        | Read SQL table, format as needed to send to FPGA device memory
src/sqlutils.cpp      | SQL Utility functions
src/sqlhost.cpp       | For spawning SQL query 
src/tbsqlutils.cpp    | Unit test bench for SQL utility library
src/q6main.cpp        | Main code to execute query 6
src/q1main.cpp        | Main code to execute query 1
src/main.cpp          | Host code with main functions and the args


## 3. COMPILATION AND EXECUTION

### Compiling to create host code
Set up environment for Xilinx SDx release
```
$source <SDX_INSTALL_DIR>/settings64.sh
```
Make XDB connector library in database/xdbconn (see README for instructions)
Build host code fo rtpch
```
make
```
Run TPCH query with the lineitem table csv file
```
$make QUERYNO=6 DBMODE=csv run_sw_emu
```
To run TPCH query with the lineitem table csv file
```
$make QUERYNO=6 DBMODE=csv run_hw_emu
```
Run TPCH query 6 using HLS C functions as OpenCL kernel on acceleration card
Compile host code and xclbin for KU3 hardware Change XPART in makefile to select between KU3/KU115
```
$make host xbin
```
Here is a brief step by step instruction on how to run it on the AWS-F1 platform:
* Building the executable by calling:
```
make all XPART=AWS
```
* Building the xclbin by calling:
```
make xbin FLOW=hw QUERYNO=6
```
Chnage queryno to 1 to create the xclbin for query 6
* Creating and registering the AFI
 
Please follow AWS instructions to create an AFI image for Amazon F1
* Check AFI registration:
```
more *afi_id.txt
aws ec2 describe-fpga-images --fpga-image-ids <afi-id from file>
```
* Setup and execute
To run query 6 please use the following command 
```
sudo sh
source /opt/Xilinx/SDx/2017.1.rte/setup.sh
bin/sqltpch -kernel sqlq6tpch -xclbin <replace_with_aws_xclbin_name>  -query 6 -db test/samplelineitem.tbl -ntuples 256"
```
To run query 1 please use the following command 
```
sudo sh
source /opt/Xilinx/SDx/2017.1.rte/setup.sh
bin/sqltpch -kernel sqlq6tpch -xclbin <replace_with_aws_xclbin_name>  -query 1 -db test/samplelineitem.tbl -ntuples 256"
```
You can feed in your own lineitem tbl generated by TPCH dbgen, and it should run as well. This code has been checked on up to 100GB of data size, and it scales well.


