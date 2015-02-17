# LatencyUnderLoad
Network latency under load test

This test is for measuring latency spikes under load for TCP network.

The latency_load_test.sh script is a wrapper for the test.
It include some options which are currently hard-coded in the script,
such as the cpus to run on.
It also support running over libvma (libvma.googlecode.com).
Please take a look at the script before running.

======
OUTPUT
======
The output files are per client process.
They are in csv format, and their name include the process ID.
For example, data_25434.csv:
0,51.000000,1
1,16.000000,1
2,11.000000,1
3,11.000000,1
4,11.000000,1

Where the first colum is the sample number,
the second colum is the round trip latency,
and the third colum is the connection ID.
