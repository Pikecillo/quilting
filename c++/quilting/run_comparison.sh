#!/bin/sh

#echo "Quilting" > comparison_results
#for file in T1.tif T2.tif T3.tif T4.tif T5.tif T6.tif T7.tif T8.tif T9.tif
#do
#	name=`basename images_report/$file .tif`
#	command="./metric images_report/$file images_report/${name}_200.tif"
#	echo $command
#	results=`$command`
#	echo $results
#	echo $results >> comparison_results
#done

echo "Efros-Leung" >> comparison_results
for file in T3.tif T5.tif T6.tif T7.tif T8.tif T9.tif
do
	name=`basename images_report/$file .tif`
	command="./metric images_report/$file images_report/${name}-el_out.tif"
	echo $command
	results=`$command`
	echo $results
	echo $results >> comparison_results
done

#echo "Efros-Leung" >> comparison_results
#for file in T1.tif T2.tif T3.tif T4.tif T5.tif T6.tif T7.tif T8.tif T9.tif
#do
#	name=`basename images_report/$file .tif`
#	echo "./metric images_report/$file images_report/$name-out.tif"
#done