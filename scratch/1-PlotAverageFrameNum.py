
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


# python3 ../scratch/1-PlotAverageFrameNum.py 
# $outputputHistoryFilename""_AFEC     $outputputHistoryFilename""_NoFEC 
# a-FEC      a-NoFEC     $LocationEcho     $appThreshold  
# $staticFEC     $ErrorModel       $PropLossModel    
# $aat_factor    $Dist 

outputName = 'FrameInfo'


inputFileName_AFEC = sys.argv[1]
inputFileName_NoFEC = sys.argv[2]
appropriate_run_pattern_AFEC = sys.argv[3]
appropriate_run_pattern_NoFEC = sys.argv[4]
location_echo = sys.argv[5]
application_threashold = sys.argv[6]
static_fec = sys.argv[7]
error_model = sys.argv[8]
prop_loss_model = sys.argv[9]
aat_factor = sys.argv[10]
distance = sys.argv[11]

#  a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel
# a-NoFEC-Low-$ErrorModel-$PropLossModel

run_pattern_AFEC = appropriate_run_pattern_AFEC + "-" + str(static_fec) + "-No-" + str(error_model) + "-" + str(prop_loss_model) + "-" + str(distance) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)
run_pattern_AFEC_4k = appropriate_run_pattern_AFEC + "-" + str(static_fec) + "-High-" + str(error_model) + "-" + str(prop_loss_model) + "-" + str(distance) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)
run_pattern_AFEC_720p = appropriate_run_pattern_AFEC + "-" + str(static_fec) + "-Low-" + str(error_model) + "-" + str(prop_loss_model) + "-" + str(distance) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)
run_pattern_No_FEC_4k = appropriate_run_pattern_NoFEC + "-High-" + str(error_model) + "-" + str(prop_loss_model) + "-" + str(distance) + "-" + str(application_threashold) + "-0"
run_pattern_No_FEC_720p = appropriate_run_pattern_NoFEC + "-Low-" + str(error_model) + "-" + str(prop_loss_model) + "-" + str(distance) + "-" + str(application_threashold) + "-0"


# each line of this file shows the variables of each run
current_data_file = open(inputFileName_AFEC, "r")
lines = current_data_file.readlines()
keys = lines[0].strip().split(',')
data = []
for line in lines[1:]:
	values = line.strip().split(',')
	data.append({k: v for k, v in zip(keys, values)})

# print(data)


#each line of this file shows the variables of each run
current_data_file = open(inputFileName_NoFEC, "r")
lines = current_data_file.readlines()
keys = lines[0].strip().split(',')
# data_nofec = []
for line in lines[1:]:
	values = line.strip().split(',')
	data.append({k: v for k, v in zip(keys, values)})

x_array = []

input_sent_4k = {}
input_sent_720p = {}
input_recv_4k = {}
input_recv_720p = {}

x_array = ["AR_FEC", "No_FEC-4K", "No_FEC-720p", "AR_FEC-4K", "AR_FEC-720p"]
input_sent_4k.update({key: None for key in x_array})
input_sent_720p.update({key: None for key in x_array})
input_recv_4k.update({key: None for key in x_array})
input_recv_720p.update({key: None for key in x_array})


