
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

#if __name__ == '__main__':
	
# python3 ../scratch/PlotAverageFrameLatency.py 
# $outputputHistoryFilename""_AFEC     $outputputHistoryFilename""_NoFEC 
# a-FEC-$staticFEC-No-$ErrorModel-$PropLossModel    $LocationEcho     $appThreshold  
# a-NoFEC-Low-$ErrorModel-$PropLossModel     a-NoFEC-High-$ErrorModel-$PropLossModel 
# a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel     a-FEC-$staticFEC-High-$ErrorModel-$PropLossModel
# $aat_factor  "${Dist[@]}" 

outputName = 'AverageFrameLatency'
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

#each line of this file shows the variables of each run
current_data_file = open(inputFileName_AFEC, "r")
lines = current_data_file.readlines()
keys = lines[0].strip().split(',')
data = []
for line in lines[1:]:
	values = line.strip().split(',')
	data.append({k: v for k, v in zip(keys, values)})

#each line of this file shows the variables of each run
current_data_file = open(inputFileName_NoFEC, "r")
lines = current_data_file.readlines()
keys = lines[0].strip().split(',')
# data_nofec = []
for line in lines[1:]:
	values = line.strip().split(',')
	data.append({k: v for k, v in zip(keys, values)})

x_array = []
y_afec = []
y_afec_err = []
y_nofec_low = []
y_nofec_low_err = []
y_nofec_high = []
y_nofec_high_err = []
y_afec_low = []
y_afec_low_err = []
y_afec_high = []
y_afec_high_err = []

for dist in distance_array:
	x_array.append(dist)
	# print(len(data))
	for run in data:
		if (run['Title'] == appropriate_run_pattern_AFEC + "-" + str(dist) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)):      #a-FEC-0-No-Table-1-dis-appThreashold-locationEcho
			y_afec.append(float(run['Total_Average_Latency(s)']))
			y_afec_err.append(float(run['std_dev_latency(s)']))
		elif (run['Title'] == appropriate_run_pattern_AFEC_Low + "-" + str(dist) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)):
			y_afec_low.append(float(run['Total_Average_Latency(s)']))
			y_afec_low_err.append(float(run['std_dev_latency(s)']))
		elif (run['Title'] == appropriate_run_pattern_AFEC_High + "-" + str(dist) + "-" + str(application_threashold) + "-" + str(location_echo) + "-" + str(aat_factor)):
			y_afec_high.append(float(run['Total_Average_Latency(s)']))
			y_afec_high_err.append(float(run['std_dev_latency(s)']))
		elif (run['Title'] == appropriate_run_pattern_NoFEC_Low + "-" + str(dist) + "-" + str(application_threashold) + "-0"):
			y_nofec_low.append(float(run['Total_Average_Latency(s)']))
			y_nofec_low_err.append(float(run['std_dev_latency(s)']))
		elif (run['Title'] == appropriate_run_pattern_NoFEC_High + "-" + str(dist) + "-" + str(application_threashold) + "-0"):
			y_nofec_high.append(float(run['Total_Average_Latency(s)']))
			y_nofec_high_err.append(float(run['std_dev_latency(s)']))


x = np.array(x_array)

y = np.array(y_afec)
y_err = np.array(y_afec_err)

y1 = np.array(y_nofec_low)
y1_err = np.array(y_nofec_low_err)
y2 = np.array(y_nofec_high)
y2_err = np.array(y_nofec_high_err)

y3 = np.array(y_afec_low)
y3_err = np.array(y_afec_low_err)
y4 = np.array(y_afec_high)
y4_err = np.array(y_afec_high_err)

# print (x_array)
# print (y_afec)
# print (y_afec_err)
# print (y_nofec_low)
# print (y_nofec_low_err)
# print (y_nofec_high)
# print (y_nofec_high_err)
# print (y_afec_low)
# print (y_afec_low_err)
# print (y_afec_high)
# print (y_afec_high_err)

# Create a figure and axis object
fig, ax = plt.subplots(figsize=(6, 5))

# Create the error bar plot for data set 1
ax.errorbar(x, y, yerr=y_err, fmt='s:', capsize=5, label='AR_FEC', color='Blue', markerfacecolor='white')

# Create the error bar plot for data set 2
ax.errorbar(x, y1, yerr=y1_err, fmt='o--', capsize=5, label='NoFEC-720p', color='red', markerfacecolor='white')

# Create the error bar plot for data set 3
ax.errorbar(x, y2, yerr=y2_err, fmt='p-.', capsize=5, label='NoFEC-4K', color='green', markerfacecolor='white')

# Create the error bar plot for data set 4
ax.errorbar(x, y3, yerr=y3_err, fmt='>--', capsize=5, label='AR_FEC-720p', color='orange', markerfacecolor='white')

# Create the error bar plot for data set 5
ax.errorbar(x, y4, yerr=y4_err, fmt='d-.', capsize=5, label='AR_FEC-4K', color='cyan', markerfacecolor='white')

# Set the axis labels and title
ax.set_xlabel('Maximum distance between sender and receiver (m)', labelpad=10, fontsize=11)
ax.set_ylabel('Average Frame Latency (s)', labelpad=10, fontsize=12)
ax.set_title('Average Frame Latency - λ: ' + str(application_threashold) + 's', fontsize=14)

# Remove the x-axis ticks that don't have corresponding value in y-axis
plt.xticks(x)
ax.tick_params(axis='y', labelsize=11)
ax.tick_params(axis='x', labelsize=12)
plt.margins(0.20)
plt.subplots_adjust(bottom=0.15)
ax.set_ylim(bottom=0)

# Add a legend to the plot
ax.legend()

# Add horizontal gridlines
ax.yaxis.grid(which='major', color='lightgray', linestyle='-.', linewidth=0.3)

# Show the plot
# plt.show()
plt.savefig(outputName + '.png', format='png', transparent=False, dpi=300)
