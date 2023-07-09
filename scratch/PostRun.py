
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

	# sent_fec_file_name = sys.argv[3]
	# recv_fec_file_name = sys.argv[4]
	# recv_Iframe_file_name = sys.argv[5]
	# recv_GOP_Info_file_name = sys.argv[6]
	# simulationTime_input = sys.argv[7]

	# out_put_file_name = sys.argv[8]
	# out_put_latency_file_name = sys.argv[9]
	# out_put_PLR_file_name = sys.argv[10]
	# out_put_report_file_name = sys.argv[11]

	# SentVideoPktInfo RecvVideoPktInfo SentFECInfo RecvFECInfo CorrectedIframe GopInfoFile 72.0 outputPktsInfoFile outputLatencyFile
	
	simulationTime_input = sys.argv[1]
	
	sent_video_file_name = 'SentVideoPktInfo'
	recv_video_file_name = 'RecvVideoPktInfo'
	sent_fec_file_name = 'SentFECInfo'
	recv_fec_file_name = 'RecvFECInfo'
	recv_Iframe_file_name = 'CorrectedIframe'
	recv_GOP_Info_file_name = 'GopInfoFile'
	
	out_put_file_name = 'outputPktsInfoFile'
	out_put_latency_file_name = 'outputLatencyFile'
	out_put_PLR_file_name = 'outputPLRFile'
	out_put_report_file_name = 'outputReportFile'
	out_put_quality_file_name = 'outputQualityFile'
	out_put_packet_info_file_name = 'outputPacketInfoFile'
	out_put_latency_frame_file_name = 'outputLatencyFrameFile'
	

	sent_video_file = open(sent_video_file_name, "r")
	recv_video_file = open(recv_video_file_name, "r")

	sent_fec_file = open(sent_fec_file_name, "r")
	recv_fec_file = open(recv_fec_file_name, "r")
	recv_Iframe_file = open(recv_Iframe_file_name, "r")
	recv_GOP_Info_file = open(recv_GOP_Info_file_name, "r")

	simulation_time = float(simulationTime_input)

	outPutFile = open(out_put_file_name, "w")
	outPutLatencyFile = open (out_put_latency_file_name, "w")
	outPutPLRFile = open (out_put_PLR_file_name, "w")
	outputReportFile = open (out_put_report_file_name, "w")
	outputQualityFile = open (out_put_quality_file_name, "w")
	outputPacketInfoFile = open (out_put_packet_info_file_name, "w")    #trasmited and received packet per gop
	outPutLatencyFrameFile = open (out_put_latency_frame_file_name, "w")

	MAX_P_B_FRAME = 29 	
	numSentIFrames = 0 			#the number of sent I frames 
	numRecvIFrames = 0 						# the number of recv I frame tha it is received with all I packets of each frame

	numSentPkt = 0 		# no matter the type of packet I or FEC or P or B or Echo
	numRecvPkt = 0 		# no matter the type of packet I or FEC or P or B or Echo
	numSentFECPkt = 0 
	numRecvFECPkt = 0 
	numRecvHighQuality = 0 
	numRecvLowQuality = 0 
	numSentHighQuality = 0 
	numSentLowQuality = 0 

	#Throughput : it should be calculated based on what we considered as a recv Frame at the end ( if we have just one lost packet in a frame, we considered whole frame asa lost frame)
	totalRecvByte = 0  

	# create a matrix same as sample    ==>  [{'size': '215', 'sentTime': 51.6691, 'recvTime': 51.6691, 'type':'I', 'latency:0.0098, 'gop':5} , {} , {} , ... , {}]
	#  index of array shows the sequence id of the packet.
	# so, first of all ,  need to create an empty array with len as equal as the max number of sequence id in the sent video file.
	packets_info = []	

	# read sent video file 
	lines = sent_video_file.readlines()
	sent_list = [l.rstrip("\n").split() for l in lines]		# sample of index in sent_list = ['50.6681', 'id', '1638', 'udp', '1472', 'type', 'I']
	# print(len(sent_list))
	last_video_packet_id = int(sent_list[len(sent_list)-1][2])		# the last id in the file

	# read sent FEC file
	lines = sent_fec_file.readlines()
	sent_fec_list = [l.rstrip("\n").split() for l in lines]
	last_fec_packet_id = int(sent_fec_list[len(sent_fec_list)-1][2])		# the last id in the  fec file

	if (last_fec_packet_id > last_video_packet_id):
		packets_info_len = last_fec_packet_id
	else:
		packets_info_len = last_video_packet_id
	
	for a in range (0,packets_info_len +2):
		packets_info.append({'sentTime': -0.1, 'size': 0, 'recvTime': -0.1, 'type': '', 'latency':-0.1, 'gop': 0, 'frameId':0 })


	for i in sent_list:		# sample of i = ['50.6681', 'id', '1638', 'udp', '1472', 'type', 'I','frameId', '14']
		packets_info[int(i[2])]['sentTime'] = float(i[0])
		packets_info[int(i[2])]['size'] = int(i[4])
		packets_info[int(i[2])]['type'] = i[6]
		packets_info[int(i[2])]['frameId'] = int(i[8])
	
	# read received file 							contains packets after correction
	lines = recv_video_file.readlines()
	recv_list = [l.rstrip("\n").split() for l in lines]
	
	for i in recv_list:		# sample of i = ['50.6681', 'id', '1638', 'udp', '1472', 'latency', '0.0064', 'frameId', '14']
		packets_info[int(i[2])]['recvTime'] = float(i[0])
		packets_info[int(i[2])]['latency'] = float(i[6])
	
	# read sent FEC file
	# lines = sent_fec_file.readlines()
	# sent_fec_list = [l.rstrip("\n").split() for l in lines]

	# prev_IframeIDs = ''
	for i in sent_fec_list:     # sample of i = ['3.0814', 'id', '81', 'udp', '1446', 'IframeIDs', '79-80']
		packets_info[int(i[2])]['sentTime'] = float(i[0])
		packets_info[int(i[2])]['size'] = int(i[4])
		packets_info[int(i[2])]['type'] = 'FEC'

		# if (i[6] != prev_IframeIDs):
		# 	numSentIFrames += 1
		# 	prev_IframeIDs = i[6]
		# 	iFrameIds = i[6].split('-')
		# 	firstI = int(iFrameIds[0])
		# 	lastI = int(iFrameIds[1])
		# 	for y in range (firstI, lastI+1):
		# 		packets_info[int(y)]['type'] = 'I'
	
	# read received FEC file
	lines = recv_fec_file.readlines()
	recv_fec_file = [l.rstrip("\n").split() for l in lines]

	for i in recv_fec_file:     # sample of i = ['50.6681', 'id', '1638', 'udp', '1472']
		packets_info[int(i[2])]['recvTime'] = float(i[0])
		packets_info[int(i[2])]['latency'] = packets_info[int(i[2])]['recvTime'] - packets_info[int(i[2])]['sentTime']

	# read GOP Info file 
	lines = recv_GOP_Info_file.readlines()					
	recv_GOP_Info_file = [l.rstrip("\n").split() for l in lines]
	numSentGOP = len(recv_GOP_Info_file) - 1 				# the first line is a title
	recv_GOP_Info_file = recv_GOP_Info_file[1:]

	# index:   sample [   0   ,      1       ,    2         ,      3        ,     4    ,     5           ,    6       ,       7      ,      8       ,      9      ]
	# meaning: sample [Gop No ,  FirstSeqGOP , Actual-PLR % , SentTime Echo ,     TT   , Predicted-PLR % , Num I-Pkts , Num FEC-Pkts , seq for echo , High=1-Low=0]
	#   i =    sample ['60'   ,  '60609'     ,    '0'       , '58.6092'     , '0.0142' ,     '4'         ,    '76'    ,     '4'      ,    '60794'   ,    High     ]
	for i in recv_GOP_Info_file: 
		for j in range (int(i[1]),int(i[8])+1):
			packets_info[j]['gop'] = int(i[0])
		packets_info[int(i[8])]['type'] = 'Echo'						#adding Echo type 
		packets_info[int(i[8])]['sentTime'] = float(i[3])				#adding Echo sentTime 
		packets_info[int(i[8])]['recvTime'] = float(i[3]) + float(i[4])	#adding Echo recvTime 
		packets_info[int(i[8])]['latency'] = float(i[4])				#adding Echo latency 
		packets_info[int(i[8])]['size'] = 20							#adding Echo size ( it is same as code, you can find it in evalvid-fec-client.cc) 

	# Since we want to support sending Echo in every where on GOP, we need to run this loop for detecting rest of the packet for each GOP
	for z in range (numSentGOP):
		currGOPNum = int(recv_GOP_Info_file[z][0])
		if (z == numSentGOP - 1):
			lastID_currGOP = len(packets_info) - 1
			firstID_currGop = int(recv_GOP_Info_file[z][1])
		else:	
			lastID_currGOP = int(recv_GOP_Info_file[z+1][1]) - 1
			firstID_currGop = int(recv_GOP_Info_file[z][8]) + 1
		for y in range (firstID_currGop, lastID_currGOP + 1):
			packets_info[y]['gop'] = currGOPNum
			# print(str(currGOPNum) + "  , "  + str(int(recv_GOP_Info_file[z][8])+1) + "   ,   " + str(lastID_currGOP + 1))

			
	# writing in outputPktsInfoFile file
	for j in range(1,len(packets_info)):			# packetinfo[0] = null. because we dont have a packet with id = 0
		ptk_info_str = "id " + str(j) + "\t\t" + "SentTime " + str(packets_info[j]['sentTime']) + "\t\t" + "RecvTime " + str(packets_info[j]['recvTime']) + "\t\t" + "Size " + str(packets_info[j]['size']) + "\t\t" + "Type " + packets_info[j]['type'] + "\t\t" + "latency " + str(packets_info[j]['latency']) + "\t\t" + "gop " + str(packets_info[j]['gop']) + "\t\t" + "FrameId " + str(packets_info[j]['frameId']) + "\n"
		outPutFile.write(ptk_info_str)


	#########################   calculation phase  #######################################

	gop_info = []
	for a in range (numSentGOP+1):
		gop_info.append({'StartIdIpkt': 0,   		# first ID of I-frame packets
						 'FinishIdIpkt': 0,  		# last ID of I-frame packets
						 'SentIpkts':0,      		# number of Sent I-frame packets
						 'RecvIpkts':0,  			# number of received I-frame packets
						 'SentFECpkts':0, 			# number of Sent FEC packets
						 'RecvFECpkts':0, 			# number of received FEC packets
						 'SentPBpkts':0, 			# number of Sent B/P-frame packets
						 'RecvPBpkts':0, 			# number of received B/P-frame packets
					  	 'min_latency': 0.0, 		# min latency between all of the received VIDEO packets
						 'max_latency': 0.0, 		# max latency between all of the received VIDEO packets
						 'ave_latency': 0.0, 		# average latency between all of the received VIDEO packets
						 'act_PLR': 222, 			# Actual PLR for I-frame packets
						 'pred_PLR':222,   			# Predicted PLR for I-frame packets
						 'quality':'',   			# Quality of sent frames in this GOP
						 'lastIDGop':0,  			# last ID of this GOP
						 'correctedI':0,			# Is the I-frame of this GOP corrected?
						 'BP_Frame_Loss': 0 , 		# the number of B/P frames are considered as lost frame
						 'BP_Frame_sent': 0, 		# the number of sent B/P frames 
						 'BP_Frame_Loss_after_correction': 0, 	# if we cannot have a corrected I-frame , we considered all B/P frames as lost frames.
						 'Total_Recv_I_Frame_Byte': 0, 			# the number of bytes of received I-frame packets
						 'Total_Recv_B_P_Frame_Byte': 0, 		# the number of bytes of received B/P-frame packets after validation on having a corrected I-frame
						 'Total_Recv_FEC_Frame_Byte': 0, 		# the number of bytes of received FEC packets
						 'EchoSeqId': 0 })			# the ID of echo packet

	# index:   sample [   0   ,      1       ,    2         ,      3        ,     4    ,     5           ,    6       ,       7      ,      8       ,      9      ]
	# meaning: sample [Gop No ,  FirstSeqGOP , Actual-PLR % , SentTime Echo ,     TT   , Predicted-PLR % , Num I-Pkts , Num FEC-Pkts , seq for echo , High=1-Low=0]
	#   i =    sample ['60'   ,  '60609'     ,    '0'       , '58.6092'     , '0.0142' ,     '4'         ,    '76'    ,     '4'      ,    '60794'   ,    'High'   ]

	for i in recv_GOP_Info_file:   
		gop_info[int(i[0])]['StartIdIpkt'] = int(i[1])  
		gop_info[int(i[0])]['FinishIdIpkt'] = int(i[1]) + int(i[6]) - 1
		gop_info[int(i[0])]['SentIpkts'] = int(i[6])
		gop_info[int(i[0])]['SentFECpkts'] = int(i[7])
		# gop_info[int(i[0])]['SentPBpkts'] = int(i[8])-(int(i[1])+int(i[6])+int(i[7]))
		# gop_info[int(i[0])]['lastIDGop'] = int(i[8])
		gop_info[int(i[0])]['EchoSeqId'] = int(i[8])
		gop_info[int(i[0])]['act_PLR'] = int(i[2])
		gop_info[int(i[0])]['pred_PLR'] = int(i[5])
		gop_info[int(i[0])]['quality'] = i[9]

	# I could not rely on the seq ID of Echo for calculating SetPBpkts and lastIDGop
	# I have to consider a loop on 
	for h in range (1,numSentGOP + 1):
		if (h == numSentGOP):
			gop_info[h]['lastIDGop'] = packets_info_len				#last ID of sent packet is the last ID for last GOP
		else:
			gop_info[h]['lastIDGop'] = gop_info[h+1]['StartIdIpkt'] - 1

		counter_BP_GOP = 0 
		startID_BP = gop_info[h]['StartIdIpkt'] + gop_info[h]['SentIpkts'] + gop_info[h]['SentFECpkts']
		for g in range (startID_BP, gop_info[h]['lastIDGop']+1):
			if (packets_info[g]['type'] == "B" or packets_info[g]['type'] == "P"):		# we should consider this comparison because we have an echo packet in this period of seqID
				counter_BP_GOP += 1
		gop_info[h]['SentPBpkts'] = counter_BP_GOP


	# read corrected I frame file after correction phase				#sample: ['20.5434','IPacketsID','7587-7703']
	lines = recv_Iframe_file.readlines()
	recv_Iframe_file = [l.rstrip("\n").split() for l in lines]
	# numRecvIFrames = len(recv_Iframe_file)

	correctedIframeArray = [a[2].split('-') for a in recv_Iframe_file]
	x = 1
	for b in correctedIframeArray:
		for y in range (x,len(gop_info)):
			if ( int(b[0]) == gop_info[y]['StartIdIpkt']):
				gop_info[y]['correctedI'] = 1
				x = y + 1
				break


	for gop in range (2, len(gop_info)):  #skip the first GOP
		#variable for each GOP
		pktnumLat = 0
		sumLat = 0 
		minLat = 99999
		maxLat = -1
		numRecvIPackets = 0  
		numRecvFecPackets = 0
		Total_Recv_I_Frame_Byte = 0 
		Total_Recv_FEC_Frame_Byte = 0 

		for i in range (gop_info[gop]['StartIdIpkt'], gop_info[gop]['lastIDGop'] + 1):  # +1 because we also want to consider the last ID of this GOP
			if (packets_info[i]['type'] == 'I'):
				if (packets_info[i]['recvTime'] != -0.1):
					sumLat += packets_info[i]['latency']
					pktnumLat +=1
					if (packets_info[i]['latency'] < minLat):
						minLat = packets_info[i]['latency']
					if (packets_info[i]['latency'] > maxLat):
						maxLat = packets_info[i]['latency']
					if (gop_info[gop]['correctedI'] == 1):
						numRecvIPackets += 1
						Total_Recv_I_Frame_Byte += packets_info[i]['size']
			elif (packets_info[i]['type'] == 'FEC'):
				if (packets_info[i]['recvTime'] != -0.1):
					numRecvFecPackets += 1
					Total_Recv_FEC_Frame_Byte += packets_info[i]['size']
			elif (packets_info[i]['type'] == 'B' or packets_info[i]['type'] == 'P'):
				if (packets_info[i]['recvTime'] != -0.1):      # b and P
					sumLat += packets_info[i]['latency']
					pktnumLat +=1
					if (packets_info[i]['latency'] < minLat):
						minLat = packets_info[i]['latency']
					if (packets_info[i]['latency'] > maxLat):
						maxLat = packets_info[i]['latency']
					# numRecvPBPackets += 1					# we want to calculate numRecvPBPackets for each frame that all of its packets will be received. so we can not count in this line

		gop_info[gop]['min_latency'] = minLat
		gop_info[gop]['max_latency'] = maxLat
		if (pktnumLat != 0 ):
			gop_info[gop]['ave_latency'] = sumLat/pktnumLat

		# recv packets info
		gop_info[gop]['RecvIpkts'] = numRecvIPackets
		gop_info[gop]['RecvFECpkts'] = numRecvFecPackets
		gop_info[gop]['Total_Recv_I_Frame_Byte'] = Total_Recv_I_Frame_Byte
		gop_info[gop]['Total_Recv_FEC_Frame_Byte'] = Total_Recv_FEC_Frame_Byte


	# outPutLatencyFile.write("gop,min_latency,max_latency,ave_latency\n")

	for gopNumber in range (1, len(gop_info)): 
		if (simulation_time < packets_info[gop_info[gopNumber]['StartIdIpkt']]['sentTime']):
			numSentIFrames = packets_info[gop_info[gopNumber]['StartIdIpkt']]['gop'] - 1
			if (simulation_time < packets_info[gop_info[gopNumber-1]['lastIDGop']]['sentTime']):   # we want to make sure that we send all gop before accepted simulation time
				numSentIFrames = numSentIFrames -1 
			break

	numSentIFrames = numSentIFrames -1  # ignoring the first gop

	outputPacketInfoFile.write("GOP,SentIpkts,SentPBpkts,RecvIpkts,RecvPBpkts,SentFECpkts,RecvFECpkts,CorrectedI,BP_Frame_sent,BP_Frame_Loss,BP_Frame_Loss_AfterCorrection,Act_PLR,Pred_PLR,Ave_Latency,Quality,Recv_I_Frame_Byte,Recv_FEC_Frame_Byte,Recv_PB_Frame_Byte" + "\n")

	numSentIFrames = int(simulation_time) - 1    # because we ignore the first GOP 
	sumHighFrames = 0
	sumLowFrames = 0 
	sumRecvHighFrame = 0 
	sumRecvLowFrame = 0 

	for i in range (2, numSentIFrames+2):
		numRecvPBPackets = 0 

		# total sent packets
		numSentPkt_CurrerntGOP = gop_info[i]['SentIpkts'] + gop_info[i]['SentPBpkts'] 
		numSentPkt += numSentPkt_CurrerntGOP + gop_info[i]['SentFECpkts']       #  + 1		# 1 = echo packet

		numRecvPkt_CurrerntGOP = gop_info[i]['RecvIpkts'] 
		# numRecvPkt = numRecvPkt + numRecvPkt_CurrerntGOP + gop_info[i]['RecvFECpkts'] 
		# if (packets_info[gop_info[i]['lastIDGop']]['type'] == 'Echo' and packets_info[gop_info[i]['lastIDGop']]['recvTime'] != -0.1):
		# 	numRecvPkt += 1
		
		# pktRatio = (float(numRecvPkt_CurrerntGOP / numSentPkt_CurrerntGOP) * 100 )

		# for z in range (gop_info[i]['StartIdIpkt'], gop_info[i]['lastIDGop']):   # didn't consider echo packet
		# 	if (packets_info[z]['recvTime'] != -0.1): 
		# 		totalRecvByte += packets_info[z]['size']
		
		numSentFECPkt += gop_info[i]['SentFECpkts']
		numRecvFECPkt += gop_info[i]['RecvFECpkts'] 

		if( gop_info[i]['correctedI'] == 1 ):
			numRecvIFrames += 1		
			if (gop_info[i]['quality'] == 'High'):
				numRecvHighQuality += 1
				# print(str(i) + str (" : high= ") + str(numRecvHighQuality))
			elif (gop_info[i]['quality'] == 'Low'):
				numRecvLowQuality += 1
				# print(str(i) + str (" : low= ") + str(numRecvLowQuality))

		if (gop_info[i]['quality'] == 'High'):
			numSentHighQuality += 1
		elif (gop_info[i]['quality'] == 'Low'):
			numSentLowQuality += 1

		# calculating B/P-frames ( send / received) 
		# if we lost one of the packet of one B/P frame , we consider this frame as a lost frame
		BP_Frame_lost = 0 
		BP_Frame_sent = 0 
		Total_Recv_B_P_Frame_Byte = 0 
		a = gop_info[i]['FinishIdIpkt'] + gop_info[i]['SentFECpkts'] + 1		# first Id after FEC packets
		flag_loss = False
		while( a < gop_info[i]['lastIDGop'] ):
			if (packets_info[a]['type'] == 'P' or packets_info[a]['type'] == 'B'): 
				currentFrameId = packets_info[a]['frameId']
				BP_Frame_sent +=1
				lastpacketId_currFrame = 0 
				for b in range (a, gop_info[i]['lastIDGop']+1):
					if (packets_info[b]['frameId'] != currentFrameId):		# b = first id of next frame
						lastpacketId_currFrame = b-1
						break
					elif (b == gop_info[i]['lastIDGop']):			# b = last id of current frame and last id of current GOP
						lastpacketId_currFrame = b
						break
				for c in range (a, lastpacketId_currFrame+1):
					if (packets_info[c]['recvTime'] == -0.1):
						BP_Frame_lost += 1				
						flag_loss = True
						break						
				if (flag_loss == False):
					for c in range (a, lastpacketId_currFrame+1):
						Total_Recv_B_P_Frame_Byte += packets_info[c]['size']
						numRecvPBPackets += 1
						# numRecvPkt_CurrerntGOP +=1
				a = lastpacketId_currFrame + 1
				flag_loss = False
	
			elif (packets_info[a]['type'] == 'Echo'):
				a += 1
			else :
				print("something wrong , these ids should be B or P frame or Echo packet: type: " + str(packets_info[a]['type']) + " , id : "+ str(a) + "\n")
				exit(1)   # error , these ids should be B or P frame 
		gop_info[i]['BP_Frame_Loss'] = BP_Frame_lost
		gop_info[i]['BP_Frame_sent'] = BP_Frame_sent
		gop_info[i]['Total_Recv_B_P_Frame_Byte'] = Total_Recv_B_P_Frame_Byte
		gop_info[i]['RecvPBpkts'] = numRecvPBPackets


		# we  want to consider B and P frames as lost if we can not recover/deliver I frame becasue delivering B/P frames without I frame is not reasonable.
		if( gop_info[i]['correctedI'] == 1 ):
			gop_info[i]['BP_Frame_Loss_after_correction'] = gop_info[i]['BP_Frame_Loss']
		else:
			gop_info[i]['BP_Frame_Loss_after_correction'] = gop_info[i]['BP_Frame_sent']		# max number of P and B frame in a GOP ( 30fps --> 29 P/B frames)
			gop_info[i]['Total_Recv_B_P_Frame_Byte'] = 0								# this variable contain the number of B/P_Recv_byte after calculation B/P frmae loss
			gop_info[i]['RecvPBpkts'] = 0 


		numRecvPkt_CurrerntGOP += gop_info[i]['RecvPBpkts']

		numRecvPkt += numRecvPkt_CurrerntGOP + gop_info[i]['RecvFECpkts'] 

		totalRecvByte += gop_info[i]['Total_Recv_I_Frame_Byte'] + gop_info[i]['Total_Recv_B_P_Frame_Byte']      #  + gop_info[i]['Total_Recv_FEC_Frame_Byte']

		ptk_info_str = str(i) + "," + str(gop_info[i]['min_latency']) + "," + str(gop_info[i]['max_latency']) + "," + str(gop_info[i]['ave_latency']) + "," + str(numRecvPkt_CurrerntGOP) +"\n"
		outPutLatencyFile.write(ptk_info_str)
		
		outPutPLRFile.write (str(i) + "," + str(gop_info[i]['act_PLR']) + "," + str(gop_info[i]['pred_PLR']) + "\n")
		outputQualityFile.write (str(i) + "," + str(gop_info[i]['quality']) + "\n")
		outputPacketInfoFile.write (str(i) + "," + str(gop_info[i]['SentIpkts']) + "," + str(gop_info[i]['SentPBpkts']) + "," + str(gop_info[i]['RecvIpkts']) + "," + str(gop_info[i]['RecvPBpkts']) 
									+ "," + str(gop_info[i]['SentFECpkts']) + "," + str(gop_info[i]['RecvFECpkts']) + "," + str(gop_info[i]['correctedI']) + "," + str(gop_info[i]['BP_Frame_sent'])
									+ "," + str(gop_info[i]['BP_Frame_Loss']) + "," + str(gop_info[i]['BP_Frame_Loss_after_correction']) + "," + str(gop_info[i]['act_PLR']) + "," + str(gop_info[i]['pred_PLR']) + "," + str(gop_info[i]['ave_latency'])
									+ "," + str(gop_info[i]['quality']) + "," + str(gop_info[i]['Total_Recv_I_Frame_Byte']) + "," + str(gop_info[i]['Total_Recv_FEC_Frame_Byte']) 
									+ "," + str(gop_info[i]['Total_Recv_B_P_Frame_Byte']) + "\n")

		# calculating the number of sent frame in different qualities
		if (gop_info[i]['quality'] == 'High'):
			sumHighFrames += (gop_info[i]['BP_Frame_sent'] + 1 )  # 1 = I-frame
		elif (gop_info[i]['quality'] == 'Low'):
			sumLowFrames += (gop_info[i]['BP_Frame_sent'] + 1 )  # 1 = I-frame

		# calculating the number of Recv frame in different qualities
		if (gop_info[i]['correctedI'] == 1):
			if (gop_info[i]['quality'] == 'High'):
				sumRecvHighFrame += (gop_info[i]['BP_Frame_sent'] - gop_info[i]['BP_Frame_Loss_after_correction'] + 1 )  # 1 = I-frame  && in this case, there is no difference between loss and loss_after_correction
			elif (gop_info[i]['quality'] == 'Low'):
				sumRecvLowFrame += (gop_info[i]['BP_Frame_sent'] - gop_info[i]['BP_Frame_Loss_after_correction'] + 1 )  # 1 = I-frame  && in this case, there is no difference between loss and loss_after_correction

	#----------------------------------------------------------

	# latency per frame with gop info 
	curr_frameId = packets_info[1]['frameId']
	pktnumLatency = 0
	sumLatency = 0.0
	FrameLatencyArray = [0] * 5000       #   (packets_info[len_packet_info_index]['frameId']+1)  bazi vaghta nemishe chon packet akhar momkene FEC ya echo bashe ke bedoone frameID hastand
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

	totalSumLatency = 0 
	for i in range (1, curr_frameId+1):
		if (FrameLatencyArray[i] != -0.1):
			totalSumLatency += FrameLatencyArray[i]
	totalAverageLatency = float(totalSumLatency/curr_frameId)

	outPutLatencyFrameFile.write("gop,frame_id,Ave_latency\n")
	for gop in range (1, int(simulation_time) + 1):
		for j in range(gop_info[gop]['StartIdIpkt'], gop_info[gop]['lastIDGop'] + 1):
			if (j == gop_info[gop]['lastIDGop']):
				outPutLatencyFrameFile.write(str(gop) + "," + str(packets_info[j]['frameId']) + "," + str(FrameLatencyArray[packets_info[j]['frameId']]) + "\n")
			elif (packets_info[j]['frameId'] != packets_info[j+1]['frameId']):
				if (packets_info[j]['frameId'] != 0):
					outPutLatencyFrameFile.write(str(gop) + "," + str(packets_info[j]['frameId']) + "," + str(FrameLatencyArray[packets_info[j]['frameId']]) + "\n")

	# ----------------------------------------------
	
	sumBPFrameLossAfterCorr = 0 
	sumBPFrameLoss = 0 
	sumSentBPFrame = 0 
	for i in range (2, int(simulation_time)+1):
		sumBPFrameLoss += gop_info[i]['BP_Frame_Loss']
		sumBPFrameLossAfterCorr += gop_info[i]['BP_Frame_Loss_after_correction']
		sumSentBPFrame += gop_info[i]['BP_Frame_sent']

	# simulation_time = int(math.ceil(float(packets_info[len(packets_info)-1]['sentTime']) - float(packets_info[1]['sentTime'])))

	final_simulation_time = int(simulation_time)-1  # ignoring the first GOP

	outputReportFile.write ("Simulation_Time(s)," + str (final_simulation_time) + "\n")
	outputReportFile.write ("Sent_I_Frames," + str(numSentIFrames) + "\n")
	outputReportFile.write ("Recv_I_Frames," + str(numRecvIFrames) + "\n")
	outputReportFile.write ("I_Frame_Loss_Rate(%)," + str(float(numSentIFrames - numRecvIFrames)/float(numSentIFrames) * 100) + "\n" )
	outputReportFile.write ("Sent_I_Frames-HighQuality," + str(numSentHighQuality) + "\n")
	outputReportFile.write ("Sent_I_Frames-LowQuality," + str(numSentLowQuality) + "\n")
	outputReportFile.write ("Recv_I_Frames-HighQuality," + str(numRecvHighQuality) + "\n")
	outputReportFile.write ("Recv_I_Frames-LowQuality," + str(numRecvLowQuality) + "\n")
	outputReportFile.write ("Total_Sent_Packets," + str(numSentPkt) + "\n")
	outputReportFile.write ("Total_Recv_Packets," + str(numRecvPkt) + "\n")
	outputReportFile.write ("Packet_Loss_Rate(%)," + str(float(numSentPkt - numRecvPkt)/float(numSentPkt) * 100) + "\n")
	outputReportFile.write ("Total_Sent_FEC_Packets," + str(numSentFECPkt) + "\n")
	outputReportFile.write ("Total_Recv_FEC_Packets," + str(numRecvFECPkt) + "\n")
	outputReportFile.write ("Sent_FEC_packets/Sent_Total_packets(%)," + str(float((float(numSentFECPkt)/float(numSentPkt))*100)) + "\n")
	outputReportFile.write ("Recv_FEC_packets/Recv_Total_packets(%)," + str(float((float(numRecvFECPkt)/float(numRecvPkt))*100)) + "\n")
	outputReportFile.write ("Throughput(Mbps)," + str(float(totalRecvByte * 8)/float(final_simulation_time * 1000000)) + "\n" )
	outputReportFile.write ("Total_Average_Latency(s)," + str(totalAverageLatency) + "\n" )
	outputReportFile.write ("BP_Frame_Loss_Rate(%)," + str(float(sumBPFrameLoss/sumSentBPFrame) * 100) + "\n" )
	outputReportFile.write ("BP_Frame_Loss_Rate_AfterCorrection(%)," + str(float(sumBPFrameLossAfterCorr/sumSentBPFrame) * 100) + "\n" )
	outputReportFile.write ("FrameHighQuality," + str(sumHighFrames) + "\n" )
	outputReportFile.write ("FrameLowQuality," + str(sumLowFrames) + "\n" )
	outputReportFile.write ("RecvFrameHighQuality," + str(sumRecvHighFrame) + "\n" )
	outputReportFile.write ("RecvFrameLowQuality," + str(sumRecvLowFrame) + "\n" )

	

	outPutFile.close ()
	outPutLatencyFile.close ()
	outputReportFile.close ()
	outPutPLRFile.close ()
	outPutLatencyFrameFile.close ()


