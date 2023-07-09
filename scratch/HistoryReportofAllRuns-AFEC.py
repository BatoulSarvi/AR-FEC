
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
import os


if __name__ == '__main__':

	
	output_label = sys.argv[1]		# outputHistoryReport_All

	directory = "."  # Replace with the directory path you want to search in
	
	# Get a list of all files in the directory
	file_list = os.listdir(directory)

	# Filter the list to only include files with the specified prefix
	prefix_AFEC = "outputAveReport_a-FEC-"
	matching_files_AFEC = [filename for filename in file_list if filename.startswith(prefix_AFEC)]
	
	
	# Print the list of matching files
	# print(matching_files_AFEC)

	out_put_file_name = output_label + "_AFEC"
	# print (out_put_file_name)
	outPutFile_AFEC = open(out_put_file_name, "w")

	# put the all results ( different seeds ) of one scenario in one file  
	# - e.g. InfoFileArray[1][6]  - 1 = i = info for each seed , 6 = j = specific value for 6th variable in each run. 
	InfoFileArray = []
	for file in matching_files_AFEC:
		current_file = open(file, "r")
		lines = current_file.readlines()
		current_file_lines = [l.rstrip("\n").split(',') for l in lines]       
		rows = []
		values = []
		for line in current_file_lines:
			rows.append(line[0])
			values.append(line[1])
		InfoFileArray.append(values)

	# Sort the InfoFileArray by the first element of each list (i.e., the element at index 0)
	InfoFileArray.sort(key=lambda x: x[0])

	# because the number of column is larger than NoFEC, so we keep this before rewriting of next loops
	outPutFile_AFEC.write(','.join(rows)+ "\n")
	for i in InfoFileArray:
		outPutFile_AFEC.write(','.join(i)+ "\n")

	outPutFile_AFEC.close()


##################################################

# Title			a-FEC-0-High-Table-1-600-1.0-16
# Simulation_Time(s)	72
# Sent_I_Frames	72
# Recv_I_Frames	56
# I_Frame_Loss_Rate(%)	21.9444444444444
# Sent_I_Frames-HighQuality	72
# Sent_I_Frames-LowQuality	0
# Recv_I_Frames-HighQuality	56
# Recv_I_Frames-LowQuality	0
# Total_Sent_Packets	180881
# Total_Recv_Packets	100120
# Packet_Loss_Rate(%)	44.2299257332386
# Total_Sent_FEC_Packets	13319
# Total_Recv_FEC_Packets	8113
# Sent_FEC_packets/Sent_Total_packets(%)	6.75792074577527
# Recv_FEC_packets/Recv_Total_packets(%)	8.22034912120237
# Throughput(Mbps)	16.2366222666667
# Total_Average_Latency(s)	0.220972737244373
# BP_Frame_Loss_Rate(%)	28.2762046696473
# BP_Frame_Loss_Rate_AfterCorrection(%)	33.6065573770492
# FrameHighQuality	2114
# FrameLowQuality	0
# RecvFrameHighQuality	1393
# RecvFrameLowQuality	0
# std_dev_latency(s)	0.008400239950599



