
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

# import matplotlib.pyplot as plt
# import numpy as np

# from tokenize import Double

# import matplotlib as mpl
# import matplotlib.patches as patches
# from matplotlib import pyplot as plt

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


if __name__ == '__main__':
	# PLR_file_name = sys.argv[1]
	report_file_name = 'outputReportFile'
	# num_GOP = int(sys.argv[2])
	# outputName = sys.argv[2]

	columnvalue = sys.argv[1]		# == 400m-FEC     or 300m-NoFEC
	
	outputName = 'ReportTable-' + str(columnvalue)+ '.png'

	report_file = open(report_file_name, "r")

	lines = report_file.readlines()
	report_list = [l.rstrip("\n").split(',') for l in lines]       

############################################

	# "Simulation_Time(s)"
	# "Sent_I_Frames," +
	# "Recv_I_Frames," +
	# "I_Frame_Loss_Rate(%),"
	# "Sent_I_Frames-HighQuality,"
	# "Sent_I_Frames-LowQuality,"
	# "Recv_I_Frames-HighQuality,"
	# "Recv_I_Frames-LowQuality,"
	# "Total_Sent_Packets,"
	# "Total_Recv_Packets,"
	# "Packet_Loss_Rate(%),"
	# "Total_Sent_FEC_Packets,"
	# "Total_Recv_FEC_Packets,"
	# "Sent_FEC_packets/Sent_Total_packets
	# "Recv_FEC_packets/Recv_Total_packets
	# "Throughput(Mbps),"
	# "Total_Average_Latency(s
	# "BP_Frame_Loss_Rate(%),"
	# "BP_Frame_Loss_Rate_AfterCorrection(%),"
	# "FrameHighQuality,"
	# "FrameLowQuality,"
	# "RecvFrameHighQuality,"
	# "RecvFrameLowQuality,"

#############################################

	rows = []
	values = []
	for line in report_list:
		rows.append(line[0])
		values.append(line[1])

	d = {'Parameters': rows, columnvalue:values}
	df = pd.DataFrame(data=d)

	fig, ax = plt.subplots()

	# hide axes
	fig.patch.set_visible(False)
	ax.axis('off')
	ax.axis('tight')

	# df = pd.DataFrame(np.random.randn(10, 4), columns=list('ABCD'))

	ax.table(cellText=df.values, colLabels=df.columns, loc='center')

	fig.tight_layout()

	ax.set_title(
		'A title for our table!',
		loc='left',
		fontsize=18,
		weight='bold'
	)	

	# plt.show()
	plt.savefig(outputName)


			
			

