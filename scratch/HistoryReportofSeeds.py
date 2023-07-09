
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

# import numpy as np
# import pandas as pd
# import matplotlib.pyplot as plt

import math
import decimal

if __name__ == '__main__':

	# This file generate a output history report file for showing the results from running a specific scenarios with different seeds.
	# At the end, it generates another output file for saving the Average Latency and Standard deviation from all of these runs with different seeds.


	# patternPath = sys.argv[1]    #a-FEC-0-No-Table-1-800-1.5-16-1
	label = sys.argv[1]		# a-FEC-0-No-Table-1-800-1.5-16
	Counter = int(sys.argv[2])		#3

	out_put_file_name = 'outputHistoryReport_' + label
	outPutFile = open(out_put_file_name, "w")

	out_put_Ave_file_name = 'outputAveReport_' + label
	outPutAveFile = open(out_put_Ave_file_name, "w")

	# out_put_Latency_file_name = 'AveLatency_' + str(label) + '.txt' 
	# outPutLatencyFile = open(out_put_Latency_file_name, "w")

	# put the all results ( different seeds ) of one scenario in one file  
	# - e.g. InfoFileArray[1][6]  - 1 = i = info for each seed , 6 = j = specific value for 6th variable in each run. 
	InfoFileArray = []
	for count in range (1,Counter+1):
		report_file_name = str(label) + "-" + str(count) +'/1_result/outputReportFile'
		# print ("File name = " + str(report_file_name) + "\n")
		report_file = open(report_file_name, "r")
		lines = report_file.readlines()
		report_list = [l.rstrip("\n").split(',') for l in lines]       
		rows = []
		rows.append("Title")
		values = []
		values.append(str(label) +"-" + str(count))
		for line in report_list:
			rows.append(line[0])
			values.append(line[1])
		InfoFileArray.append(values)

	outPutFile.write(','.join(rows)+ "\n")
	for i in InfoFileArray:
		outPutFile.write(','.join(i)+ "\n")

	#---------- 
	runMapInfo = []
	for i in range (0, Counter):
		run_info = {}
		for j in range (0, len(rows)):
			run_info[rows[j]] = InfoFileArray[i][j]  
		runMapInfo.append(run_info)
		# print (run_info)

############################################
	
	# print(runMapInfo)

	# [{'Title': 'a-FEC-0-No-Table-1-800-1.5-16-10', 
	#   'Simulation_Time(s)': '72.0', 
	#   'Sent_I_Frames': '72', 
	#   'Recv_I_Frames': '69', 
	#   'I_Frame_Loss_Rate(%)': '4.166666666666666', 
	#   'Sent_I_Frames-HighQuality': '46', 
	#   'Sent_I_Frames-LowQuality': '26', 
	#   'Recv_I_Frames-HighQuality': '44', 
	#   'Recv_I_Frames-LowQuality': '25', 
	#   'Total_Sent_Packets': '98210', 
	#   'Total_Recv_Packets': '84932', 
	#   'Packet_Loss_Rate(%)': '13.52000814581', 
	#   'Total_Sent_FEC_Packets': '5596', 
	#   'Total_Recv_FEC_Packets': '5195', 
	#   'Sent_FEC_packets/Sent_Total_packets(%)': '5.6979940942877505', 
	#   'Recv_FEC_packets/Recv_Total_packets(%)': '6.1166580323082', 
	#   'Throughput(Mbps)': '13.724373666666667', 
	#   'Total_Average_Latency(s)': '0.11788211169324499'}, ...]

	# "Simulation_Time(s),"
	# "Sent_I_Frames," + str
	# "Recv_I_Frames," + str
	# "I_Frame_Loss_Rate(%)," +
	# "Sent_I_Frames-HighQuality," +
	# "Sent_I_Frames-LowQuality," +
	# "Recv_I_Frames-HighQuality," +
	# "Recv_I_Frames-LowQuality," +
	# "Total_Sent_Packets," +
	# "Total_Recv_Packets," +
	# "Packet_Loss_Rate(%)," +
	# "Total_Sent_FEC_Packets," +
	# "Total_Recv_FEC_Packets," +
	# "Sent_FEC_packets/Sent_Total_packets(%),"
	# "Recv_FEC_packets/Recv_Total_packets(%),"
	# "Throughput(Mbps)," +
	# "Total_Average_Latency(s),"
	# "BP_Frame_Loss_Rate(%)," +
	# "BP_Frame_Loss_Rate_AfterCorrection(%)," +
	# "FrameHighQuality," +
	# "FrameLowQuality," +
	# "RecvFrameHighQuality," +
	# "RecvFrameLowQuality," +

#############################################
	
	sum_history = {}
	aveHistory_Dic = {}
	for a in range (0,len(rows)):
		sum_history[rows[a]] = 0
		aveHistory_Dic[rows[a]] = 0

	aveHistory_Dic['Title'] = label
	aveHistory_Dic['Simulation_Time(s)'] = runMapInfo[0]['Simulation_Time(s)']

	IndexNameVariable = 2    # becasue we did a history act on 'Title' and 'Simulation_Time(s)'. So it should be started from index =2 
	for i in range (0,Counter):
		for a in range (2, len(rows)):
			sum_history[rows[a]] += float(runMapInfo[i][rows[a]])
	for j in range (2, len(rows)):
		# aveHistory_Dic[rows[j]] = math.ceil(sum_history[rows[j]] / Counter)  # rond be addade bozorgtar.
		if (rows[j] == 'I_Frame_Loss_Rate(%)' or rows[j] == 'Packet_Loss_Rate(%)' or rows[j] == 'Sent_FEC_packets/Sent_Total_packets(%)' or rows[j] == 'Recv_FEC_packets/Recv_Total_packets(%)' or rows[j] == 'Throughput(Mbps)' or rows[j] == 'Total_Average_Latency(s)' or rows[j] == 'BP_Frame_Loss_Rate(%)' or rows[j] == 'BP_Frame_Loss_Rate_AfterCorrection(%)' ):
			aveHistory_Dic[rows[j]] = float(float(sum_history[rows[j]]) / float(Counter))
		else:
			aveHistory_Dic[rows[j]] = round(sum_history[rows[j]] / float(Counter))  	
	

	# # Calculating average latency and standard deviation on all different seed of one scenario.
			
	Average_latency = 0.0
	sum_latency = 0.0 
	for i in range (0,Counter):
		sum_latency += float(runMapInfo[i]['Total_Average_Latency(s)'])
	Average_latency = sum_latency/Counter
	
	sum_diff = 0.0
	for j in range (0, Counter):
		sum_diff += pow( float(runMapInfo[i]['Total_Average_Latency(s)']) - Average_latency,2)
	
	std_dev = math.sqrt(float(sum_diff/Counter)) 

	rows.append('std_dev_latency(s)')
	aveHistory_Dic['std_dev_latency(s)'] = std_dev

	for b in range (0,len(rows)):
		outPutAveFile.write(str(rows[b]) + "," + str(aveHistory_Dic[rows[b]]) + "\n")

	outPutAveFile.close()
	outPutFile.close()
