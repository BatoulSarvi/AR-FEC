
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

	# This file generate a output history report file for showing the results from running a specific scenarios with different seeds.
	# At the end, it generates another output file for saving the Average Latency and Standard deviation from all of these runs with different seeds.

	output_label = sys.argv[1]		# outputHistoryReport_All

	directory = "."  # Replace with the directory path you want to search in
	
	# Get a list of all files in the directory
	file_list = os.listdir(directory)

	# Filter the list to only include files with the specified prefix
	prefix_NoFEC = "outputAveReport_a-NoFEC-"
	matching_files_NoFEC = [filename for filename in file_list if filename.startswith(prefix_NoFEC)]
	
	# Print the list of matching files
	# print(matching_files_NoFEC)

	out_put_file_name = output_label + "_NoFEC"
	# print (out_put_file_name)
	outPutFile_NoFEC = open(out_put_file_name, "w")


	# put the all results ( different seeds ) of one scenario in one file  
	# - e.g. InfoFileArray[1][6]  - 1 = i = info for each seed , 6 = j = specific value for 6th variable in each run. 

	InfoFileArray = []
	for file in matching_files_NoFEC:
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

	outPutFile_NoFEC.write(','.join(rows)+ "\n")
	for i in InfoFileArray:
		outPutFile_NoFEC.write(','.join(i)+ "\n")
	
	outPutFile_NoFEC.close()


############################################
	
# Simulation_Time(s),72.0
# Sent_I_Frames,73
# Recv_I_Frames,28
# I_Frame_Loss_Rate(%),61.64383561643836
# Total_Sent_Packets,166632
# Total_Recv_Packets,72226
# Packet_Loss_Rate(%), 56.65538431993855
# Throughput(Mbps),7.318558555555556
# Total_Average_Latency(s),0.21717624763358342
# BP_Frame_Loss_Rate(%),20.376629647513276
# BP_Frame_Loss_Rate_AfterCorrection(%),63.061323032351524
# SentFrame,2144
# RecvFrame,793