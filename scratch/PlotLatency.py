
#  * This program is free software; you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License version 2 as
#  * published by the Free Software Foundation;
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program; if not, write to the Free Software
#  * Foundation
#  *
#  * Author: Batoul Sarvi <batoul.sarvi@gmail.com>


#! /usr/bin/env python3

import sys

import matplotlib.pyplot as plt
import numpy as np

from tokenize import Double


if __name__ == '__main__':
	# latency_file_name = sys.argv[1]
	latency_file_name = 'outputLatencyFile'
	# num_GOP = int(sys.argv[2])
	outputNameStr = sys.argv[1]
	outputName = 'Latency-'+ str(outputNameStr) +'.png'

	# print(outputName)

	latency_file = open(latency_file_name, "r")

	lines = latency_file.readlines()
	latency_list = [l.rstrip("\n").split(',') for l in lines]       # gop , min_latency , max_latency , ave_latency

	gopArray = []
	minArray = []
	maxArray = []
	aveArray = []
	ratioArray = []
	for i in latency_list: 				 # gop , min_latency , max_latency , ave_latency, packet_ratio
		gopArray.append(int(i[0]))
		if (i[1] == '99999'):
			minArray.append(0.0)
			maxArray.append(0.0)
			aveArray.append(0.0)
			ratioArray.append(0.0)
		else:	
			minArray.append(float(i[1]))
			maxArray.append(float(i[2]))
			aveArray.append(float(i[3]))
			ratioArray.append(float(i[4]))

	# # ignoring the first GOP
	# gopArray = gopArray[2:]
	# minArray = minArray[2:]
	# maxArray = maxArray[2:]
	# aveArray = aveArray[2:]
	# ratioArray =  ratioArray[2:]


	# xpoints = np.array([1, num_GOP])
	# ypoints = np.array(aveArray[1:num_GOP+1])

	# print(aveArray[:num_GOP])



	# font1 = {'family':'serif','color':'blue','size':12}
	# font2 = {'family':'serif','color':'darkred','size':12}

	# # plt.plot(aveArray[:num_GOP], marker='o', linestyle='-', color='r', ms = 5, mec = 'r', mfc = 'r', label='Average Latency')
	# plt.plot(aveArray, marker='o', linestyle='-', color='r', ms = 5, mec = 'r', mfc = 'r', label='Average Latency')
	# plt.plot(minArray, marker='*', linestyle='--', color='orange', ms = 5, mec = 'orange', mfc = 'orange', label='Min Latency')
	# plt.plot(maxArray, marker='+', linestyle='--', color='hotpink', ms = 5, mec = 'hotpink', mfc = 'hotpink', label='Max Latency')
	# plt.xlabel('GOP', fontdict = font2)
	# plt.xlim([0, len(latency_list)])
	# plt.ylabel('Latency (s)',fontdict = font2)
	# # plt.legend(bbox_to_anchor=(1.05, 1),loc='upper left', borderaxespad=0.)
	# plt.legend(bbox_to_anchor =(1, 1.15), ncol = 3)
	# # plt.legend(loc='lower center', ncol = 3, bbox_to_anchor=(1.05, 1))
	
	# # plt.show()
	# plt.savefig(outputName)




	fig, ax1 = plt.subplots()
	ax1.plot(gopArray,aveArray,color="blue")
	ax2 = ax1.twinx()
	ax2.plot(gopArray,ratioArray,color="green")

	ax1.set_ylabel("Average Latency (s)",color="blue")
	ax2.set_ylabel("Received Packets",color="green")
	
	# plt.show()
	fig.savefig(outputName,bbox_inches='tight')

# #closing all files

			
