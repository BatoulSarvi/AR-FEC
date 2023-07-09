
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

outputputHistoryFilename='outputHistoryReport_All'

cd SimulationResults

for ErrorModel in Table     # Yans Nist    scenario
do
	for PropLossModel in 1        #0 1 2 3     (Random: 0, Friis:1, LogDistance: 2, TworayGround: 3)     scenario
	do
		for Dist in 400 600 800     #400 600 800 
		do	
			for staticFEC in 0 		
			do 
				for StaticQualityFEC in No High Low   	
				do 
					for appThreshold in 1.0     #0.5 1.0 1.5 	
					do
						for LocationEcho in 16     
						do
							for AAT_Factor in 0.8     #0.2 0.3 0.4    #factor for  AAT = AT ( 0.8 - ( AAT_Factor * consecutiveDelayedRTT))
							do  
								echo a-FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor
								for Seed in 1 2 3 4 5 6 7 8 9 10 
								do
									echo "Seed:$Seed"
									cd a-FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor-$Seed
									cd 1_result
									python3 ../../../scratch/PostRun.py 72
									python3 ../../../scratch/PlotReportTable.py FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor-$Seed
									python3 ../../../scratch/PlotPLR.py 
									python3 ../../../scratch/PlotLatency.py FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor-$Seed
									cd ..
									cd ..
								done							
								python3 ../scratch/HistoryReportofSeeds.py a-FEC-$staticFEC-$StaticQualityFEC-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$LocationEcho-$AAT_Factor 10
							done
						done
					done
				done
			done
		done
	done
done
cd ..

# # #------------------------------------------------------------------   NO FEC -----------------------------------------

cd SimulationResults
for ErrorModel in Table     # Yans Nist 
do
	for PropLossModel in 1      #0 1 2 3
	do
		for Dist in 400 600 800
		do
			for Quality in High Low  
			do
				for appThreshold in 1.0  #0.5 1.0 1.5   
				do
					echo a-NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-0
					for Seed in 1 2 3 4 5 6 7 8 9 10 
					do	
						echo "Seed:$Seed" 				
						cd a-NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-0-$Seed
						cd 1_result
						python3 ../../../scratch/PostRunNoFEC.py 72 $Quality
						python3 ../../../scratch/PlotReportTable.py NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-$Seed	
						python3 ../../../scratch/PlotPLR-NoFEC.py  PLR-NOFEC-$Quality-$Dist-$appThreshold-$Seed	
						python3 ../../../scratch/PlotLatency.py NOFEC-$Quality-$Dist-$appThreshold-$Seed
						cd ..
						cd ..
					done
					python3 ../scratch/HistoryReportofSeeds.py a-NoFEC-$Quality-$ErrorModel-$PropLossModel-$Dist-$appThreshold-0 10
				done
			done  
		done 
	done 
done 
cd ..

#------------------------------------------------------------------ Create output files of all runs on AFEC   -----------------------------------------

cd SimulationResults
echo "creating an output file of all runs on AFEC ... "
python3 ../scratch/HistoryReportofAllRuns-AFEC.py $outputputHistoryFilename  				
cd ..

# #------------------------------------------------------------------ Create output files of all runs on NoFEC   -----------------------------------------

cd SimulationResults
echo "creating an output file of all runs on NoFEC ... "
python3 ../scratch/HistoryReportofAllRuns-NoFEC.py $outputputHistoryFilename 
cd ..

#--------------------------------------------------------  Plot phase ----------------------------------------------------
cd SimulationResults

Dist=(400 600 800)
current_directory=$(pwd)
echo "Current directory: $current_directory" 
echo "************************"
LocationEcho=16
appThreshold=(0.5 1.0 1.5)
staticFEC=0
ErrorModel=Table
PropLossModel=1
aat_factor=0.8

echo "plot ave frame latency "
python3 ../scratch/PlotAverageFrameLatency.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC-$staticFEC-No-$ErrorModel-$PropLossModel $LocationEcho "${appThreshold[1]}" a-NoFEC-Low-$ErrorModel-$PropLossModel a-NoFEC-High-$ErrorModel-$PropLossModel a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel a-FEC-$staticFEC-High-$ErrorModel-$PropLossModel $aat_factor "${Dist[@]}" 

echo "plot throughput and FEC packet rate "
python3 ../scratch/PlotThroughput-additiveAFEC.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC-$staticFEC-No-$ErrorModel-$PropLossModel $LocationEcho "${appThreshold[1]}" a-NoFEC-Low-$ErrorModel-$PropLossModel a-NoFEC-High-$ErrorModel-$PropLossModel a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel a-FEC-$staticFEC-High-$ErrorModel-$PropLossModel $aat_factor "${Dist[@]}"

python3 ../scratch/1-PlotAverageFrameNum.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC a-NoFEC $LocationEcho "${appThreshold[1]}" $staticFEC $ErrorModel $PropLossModel $aat_factor "${Dist[0]}" 
python3 ../scratch/1-PlotAverageFrameNum.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC a-NoFEC $LocationEcho "${appThreshold[1]}" $staticFEC $ErrorModel $PropLossModel $aat_factor "${Dist[1]}" 
python3 ../scratch/1-PlotAverageFrameNum.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC a-NoFEC $LocationEcho "${appThreshold[1]}" $staticFEC $ErrorModel $PropLossModel $aat_factor "${Dist[2]}" 

python3 ../scratch/1-PlotAverageFrameNum.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC a-NoFEC $LocationEcho "${appThreshold[0]}" $staticFEC $ErrorModel $PropLossModel $aat_factor "${Dist[2]}" 
python3 ../scratch/1-PlotAverageFrameNum.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC a-NoFEC $LocationEcho "${appThreshold[2]}" $staticFEC $ErrorModel $PropLossModel $aat_factor "${Dist[2]}" 


echo "plot ave frame latency for different application thresholds "
python3 ../scratch/PlotAverageFrameLatency-appth.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC-$staticFEC-No-$ErrorModel-$PropLossModel $LocationEcho "${Dist[2]}" a-NoFEC-Low-$ErrorModel-$PropLossModel a-NoFEC-High-$ErrorModel-$PropLossModel a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel a-FEC-$staticFEC-High-$ErrorModel-$PropLossModel $aat_factor "${appThreshold[@]}"

echo "plot throughput and FEC packet rate for different application thresholds "
python3 ../scratch/PlotThroughput-additiveAFEC-appth.py $outputputHistoryFilename""_AFEC $outputputHistoryFilename""_NoFEC a-FEC-$staticFEC-No-$ErrorModel-$PropLossModel $LocationEcho "${Dist[2]}" a-NoFEC-Low-$ErrorModel-$PropLossModel a-NoFEC-High-$ErrorModel-$PropLossModel a-FEC-$staticFEC-Low-$ErrorModel-$PropLossModel a-FEC-$staticFEC-High-$ErrorModel-$PropLossModel $aat_factor "${appThreshold[@]}"


cd ..
