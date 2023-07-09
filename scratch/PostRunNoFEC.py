
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

# from typing import List, Dict, Any, Union
# import os
# from decimal import Decimal
from struct import pack
import sys
import math
# import collections
from tokenize import Double


if __name__ == '__main__':
	# sent_video_file_name = sys.argv[1]
	# recv_video_file_name = sys.argv[2]
	# simulationTime_input = sys.argv[3]
	
	# out_put_file_name = sys.argv[4]
	# out_put_latency_file_name = sys.argv[5]
	# out_put_PLR_file_name = sys.argv[6]
	# out_put_report_file_name = sys.argv[7]

	simulationTime_input = sys.argv[1]
	quality_input = sys.argv[2]
	
	sent_video_file_name = 'SentVideoPktInfo'
	recv_video_file_name = 'RecvVideoPktInfo'
	
	out_put_file_name = 'outputPktsInfoFile'
	out_put_latency_file_name = 'outputLatencyFile'
	out_put_PLR_file_name = 'outputPLRFile'
	out_put_report_file_name = 'outputReportFile'
	out_put_packet_info_file_name = 'outputPacketInfoFile'
	out_put_latency_frame_file_name = 'outputLatencyFrameFile'



	sent_video_file = open(sent_video_file_name, "r")
	recv_video_file = open(recv_video_file_name, "r")

	outPutFile = open(out_put_file_name, "w")
	outPutLatencyFile = open (out_put_latency_file_name, "w")
	outPutPLRFile = open (out_put_PLR_file_name, "w")
	outputReportFile = open (out_put_report_file_name, "w")
	outputPacketInfoFile = open (out_put_packet_info_file_name, "w")    #trasmited and received packet per gop
	outPutLatencyFrameFile = open (out_put_latency_frame_file_name, "w")


	simulation_time = float(simulationTime_input)

	# create a matrix same as sample    ==>  [ {'size': '215', 'sentTime': 51.6691, 'recvTime': 51.6691, 'type':'I', 'latency:0.0098, 'gop':5} , {} , {} , ... , {}]
	#  index of array shows the sequence id of the packet.
	# so, first of all ,  need to create an empty array with len as equal as the max number of sequence id in the sent video file.
	packets_info = []	

	# read sent video file 
	lines = sent_video_file.readlines()
	sent_list = [l.rstrip("\n").split() for l in lines]		# sample of index in sent_list = ['50.6681', 'id', '1638', 'udp', '1472', 'type', 'I']
	last_video_packet_id = int(sent_list[len(sent_list)-1][2])		# the last id in the file
	
	for a in range (0,last_video_packet_id+1):
		packets_info.append({'sentTime': -0.1, 'size': 0, 'recvTime': -0.1, 'type': '', 'latency':-0.1, 'gop':0, 'frameId':0 })

	for i in sent_list:		# sample of i = ['50.6681', 'id', '1638', 'udp', '1472', 'type', 'I', 'gop','12', 'frameId', '14']
		packets_info[int(i[2])]['sentTime'] = float(i[0])
		packets_info[int(i[2])]['size'] = int(i[4])
		packets_info[int(i[2])]['type'] = i[6]
		packets_info[int(i[2])]['gop'] = int(i[8])
		packets_info[int(i[2])]['frameId'] = int(i[10])
	
	# read received file 
	lines = recv_video_file.readlines()
	recv_list = [l.rstrip("\n").split() for l in lines]
	
	for i in recv_list:		# sample of i = ['50.6681', 'id', '1638', 'udp', '1472', 'latency', '0.0064']
		packets_info[int(i[2])]['recvTime'] = float(i[0])
		packets_info[int(i[2])]['latency'] = float(i[6])
	
	for j in range (len(packets_info)):
		ptk_info_str = "id " + str(j) + "\t\t" + "SentTime " + str(packets_info[j]['sentTime']) + "\t\t" + "RecvTime " + str(packets_info[j]['recvTime']) + "\t\t" + "Size " + str(packets_info[j]['size']) + "\t\t" + "Type " + packets_info[j]['type'] + "\t\t" + "latency " + str(packets_info[j]['latency']) + "\t\t" + "gop " + str(packets_info[j]['gop']) + "\t\t" + "FrameId " + str(packets_info[j]['frameId']) + "\n"
		outPutFile.write(ptk_info_str)


	#########################   calculation phase  #######################################

	numSentGOP = packets_info[last_video_packet_id]['gop'] 

	# numSentIFrames = numSentGOP 			# the number of sent I frames 
	numSentIFrames = 0 			# the number of sent I frames 
	numRecvIFrames = 0 						# the number of recv I frame tha it is recieved with all I packets of each frame
	
	numSentPkt = 0 		# no matter the type of packet I or P or B
	numRecvPkt = 0 		# no matter the type of packet I or P or B

	#Throughput : it should be calculated based on what we considered as a recv Frame at the end ( if we have just one lost packet in a frame, we considered whole frame asa lost frame)
	totalRecvByte = 0  	


	#variable for each GOP
	numSentIPackets = 0
	numRecvIPackets = 0 
	numSentPBPackets = 0
	numRecvPBPackets = 0 

	
	gop_info = []
	for a in range (numSentGOP+1):
		gop_info.append({'StartIdIpkt': 0, 'SentIpkts':0, 'RecvIpkts':0, 'SentPBpkts':0, 'RecvPBpkts':0, 'min_latency': 0.0, 
						 'max_latency': 0.0, 'ave_latency': 0.0, 'act_PLR': 222, 'BP_Frame_Loss': 0, 'recv_I_frame': 0 , 'lastIDGop':0, 
						 'B_P_Frame_sent': 0, 'Total_Recv_I_Frame_Byte': 0, 'Total_Recv_B_P_Frame_Byte': 0 , 'BP_Frame_Loss_after_correction': 0})

	outputPacketInfoFile.write("GOP,SentIpkts,SentPBpkts,RecvIpkts,RecvPBpkts,recv_I_frame,B_P_Frame_sent,BP_Frame_Loss,Act_PLR,Ave_Latency,Quality,Recv_I_Frame_Byte,Recv_PB_Frame_Byte" + "\n")

	gopNumber = 1
	pktnumLat = 0
	sumLat = 0 
	minLat = 99999
	maxLat = -1
	startIdIpkt = 1

	for i in range (1,len(packets_info)):
		if (packets_info[i]['gop'] == gopNumber+1 ):
			gop_info[gopNumber]['min_latency'] = minLat
			gop_info[gopNumber]['max_latency'] = maxLat
			if (pktnumLat != 0 ):
				gop_info[gopNumber]['ave_latency'] = sumLat/pktnumLat
			gop_info[gopNumber]['StartIdIpkt'] = startIdIpkt
			gop_info[gopNumber]['lastIDGop'] = i - 1
			gop_info[gopNumber]['SentIpkts'] = numSentIPackets
			gop_info[gopNumber]['RecvIpkts'] = numRecvIPackets
			gop_info[gopNumber]['SentPBpkts'] = numSentPBPackets
			gop_info[gopNumber]['RecvPBpkts'] = numRecvPBPackets
			gop_info[gopNumber]['act_PLR'] = int(math.ceil((float(numSentIPackets-numRecvIPackets)/float(numSentIPackets) * 100)))
			if (gop_info[gopNumber]['act_PLR'] == 0 ):
				gop_info[gopNumber]['recv_I_frame'] = 1
			startIdIpkt = i
			gopNumber +=1
			numSentIPackets = 0 
			numRecvIPackets = 0 
			numSentPBPackets = 0
			numRecvPBPackets = 0 
			pktnumLat = 0
			sumLat = 0 
			minLat = 99999
			maxLat = -1

		if (packets_info[i]['type'] == 'I'):
			if (packets_info[i]['sentTime'] != -0.1):
				numSentIPackets += 1
			if (packets_info[i]['recvTime'] != -0.1):
				sumLat += packets_info[i]['latency']
				pktnumLat +=1
				if (packets_info[i]['latency'] < minLat):
					minLat = packets_info[i]['latency']
				if (packets_info[i]['latency'] > maxLat):
					maxLat = packets_info[i]['latency']
				numRecvIPackets += 1

		elif (packets_info[i]['type'] == 'B' or packets_info[i]['type'] == 'P'):
			if (packets_info[i]['sentTime'] != -0.1):
				numSentPBPackets += 1
			if (packets_info[i]['recvTime'] != -0.1):      # b and P
				sumLat += packets_info[i]['latency']
				pktnumLat +=1
				if (packets_info[i]['latency'] < minLat):
					minLat = packets_info[i]['latency']
				if (packets_info[i]['latency'] > maxLat):
					maxLat = packets_info[i]['latency']
				numRecvPBPackets += 1

	# Generating output file with data after considering   +++++  "lost I FRAME"  +++++
	# if ( act_PLR > 0 and act_PLR < 100 )
	#	this I frame should be considered as a lost

	
	# outPutLatencyFile.write("gop,min_latency,max_latency,ave_latency\n")

	for gopNumber in range (1, len(gop_info)):
		if (simulation_time < packets_info[gop_info[gopNumber]['StartIdIpkt']]['sentTime']):
			numSentIFrames = packets_info[gop_info[gopNumber]['StartIdIpkt']]['gop'] - 1
			if (simulation_time < packets_info[gop_info[gopNumber-1]['lastIDGop']]['sentTime']):   # we want to make sure that we send all gop before accepted simulation time
				numSentIFrames = numSentIFrames -1 
			break

	numSentIFrames = numSentIFrames -1  # ignoring the first gop

	sumSentFrame = 0 
	sumRecvFrame = 0 

	for i in range (2, numSentIFrames+2):	  # 2 means to ignore the first gop
		numSetPkt_CurrerntGOP = 0 
		numRecvPkt_CurrerntGOP = 0 
		Total_Recv_I_Frame_Byte = 0 
		Total_Recv_B_P_Frame_Byte = 0 

		numSetPkt_CurrerntGOP = gop_info[i]['SentIpkts'] + gop_info[i]['SentPBpkts']
		numSentPkt = numSentPkt + numSetPkt_CurrerntGOP

		if (gop_info[i]['act_PLR'] == 0):
			numRecvIFrames +=1
			numRecvPkt_CurrerntGOP = gop_info[i]['RecvIpkts']
			for pkt in range (gop_info[i]['StartIdIpkt'],gop_info[i]['StartIdIpkt'] + gop_info[i]['RecvIpkts']):
				Total_Recv_I_Frame_Byte += packets_info[pkt]['size']
		# numRecvPkt += numRecvPkt_CurrerntGOP


		B_P_Frame_lost = 0 
		B_P_Frame_sent = 0 
		flag_loss = False
		a = gop_info[i]['StartIdIpkt'] + gop_info[i]['SentIpkts']
		while( a < gop_info[i]['lastIDGop'] ):
			# print ("a: " + str (a))
			if (packets_info[a]['type'] == 'P' or packets_info[a]['type'] == 'B'): 
				currentFrameId = packets_info[a]['frameId']
				# print ("currentFrameId   " + str(currentFrameId))
				B_P_Frame_sent +=1
				lastpacketId_currFrame = 0 
				for b in range (a, gop_info[i]['lastIDGop']+1):
					if (packets_info[b]['frameId'] != currentFrameId):
						lastpacketId_currFrame = b-1
						break
					elif (b == gop_info[i]['lastIDGop']):
						lastpacketId_currFrame = b
						break
				# print("lastpacketId_currFrame   " + str(lastpacketId_currFrame)+ "\n")
				for c in range (a, lastpacketId_currFrame+1):
					if (packets_info[c]['recvTime'] == -0.1):
						B_P_Frame_lost += 1
						flag_loss = True
						break						
				if (flag_loss == False):
					for c in range (a, lastpacketId_currFrame+1):
						Total_Recv_B_P_Frame_Byte += packets_info[c]['size']
						numRecvPkt_CurrerntGOP +=1
				a = lastpacketId_currFrame + 1
				flag_loss = False
			else:
				print("something wrong")
				exit(1)   # error , these ids should be B or P frame 

		gop_info[i]['BP_Frame_Loss'] = B_P_Frame_lost
		gop_info[i]['B_P_Frame_sent'] = B_P_Frame_sent
		gop_info[i]['Total_Recv_I_Frame_Byte'] = Total_Recv_I_Frame_Byte
		gop_info[i]['Total_Recv_B_P_Frame_Byte'] = Total_Recv_B_P_Frame_Byte

		numRecvPkt += numRecvPkt_CurrerntGOP		
		# the number of recv packet should be corrected. it is  based on the B/P frame loss after correction.
		
		if (gop_info[i]['act_PLR'] == 0):
			gop_info[i]['BP_Frame_Loss_after_correction'] = gop_info[i]['BP_Frame_Loss']
		else:
			gop_info[i]['BP_Frame_Loss_after_correction'] = gop_info[i]['B_P_Frame_sent']		# max number of P and B frame in a GOP ( 30fps --> 29 P/B frames)
			gop_info[i]['Total_Recv_B_P_Frame_Byte'] = 0


		totalRecvByte += gop_info[i]['Total_Recv_I_Frame_Byte'] + gop_info[i]['Total_Recv_B_P_Frame_Byte']

		ptk_info_str = str(i) + "," + str(gop_info[i]['min_latency']) + "," + str(gop_info[i]['max_latency']) + "," + str(gop_info[i]['ave_latency']) + "," + str(numRecvPkt_CurrerntGOP) + "\n"
		outPutLatencyFile.write(ptk_info_str)

		outPutPLRFile.write (str(i) + "," + str(gop_info[i]['act_PLR']) + "\n")

		outputPacketInfoFile.write (str(i) + "," + str(gop_info[i]['SentIpkts']) + "," + str(gop_info[i]['SentPBpkts']) + "," + str(gop_info[i]['RecvIpkts']) + "," + str(gop_info[i]['RecvPBpkts']) 
									+ "," + str(gop_info[i]['recv_I_frame']) + "," + str(gop_info[i]['B_P_Frame_sent']) + "," + str(gop_info[i]['BP_Frame_Loss']) + "," + str(gop_info[i]['act_PLR']) 
									+ "," + str(gop_info[i]['ave_latency']) + "," + str(quality_input) + "," + str(gop_info[i]['Total_Recv_I_Frame_Byte']) + "," + str(gop_info[i]['Total_Recv_B_P_Frame_Byte']) + "\n")


		# calculating the number of sent frame 
		sumSentFrame += (gop_info[i]['B_P_Frame_sent'] + 1 )  # 1 = I-frame
		
		# calculating the number of Recv frame 
		if (gop_info[i]['act_PLR'] == 0):
			sumRecvFrame += (gop_info[i]['B_P_Frame_sent'] - gop_info[i]['BP_Frame_Loss_after_correction'] + 1 )  # 1 = I-frame  && in this case, there is no difference between loss and loss_after_correction
			
	# --------------------------------------------------------------

	# latency per frame with gop info 
	curr_frameId = packets_info[1]['frameId']
	pktnumLatency = 0
	sumLatency = 0.0
	FrameLatencyArray = [0] * (packets_info[len(packets_info)-1]['frameId']+1)    
	   
	for i in range (1, len(packets_info)):
		if (packets_info[i]['frameId'] == curr_frameId + 1):
			if (pktnumLatency != 0 ):
				FrameLatencyArray[curr_frameId] = float(sumLatency/pktnumLatency)
			else:
				FrameLatencyArray[curr_frameId] = -0.1
			curr_frameId = packets_info[i]['frameId']
			sumLatency = 0.0 
			pktnumLatency = 0 
		if (packets_info[i]['recvTime'] != -0.1):
			sumLatency += packets_info[i]['latency']
			pktnumLatency +=1
	if (pktnumLatency != 0 ):			# for the last frameID
		FrameLatencyArray[curr_frameId] = float(sumLatency/pktnumLatency)
	else:
		FrameLatencyArray[curr_frameId] = -0.1 

	# ingoring the first gop 
	start_pkt_ID = gop_info[1]['lastIDGop'] + 1
	start_frame_ID = packets_info[start_pkt_ID]['frameId']

	totalSumLatency = 0 
	for i in range (start_frame_ID, curr_frameId+1):
		if (FrameLatencyArray[i] != -0.1):
			totalSumLatency += FrameLatencyArray[i]
	totalAverageLatency = float(totalSumLatency/(curr_frameId - start_frame_ID + 1))

	outPutLatencyFrameFile.write("gop,frame_id,Ave_latency\n")
	for gop in range (1, int(simulation_time) + 1):
		for j in range(gop_info[gop]['StartIdIpkt'], gop_info[gop]['lastIDGop'] + 1):
			if (j == gop_info[gop]['lastIDGop']):
				outPutLatencyFrameFile.write(str(gop) + "," + str(packets_info[j]['frameId']) + "," + str(FrameLatencyArray[packets_info[j]['frameId']]) + "\n")
			elif (packets_info[j]['frameId'] != packets_info[j+1]['frameId']):
				if (packets_info[j]['frameId'] != 0):
					outPutLatencyFrameFile.write(str(gop) + "," + str(packets_info[j]['frameId']) + "," + str(FrameLatencyArray[packets_info[j]['frameId']]) + "\n")

	#----------------------------------------------------------
	
	sumBPFrameLossAfterCorr = 0 
	sumBPFrameLoss = 0 
	sumSentBPFrame = 0 
	for i in range (2, numSentIFrames+2):
		sumBPFrameLoss += gop_info[i]['BP_Frame_Loss']
		sumBPFrameLossAfterCorr += gop_info[i]['BP_Frame_Loss_after_correction']
		sumSentBPFrame += gop_info[i]['B_P_Frame_sent']

	# ---------------------------------------------------

	# simulation_time = int(math.ceil(float(packets_info[len(packets_info)-1]['sentTime']) - float(packets_info[1]['sentTime'])))

	final_simulation_time = int(simulation_time)-1  # ignoring the first GOP

	outputReportFile.write ("Simulation_Time(s)," + str (final_simulation_time) + "\n")
	outputReportFile.write ("Sent_I_Frames," + str(numSentIFrames) + "\n")
	outputReportFile.write ("Recv_I_Frames," + str(numRecvIFrames) + "\n")
	outputReportFile.write ("I_Frame_Loss_Rate(%)," + str(float(numSentIFrames - numRecvIFrames)/float(numSentIFrames) * 100) + "\n" )
	outputReportFile.write ("Total_Sent_Packets," + str(numSentPkt) + "\n")
	outputReportFile.write ("Total_Recv_Packets," + str(numRecvPkt) + "\n")
	outputReportFile.write ("Packet_Loss_Rate(%)," + str(float(numSentPkt - numRecvPkt)/float(numSentPkt) * 100) + "\n")
	outputReportFile.write ("Throughput(Mbps)," + str(float(totalRecvByte * 8)/float(final_simulation_time * 1000000)) + "\n" )
	outputReportFile.write ("Total_Average_Latency(s)," + str(totalAverageLatency) + "\n" )
	outputReportFile.write ("BP_Frame_Loss_Rate(%)," + str(float(sumBPFrameLoss/sumSentBPFrame) * 100) + "\n" )
	outputReportFile.write ("BP_Frame_Loss_Rate_AfterCorrection(%)," + str(float(sumBPFrameLossAfterCorr/sumSentBPFrame) * 100) + "\n" )
	outputReportFile.write ("SentFrame," + str(sumSentFrame) + "\n" )
	outputReportFile.write ("RecvFrame," + str(sumRecvFrame) + "\n" )
	
# closing all files
	outPutFile.close ()
	outPutLatencyFile.close ()
	outputReportFile.close ()
	outPutPLRFile.close ()


			
			

