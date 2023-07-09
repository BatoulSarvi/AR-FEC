
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

import matplotlib.pyplot as plt
import numpy as np
import sys
# from tabulate import tabulate

# python3 ../scratch/PlotThroughput-additiveAFEC.py 
# $outputputHistoryFilename""_AFEC     $outputputHistoryFilename""_NoFEC 
# a-FEC-$staticFEC-No-$ErrorModel-$PropLossModel    $LocationEcho     $appThreshold  
# a-NoFEC-Low-$ErrorModel-$PropLossModel     a-NoFEC-High-$ErrorModel-$PropLossModel 
# a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel  a-FEC-$staticFEC-High-$ErrorModel-$PropLossModel
# $aat_factor    "${Dist[@]}" 

outputName_throughput = 'Throghput'
outputName_FEC = 'FECPacketRate.jpg'
# outputName_Throu_FEC = 'ThroghputANDFecPacketRate.jpg'

inputFileName_AFEC = sys.argv[1]
inputFileName_NoFEC = sys.argv[2]
appropriate_run_pattern_AFEC = sys.argv[3]
location_echo = sys.argv[4]
application_threashold = sys.argv[5]
appropriate_run_pattern_NoFEC_Low = sys.argv[6]
appropriate_run_pattern_NoFEC_High= sys.argv[7]
appropriate_run_pattern_AFEC_Low = sys.argv[8]
appropriate_run_pattern_AFEC_High= sys.argv[9]
aat_factor = sys.argv[10]
distance_array = sys.argv[11:]

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

input_afec = []
input_nofec_4k = []
input_nofec_720p = []
input_afec_fecPacketRate =[]
input_afec_low = []
input_afec_fecPacketRate_low =[]
input_afec_high = []
input_afec_fecPacketRate_high =[]


for dist in distance_array:
	x_array.append(dist)
	# print(len(data))
	for run in data:
		if (run['Title'] == appropriate_run_pattern_AFEC + "-" + str(dist) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor) ):      #a-FEC-0-No-Table-1-dis-appThreashold-locationEcho
			input_afec.append(float(run['Throughput(Mbps)']))
			input_afec_fecPacketRate.append(float(run['Sent_FEC_packets/Sent_Total_packets(%)']))
		elif (run['Title'] == appropriate_run_pattern_AFEC_Low + "-" + str(dist) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)):      #a-FEC-0-No-Table-1-dis-appThreashold-locationEcho
			input_afec_low.append(float(run['Throughput(Mbps)']))
			input_afec_fecPacketRate_low.append(float(run['Sent_FEC_packets/Sent_Total_packets(%)']))
		elif (run['Title'] == appropriate_run_pattern_AFEC_High + "-" + str(dist) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)):      #a-FEC-0-No-Table-1-dis-appThreashold-locationEcho
			input_afec_high.append(float(run['Throughput(Mbps)']))
			input_afec_fecPacketRate_high.append(float(run['Sent_FEC_packets/Sent_Total_packets(%)']))
		elif (run['Title'] == appropriate_run_pattern_NoFEC_Low + "-" + str(dist) + "-" + str(application_threashold) + "-0"):
			input_nofec_720p.append(float(run['Throughput(Mbps)']))
		elif (run['Title'] == appropriate_run_pattern_NoFEC_High + "-" + str(dist) + "-" + str(application_threashold) + "-0"):
			input_nofec_4k.append(float(run['Throughput(Mbps)']))

#---------------------------------------------------------------

# Throughput data
distance = x_array
nofec_720p = input_nofec_720p
nofec_4k = input_nofec_4k
afec = input_afec
afec_low = input_afec_low
afec_high = input_afec_high

# Define the hatch patterns for the columns
patterns = ['/', '*', '.', 'x']

# Create the figure and subplots
fig, ax1 = plt.subplots(figsize=(6, 6))

