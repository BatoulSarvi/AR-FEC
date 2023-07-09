
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
	# PLR_file_name = sys.argv[1]
	PLR_file_name = 'outputPLRFile'
	# num_GOP = int(sys.argv[2])
	# outputName = sys.argv[2]
	outputName = 'PLR_NoFEC.png'

	inputLabel = sys.argv[1]

	PLR_file = open(PLR_file_name, "r")

	lines = PLR_file.readlines()
	PLR_list = [l.rstrip("\n").split(',') for l in lines]       # gop , actual plr

	gopArray = [0] * len(PLR_list)
	actualPLRArray = [0] * len(PLR_list)
	# predPLRArray = [0] * len(PLR_list)
	
	for i in range (len(PLR_list)): 				 # gop , actual plr
		gopArray[i] = int(PLR_list[i][0])
		if (PLR_list[i][1] == '222'):
			actualPLRArray[i] = 100
		else:	
			actualPLRArray[i] = int(PLR_list[i][1])
		# predPLRArray[i] = int(PLR_list[i][2])



	font1 = {'family':'serif','color':'blue','size':12}
	font2 = {'family':'serif','color':'darkred','size':12}

	plt.plot(actualPLRArray, marker='*', linestyle='-', color='orange', ms = 5, mec = 'orange', mfc = 'orange', label=inputLabel)
	# plt.plot(predPLRArray, marker='+', linestyle='-', color='hotpink', ms = 5, mec = 'hotpink', mfc = 'hotpink', label='Predicted PLR')
	plt.xlabel('GOP', fontdict = font2)
	plt.xlim([0, len(PLR_list)])
	plt.ylabel('PLR (%)',fontdict = font2)
	# plt.legend(bbox_to_anchor=(1.05, 1),loc='upper left', borderaxespad=0.)
	plt.legend(bbox_to_anchor =(0.75, 1.15), ncol = 3)
	# plt.legend(loc='lower center', ncol = 3, bbox_to_anchor=(1.05, 1))
	
	
	# plt.show()
	plt.savefig(outputName)

# #closing all files

			
			