for run in data:
	if (run['Title'] == run_pattern_AFEC ):      #a-FEC-0-No-Table-1-dis-appThreashold-locationEcho
		input_sent_4k[x_array[0]] = float(run['FrameHighQuality'])
		input_sent_720p[x_array[0]] = float(run['FrameLowQuality'])
		input_recv_4k[x_array[0]] = float(run['RecvFrameHighQuality'])
		input_recv_720p[x_array[0]] = float(run['RecvFrameLowQuality'])
	elif (run['Title'] == run_pattern_AFEC_720p ):      #a-FEC-0-Low-Table-1-dis-appThreashold-locationEcho
		input_sent_4k[x_array[4]] = float(run['FrameHighQuality'])  #0.0
		input_sent_720p[x_array[4]] = float(run['FrameLowQuality'])  
		input_recv_4k[x_array[4]] = float(run['RecvFrameHighQuality']) #0.0
		input_recv_720p[x_array[4]] = float(run['RecvFrameLowQuality']) 
	elif (run['Title'] == run_pattern_AFEC_4k ):      #a-FEC-0-High-Table-1-dis-appThreashold-locationEcho
		input_sent_4k[x_array[3]] = float(run['FrameHighQuality'])
		input_sent_720p[x_array[3]] = float(run['FrameLowQuality'])   # 0.0
		input_recv_4k[x_array[3]] = float(run['RecvFrameHighQuality'])
		input_recv_720p[x_array[3]] = float(run['RecvFrameLowQuality'])  #0.0
	elif (run['Title'] == run_pattern_No_FEC_720p ):
		input_sent_720p[x_array[2]] = float(run['SentFrame'])
		input_recv_720p[x_array[2]] = float(run['RecvFrame'])
		input_sent_4k[x_array[2]] = 0.0 
		input_recv_4k[x_array[2]] = 0.0 
	elif (run['Title'] == run_pattern_No_FEC_4k ):
		input_sent_720p[x_array[1]] = 0.0
		input_recv_720p[x_array[1]] = 0.0
		input_sent_4k[x_array[1]] = float(run['SentFrame'])
		input_recv_4k[x_array[1]] = float(run['RecvFrame'])

sent_4k = [input_sent_4k[index] for index in x_array]
sent_720p = [input_sent_720p[index] for index in x_array]
recv_4k = [input_recv_4k[index] for index in x_array]
recv_720p = [input_recv_720p[index] for index in x_array]

# print(x_array)
# print(sent_4k)
# print(sent_720p)
# print(recv_4k)
# print(recv_720p)


# Create clustered stacked columns for sent arrays and recv arrays
fig, ax = plt.subplots(figsize=(10, 10))

# Define the width and spacing of the bars
bar_width = 0.20
spacing = 0.03

# Define the hatch patterns for the columns
# patterns = ['/', '*', '.', 'x']		
patterns = ['\\\\', '////', '----', '++', '-.']	
# colors = ['#9148C8', '#00C864', '#EF7031', '#2E5DF2', '#70818E']
colors = [ '#F286CB', '#7E81F6','#84F660', '#FFFF00', '#050643', '#D6D7E0']
#FFFF00

# Define the x-axis locations of the bars
x = np.arange(len(x_array))

# Plot the bars for the sent arrays
ax.bar(x - bar_width/2 - spacing, sent_4k, width=bar_width, hatch=patterns[0], edgecolor=colors[4], facecolor=colors[0], label='Sent-4K')
ax.bar(x - bar_width/2 - spacing, sent_720p, bottom=sent_4k, width=bar_width, hatch=patterns[1], edgecolor=colors[4], facecolor=colors[1], label='Sent-720p')

# Plot the bars for the recv arrays
ax.bar(x + bar_width/2 + spacing, recv_4k, width=bar_width, hatch=patterns[2], edgecolor=colors[4], facecolor=colors[2], label='Recv-4K')
ax.bar(x + bar_width/2 + spacing, recv_720p, bottom=recv_4k, width=bar_width, hatch=patterns[3], edgecolor=colors[4], facecolor=colors[3], label='Recv-720p')

# Set the x-axis tick labels to the Dist array
ax.set_xticks(x)
ax.set_xticklabels(x_array, fontsize=16, rotation=18)
# ax.set_xlabel('Different Mechanisms', labelpad=12, fontsize=14)

# Set the y-axis label and title
ax.set_ylabel('Number of Frames', labelpad=12, fontsize=20)
ax.set_title('Frame Statistics - ' + str(distance) + 'm - λ: ' + str(application_threashold) + 's', fontsize=24)

# Add a legend
ax.legend(fontsize=16)

plt.margins(0.1)

# Add horizontal gridlines
ax.yaxis.grid(which='major', color=colors[5], linestyle=patterns[4], linewidth=0.3)
ax.set_ylim(500, 2600)

ax.tick_params(axis='y', labelsize=18)
ax.tick_params(axis='x', labelsize=18)

# plt.show()
plt.savefig(str(distance) + 'm-' + str(application_threashold) + 's-' + outputName + '.png', format='png', transparent=False, dpi=300)

