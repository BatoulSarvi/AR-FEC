
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

#!/bin/bash


mkdir SimulationResults

AAT_Constant=1.0
changing_value_PLR=2
nStaNode=2

for ErrorModel in Table     # Yans Nist    scenario
do
	for PropLossModel in 1        #0 1 2 3     (Random: 0, Friis:1, LogDistance: 2, TworayGround: 3)     scenario
	do
		for Dist in 400 600 800    	#400 600 800 900     scenario
		do	
			for staticFEC in 0     
			do 
				for StaticQualityFEC in No  # High Low
				do 
					for appThreshold in 1.0    #1.0 1.25 1.5 2.0		
					do
						for LocationEcho in 16    # sending Ping packet in the middle of GOP
						do 
							for AAT_Factor in 0.8     
							do 
								for Seed in 1 2 3 4 5 6 7 8 9 10 
								do
									mySeed="$(shuf -i 2000000-65000000 -n 1)"
									mkdir 1_result
									echo "FEC staticFEC:$staticFEC StaticQualityFEC:$StaticQualityFEC ErrorModel:$ErrorModel PropLossModel:$PropLossModel Dist:$Dist appThreshold:$appThreshold LocationEcho:$LocationEcho AAT_Factor:$AAT_Factor AAT_Constant:$AAT_Constant changing_value_PLR:$changing_value_PLR Seed:$Seed"
									export 'NS_LOG=EvalvidFecServer=level_function|prefix_func|prefix_time:EvalvidFecClient=level_function|prefix_func|prefix_time'						
									./waf --run "scratch/My-scenari-Drones.cc --withFEC=true --staticFEC=$staticFEC --StaticQualityFEC=$StaticQualityFEC --myErrorModel=$ErrorModel --inputPropLoss=$PropLossModel --maxDist=$Dist --appThreshold=$appThreshold --LocationEcho=$LocationEcho --AAT_Factor=$AAT_Factor --AAT_Constant=$AAT_Constant --changing_value_PLR=$changing_value_PLR --nStaNode=$nStaNode --seed=$mySeed" > 1_result/myScenarioDrone.out 2>&1								
									mkdir a-FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor-$Seed
									mv 1_result a-FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor-$Seed
									mv a-FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor-$Seed SimulationResults
								done
							done
						done
					done
				done
			done
		done
	done
done


# #---------------------------------------------------------------- NO FEC ---------------------------------------------

# # mkdir SimulationResults
for ErrorModel in Table     # Yans Nist 
do
	for PropLossModel in 1      #0 1 2 3
	do
		for Dist in 400 600 800 
		do
			for Quality in High Low   
			do
				for appThreshold in 1.0   # 0.5 1.0 1.5      	
				do
					for Seed in 1 2 3 4 5 6 7 8 9 10 
					do
						mySeed="$(shuf -i 2000000-65000000 -n 1)"
						mkdir 1_result
						echo "NoFEC  Quality:$Quality  ErrorModel:$ErrorModel PropLossModel:$PropLossModel Dist:$Dist appThreshold:$appThreshold Seed:$Seed" 				
						export 'NS_LOG=EvalvidServer=level_function|prefix_func|prefix_time:EvalvidClient=level_function|prefix_func|prefix_time'
						./waf --run "scratch/My-scenari-Drones.cc --withFEC=false --quality_noFEC=$Quality --myErrorModel=$ErrorModel --inputPropLoss=$PropLossModel --maxDist=$Dist --nStaNode=$nStaNode --seed=$mySeed" > 1_result/myScenarioDrone.out 2>&1				
						mkdir a-NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-0-$Seed
						mv 1_result a-NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-0-$Seed
						mv a-NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-0-$Seed SimulationResults
					done
				done
			done  
		done 
	done 
done 