# Create the first plot
ax1.plot(distance, nofec_720p, label='NoFEC-720p', marker='o', linestyle='--', markersize=7, color='orange')
ax1.plot(distance, nofec_4k, label='NoFEC-4K', marker='d', linestyle='-.', markersize=7, color='green')
ax1.plot(distance, afec, label='AR_FEC', marker='s', linestyle=':', markersize=7, color='blue')
ax1.plot(distance, afec_high, label='AR_FEC-4K', marker='p', linestyle=':', markersize=7, color='black')
ax1.plot(distance, afec_low, label='AR_FEC-720p', marker='D', linestyle=':', markersize=7, color='cyan')
ax1.set_ylabel('Throughput(Mbps)', labelpad=10, fontsize=12)
ax1.set_xlabel('Max Distance between Sender and Receiver (m)', labelpad=10, fontsize=12)
ax1.set_title('Throughput - λ: ' + str(application_threashold) + ' s', fontsize=16)
ax1.legend(loc='best')
# Set the x-axis limits to create space on the left side
# Alternatively, you can use the maximum value of distance plus some extra padding as the right limit
right_limit = 0 + 2.3  # add 100 as padding
ax1.set_xlim(left=-0.3, right=right_limit)

# Add horizontal gridlines
ax1.yaxis.grid(which='major', color='lightgray', linestyle='-', linewidth=0.3)

ax1.tick_params(axis='y', labelsize=13)
ax1.tick_params(axis='x', labelsize=12)
ax1.set_ybound(lower=0)

fig.subplots_adjust(wspace=0.3)
# Display the chart
# plt.show()
plt.savefig(outputName_throughput + '.png', format='png', transparent=False, dpi=300)

# -------------------------------------------------------------------------------

# FEC packets rate data
distance = x_array
afec_fec_packets_rate = input_afec_fecPacketRate
afec_low_fec_packets_rate = input_afec_fecPacketRate_low
afec_high_fec_packets_rate = input_afec_fecPacketRate_high

out_put_file_name = 'outputfile_FEC_rate_All'
out_put_file_txt = open(out_put_file_name, "w")
out_put_file_txt.write(','.join(['Title'] + x_array) + "\n")
out_put_file_txt.write(','.join(['afec'] + list(map(str, afec_fec_packets_rate))) + "\n")
out_put_file_txt.write(','.join(['afec_low'] + list(map(str, afec_low_fec_packets_rate))) + "\n")
out_put_file_txt.write(','.join(['afec_high'] + list(map(str, afec_high_fec_packets_rate))) + "\n")


# Define the width and spacing of the bars
bar_width = 0.25
spacing = 0.04

# Define the x-axis locations of the bars
x = np.arange(len(distance))

# Define the hatch patterns for the columns
patterns = ['/', '*', '.', 'x']

# Create the figure and axis
fig, ax = plt.subplots(figsize=(7, 5))

# Plot the bars
ax.bar(x - bar_width - spacing, afec_low_fec_packets_rate, width=bar_width, hatch=patterns[3], edgecolor='k', facecolor='cyan', label='AR_FEC-720p')
ax.bar(x, afec_fec_packets_rate, width=bar_width, hatch=patterns[0], edgecolor='k', facecolor='Blue', label='AR_FEC' )
ax.bar(x + bar_width + spacing, afec_high_fec_packets_rate, width=bar_width, hatch=patterns[2], edgecolor='k', facecolor='white', label='AR_FEC-4K' )

# Set the axis labels, title, and limits
ax.set_ylabel('FEC Packets Rate (%)', labelpad=10)
ax.set_xlabel('Max Distance between Sender and Receiver (m)', labelpad=10, fontsize='small')
ax.set_title('FEC Packets Rate in AFEC mechanism',  fontsize='small')
ax.set_xlim(left=-0.5, right=len(distance)-1+0.5)
ax.set_xticks(x)
ax.set_xticklabels(distance)

# Add the legend and grid
ax.legend(loc='best', fontsize='small')
ax.yaxis.grid(which='major', color='lightgray', linestyle='-', linewidth=0.3)

# Save the chart
plt.savefig(outputName_FEC, dpi=300)

# # ----------------

# # FEC packets rate data
# distance = x_array
# afec_fec_packets_rate = input_afec_fecPacketRate
# afec_low_fec_packets_rate = input_afec_fecPacketRate_low
# afec_high_fec_packets_rate = input_afec_fecPacketRate_high

# # Organize data into a list of lists
# table_data = [
#     ['Distance', 'AR_FEC-720p', 'AR_FEC', 'AR_FEC-4K']
# ]

# for i in range(len(distance)):
#     # row = [distance[i], afec_low_fec_packets_rate[i], afec_fec_packets_rate[i], afec_high_fec_packets_rate[i]]
#     row = [distance[i], round(afec_low_fec_packets_rate[i], 2), round(afec_fec_packets_rate[i], 2), round(afec_high_fec_packets_rate[i], 2)]
#     table_data.append(row)
    

