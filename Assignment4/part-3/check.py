import sys
import subprocess
#####################################
# python3 check.py 30 10000         #
# number of threads = 30            #
# number of testcases = 10000       #
#####################################
subprocess.check_output('./serial_hash sample-60000.bin 60001 1',shell = True)

serial_output = (open('thread-1.out','r').read().split('\n'))
serial_output.pop()
serial_dict = dict()
# print(serial_output)
for line in serial_output:
	ele = line.split()
	serial_dict[int(ele[0])] = ele[1:3]

subprocess.check_output('./parallel_hash sample-60000.bin 60001 %d'%(int(sys.argv[1])),shell = True)
parallel_dict = dict()
for i in range(0,int(sys.argv[1])):
	thread_num = i+1
	parallel_output = open('thread-%d.out'%(thread_num),'r').read().split('\n')
	parallel_output.pop()
	for line in parallel_output:
		ele = line.split()
		parallel_dict[int(ele[0])] = ele[1:3]
# NUmber of queries in sys.argv[2]
for x in range(0,int(sys.argv[2])):
	i = x+1
	c = '%d'%(i)
	# print(c)
	if(serial_dict[i] != parallel_dict[i]):
		print('Failed at ops %d'%(i))
print('Passed!')