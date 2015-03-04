#!/bin/bash
app_type="CLIENT"
warm_up="NOWARMUP"
extra_conn="NOEXTRA"
tcp_nodelay="DELAY"
start_port=10002
msg_size=1000
mps=1000
cpu_range="8-15"
num_connection=10
num_process=1
duration=5
available_core=('9');#'8' '9' '10' '11' '24' '25' '26' '27'
vma_interal_thread_cores=('8')
three_t_rule=0
do_kill=1
vma_path="libvma.so"

function print_usage 
{
    cat <<EOF
    $0 [-hsvwn] [-e <open/close>] [-m <m_size>] [-r <mps>] [-c <connection_num>] [-p <process_num>] [-d <duration_sec>] <server_IP>

    Required Parameters:
      server_IP           = Server IP

    Optional Parameters:
	  -h                    -- print help message
	  -s                    -- run as server
	  -v                    -- load vma
	  -w                    -- Warm up before collecting data
	  -n                    -- Use TCP_NODELAY option
	  -e <extra op>         -- Use extra connection that will be handled during test
	  -m <m_size>           -- message size in bytes
	  -r <mps>              -- message rate per second (client side)
	  -c <connection_num>   -- number of connection per process
	  -p <process_num>      -- number of process to run in parrall (should be less than cpu range)
	  -d <duration_sec>     -- time in seconds to run test
          -k <don't kill>       -- don't kill running instances before test start


EOF
}

OPTIND=1
while getopts "hsvwnke:m:r:R:c:p:d:" opt; do
  case "$opt" in
    h) print_usage
       exit 0
       ;;
    s) app_type="SERVER"
		three_t_rule=1
	  ;;
	v) vma=1
      ;;  
	w) warm_up="WARMUP"
      ;;  
	n) tcp_nodelay="NODELAY"
      ;;
        k) do_kill=0
      ;;
	e) if [ "$OPTARG" == "open" ]; then
		extra_conn="EXTRA_OPEN"
	   elif [ "$OPTARG" == "close" ]; then
		extra_conn="EXTRA_CLOSE"
	   else 
		print_usage
		exit 0		
	   fi
      ;;  	  
    m) msg_size=$OPTARG
      ;;
    r) mps=$OPTARG
      ;;
    R) cpu_range=$OPTARG
      ;;
    c) num_connection=$OPTARG
      ;;
    p) num_process=$OPTARG
      ;;
    d) duration=$OPTARG
      ;; 
    *) echo Unknown argument: $opt
       print_usage
       exit 1
       ;;
  esac
done
shift $((OPTIND-1))

if [ -z "$1" ]; then
    print_usage
    exit 1
fi

declare -r SERVER_IP=$1
declare -r TEST="./latencyLoadTest"
declare -r INTERLEAVE_PARAMETER="numactl --interleave=all"

if [ $do_kill == 1 ]; then
	killall -9 latencyLoadTest
fi

#IFS='-' read -ra core_range <<< "$cpu_range"

#for core_id in $(seq ${core_range[0]} 1 ${core_range[1]}); do
#	available_core+=($core_id)
#done

num_cores=${#available_core[@]}
num_internal_thread_cores=${#vma_interal_thread_cores[@]}

dest_port=$start_port
for process_id in $(seq 0 1 $((num_process-1))); do
	dest_port=$((start_port+process_id))
	core_id=$((process_id%num_cores))
	internal_thread_core_id=$((process_id%num_internal_thread_cores))

	if [ -n "$vma" ]; then
		#VMA_TCP_TIMER_RESOLUTION_MSEC=60000 VMA_TIMER_RESOLUTION_MSEC=60000 
		VMA_TCP_3T_RULES=$three_t_rule VMA_INTERNAL_THREAD_AFFINITY=${vma_interal_thread_cores[internal_thread_core_id]} VMA_PROGRESS_ENGINE_INTERVAL=0 VMA_SELECT_POLL=-1 VMA_RX_POLL=-1 LD_PRELOAD=$vma_path $INTERLEAVE_PARAMETER  taskset -c ${available_core[core_id]} "$TEST" $app_type $SERVER_IP $dest_port $msg_size $mps $duration $num_connection $warm_up $tcp_nodelay $extra_conn &
	else
		$INTERLEAVE_PARAMETER  taskset -c ${available_core[core_id]} "$TEST" $app_type $SERVER_IP $dest_port $msg_size $mps $duration $num_connection $warm_up $tcp_nodelay $extra_conn &
	fi
done