# # Create the figure and axis
# fig, ax = plt.subplots()

# # Hide axis and grid lines
# ax.axis('off')
# ax.axis('tight')

# # Create the table and set the cell colors
# table = ax.table(cellText=table_data, colLabels=None, cellLoc='center', loc='center')
# for i in range(len(table_data)):
#     for j in range(len(table_data[i])):
#         if i < 1:
#             # Set the header row and separator row colors
#             table[i, j].set_facecolor('lightcyan')
#         elif j == 0:
#             # Set the first column color
#             table[i, j].set_facecolor('white')
#         else:
#             # Set the data cell colors
#             table[i, j].set_facecolor('white')

# # Save the table as an image
# plt.savefig('table_fec.jpg', dpi=300)


# # --------------------------------------------------------------

# # Throughput data
# distance = x_array
# nofec_720p = input_nofec_720p
# nofec_4k = input_nofec_4k
# afec = input_afec
# afec_low = input_afec_low
# afec_high = input_afec_high

# # FEC packets rate data
# afec_fec_packets_rate = input_afec_fecPacketRate
# afec_low_fec_packets_rate = input_afec_fecPacketRate_low
# afec_high_fec_packets_rate = input_afec_fecPacketRate_high

# # Define the width and spacing of the bars
# bar_width = 0.25
# spacing = 0.04

# # Define the x-axis locations of the bars
# x = np.arange(len(distance))

# # Define the hatch patterns for the columns
# patterns = ['/', '*', '.', 'x']

# # Create the figure and subplots
# fig3, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(8, 5))

# # Create the first plot
# ax1.plot(distance, nofec_720p, label='NoFEC-(720p)', marker='o', linestyle='--', markersize=7, color='orange')
# ax1.plot(distance, nofec_4k, label='NoFEC-(4K)', marker='d', linestyle='-.', markersize=7, color='green')
# ax1.plot(distance, afec, label='AR_FEC', marker='s', linestyle=':', markersize=7, color='blue')
# ax1.plot(distance, afec_high, label='AR_FEC-4K', marker='p', linestyle=':', markersize=7, color='black')
# ax1.plot(distance, afec_low, label='AR_FEC-720p', marker='D', linestyle=':', markersize=7, color='cyan')
# ax1.set_ylabel('Throughput (Mbps)', labelpad=10)
# ax1.set_xlabel('Max Distance between Sender and Receiver (m)', labelpad=10, fontsize='small')
# ax1.set_title('Throughput',  fontsize='small')
# ax1.legend(loc='best', fontsize='small')
# # Set the x-axis limits to create space on the left side
# # Alternatively, you can use the maximum value of distance plus some extra padding as the right limit
# right_limit = 0 + 2.3  # add 100 as padding
# ax1.set_xlim(left=-0.3, right=right_limit)

# # Add horizontal gridlines
# ax1.yaxis.grid(which='major', color='lightgray', linestyle='-', linewidth=0.3)

# # Create the second plot
# ax2.bar(x - bar_width - spacing, afec_low_fec_packets_rate, width=bar_width, hatch=patterns[3], edgecolor='k', facecolor='cyan', label='AR_FEC-720p')
# ax2.bar(x, afec_fec_packets_rate, width=bar_width, hatch=patterns[0], edgecolor='k', facecolor='Blue', label='AR_FEC' )
# ax2.bar(x + bar_width + spacing, afec_high_fec_packets_rate, width=bar_width, hatch=patterns[2], edgecolor='k', facecolor='white', label='AR_FEC-4K' )
# ax2.set_ylabel('FEC Packets Rate(%)', labelpad=10)
# ax2.set_xlabel('Max Distance between Sender and Receiver (m)', labelpad=10, fontsize='small')
# ax2.set_title('FEC Packets Rate in AR_FEC mechanism',  fontsize='small')
# ax2.set_xlim(left=-0.5, right=len(distance)-1+0.5)
# ax2.legend(loc='best', fontsize='small')
# ax2.yaxis.grid(which='major', color='lightgray', linestyle='-', linewidth=0.3)
# ax2.set_xticks(x)
# ax2.set_xticklabels(distance)

# # Adjust the spacing between the subplots
# # fig.subplots_adjust(hspace=0.4)
# fig3.subplots_adjust(wspace=0.45)
# plt.xticks(x)
# # plt.margins(0.35)

# # Display the chart
# # plt.show()
# plt.savefig(outputName_Throu_FEC, dpi=300)
