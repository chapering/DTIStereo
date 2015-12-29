
: '
this script, presuming that all executables and data have been readyed in the
configurable directories as follows, boosts a run of the whole study of seeding
methods in DTI tractography.
--- this version is proprietarily written for formal seeding study.
'
BASEDIR=`pwd`

BINDIR=${BASEDIR}/bin
TASKDIRS="pretask task1 task2 task3 task4 task5 task6"
EXECUTABLES="singleitr multiviewitr multiwindowitr"

DATADIR=${BASEDIR}/data
ALLNORMALCASEDATADIRS="${DATADIR}/normal_whole ${DATADIR}/normal_allfb ${DATADIR}/normal_partial"
BUNDLES="cst cg cc ifo ilf"

LOG=${BASEDIR}/studylog_blk2

ORDERARRAY=()
NUMROW=0 #number of row in the order matrix
NUMCOL=0 #number of column in the order matrix
#FBARRARY=(cst cg cc ifo ilf)
CURORDER=""
CURFB=""

TASKPROGIDFILE=${BASEDIR}/taskprogidfile
TASKNO=0

TUBERADIUS="0.2"

function checkbins()
{
	if [ ! -d ${BINDIR} ];then
		echo "FATAL: directory ${BINDIR} NOT found, please set up firstly."
		return 1
	fi

	for ts in ${TASKDIRS}
	do
		for bin in ${EXECUTABLES}
		do
			if [ ! -s ${BINDIR}/${ts}/${bin} ];then
				echo "ERROR: executable ${bin} for ${ts} NOT found,"
			    echo "please set up firstly."
				return 1
			fi
		done
	done
	return 0
}

function checkdata()
{
	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		for bdir in ${NORMALCASEDATADIR}
		#${ABNORMALCASEDATADIR}
		do
			if [ ! -d ${bdir} ];then
				echo "ERROR: data directory ${bdir} NOT found."
				return 1
			fi

			: ' now we totally discard the whole brain models
			for ((n=1;n<=5;n++));do
				if [ `ls ${bdir}/normal_s${n}.data | wc -l` -le 0 ];then
					echo "ERROR: data of resolution ${n}x${n}x${n} NOT found in the"
					echo "directory ${bdir}"
					return 1
				fi
			done
			'

			# check task-specific data
			for ts in ${TASKDIRS}
			do
				if [ $ts = "pretask" ];then
					continue
				fi

				taskdatadir="region_for_${ts}"
				for ((n=1;n<=5;n++));do
					for fb in ${BUNDLES}
					do
						# cc and ifo not used in task6
						if [ $ts = "task6" ];then
							if [ $fb = "cc" -o $fb = "ifo" ];then
								continue
							fi
						fi

						if [ `ls -R ${bdir}/${taskdatadir}/s${n}/${fb}/ | grep data | wc -l` -lt 2 ];then
							echo "ERROR: data missed for ${ts}/s${n}/${fb}."
							return 1
						fi
					done
				done
			done
		done
	done

	return 0
}
: '
$1 - task order text file
$? - 0 for success and 1 otherwise
'
function readorder()
{
	if [ $# -lt 1 -o ! -s $1 ];then
		echo "In readorder: too few arguments or file does not exist."
		return 1
	fi

	: '
	take the number of fields in the first line as the number of column  of
	the matrix to read into
	'
	local fldnum=0
	local currow=0
	#cat $1 | \
	while read curline
	do
		curfldnum=`echo $curline | awk '{print NF}'`
		if [ $fldnum -eq 0 ];then
			fldnum=$curfldnum
		elif [ $curfldnum -ne $fldnum ];then
			echo "inconsistent row - different number of columns."
			return 1
		fi

		local currol=0
		for num in $curline
		do
			let "index = $currow * $fldnum + $currol"
			ORDERARRAY[$index]=$num
			let "currol += 1"
		done

		let "currow += 1"

	done < $1

	let NUMROW=$currow
	let NUMCOL=$fldnum
	#echo "NUMCOL=$NUMCOL"
	#echo "NUMROW=$NUMROW"

	return 0
}

function print_order()
{
	echo ${ORDERARRAY[*]} | xargs -n $NUMCOL
}

function taskflag()
{
	echo -e "\n##############################################################" >> $LOG
	echo "                           TASK $1                            " >> $LOG
	echo -e "##############################################################\n" >> $LOG
}

function updatetaskprog()
{
	let "TASKNO += 1"
	echo -e "\n\n       $TASKNO / 180" > ${TASKPROGIDFILE}
}

#-----------------------------------------#
#
# pretask
#
#-----------------------------------------#
function pretask()
{
	: '
	this is a trivial, actually virtual, task. It just show the introductory
	message of the tasks to follow
	'
	${BINDIR}/pretask/singleitr \
		-f ${NORMALCASEDATADIR}/region_for_task1/s1/cc/region_s1.data \
		-t ${BINDIR}/pretask/tasktext \
		-V 1>> $LOG 2>&1
}

#-----------------------------------------#
#
# task 1
#
#-----------------------------------------#
function task1()
{
	taskflag 1

	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		# the same participant does the second block of task data
		FBARRARY=( $(echo "cst cg cc ifo ilf" | sed -r 's/(.[^;]*;)/ \1 /g' | tr " " "\n" | shuf | tr -d " " ) )
		echo "focus order:" ${FBARRARY[*]} >> $LOG
		di=0
		for n in ${CURORDER}
		do
			updatetaskprog
			CURFB=${FBARRARY[$di]}
			flip=0
			if [ "$CURFB" == "cst" ];then
				flip=1
			fi

			let "di += 1"

			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
			${BINDIR}/task1/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/pos1/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/pos1/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/pos1/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/pos1/tumorbox_1_region_s${n}.data \
			-t ${BINDIR}/task1/tasktext \
			-p ${TASKPROGIDFILE} \
			-i ${flip} \
			-r ${TUBERADIUS} \
			-V 1>> $LOG 2>&1

			if [ $? -ne 0 ];then
				return 3
			fi
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 2 
#
#-----------------------------------------#
function task2()
{
	taskflag 2

	: '
	local keys1=(1 1 2 3 1)
	local keys2=(2 3 2 2 1)
	local keys3=(3 2 1 3 1)
	local keys4=(3 2 3 2 3)
	local keys5=(1 1 1 1 2)
	'
	local keys1=(2 2 3 3 2)
	local keys2=(1 2 2 1 2)
	local keys3=(3 2 1 3 1)
	local keys4=(3 2 3 2 3)
	local keys5=(2 2 3 2 3)

	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		# the same participant does the second block of task data
		sfb=(cst cg cc ifo ilf)
		dices=( $(echo "0 1 2 3 4" | sed -r 's/(.[^;]*;)/ \1 /g' | tr " " "\n" | shuf | tr -d " " ) )
		FBARRARY=( ${sfb[${dices[0]}]} ${sfb[${dices[1]}]} ${sfb[${dices[2]}]} ${sfb[${dices[3]}]} ${sfb[${dices[4]}]} )
		echo "focus order:" ${FBARRARY[*]} >> $LOG
		di=0
		for n in ${CURORDER}
		do
			updatetaskprog
			CURKEY=$(eval echo \${keys$((${dices[$di]}+1))[$((n-1))]})

			CURFB=${FBARRARY[$di]}

			let "di += 1"

			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
			${BINDIR}/task2/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/pos1/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/pos1/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/pos1/fiberidx_*_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/pos1/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/pos1/tumorbox_1_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/pos1/tumorbox_2_region_s${n}.data \
			-t ${BINDIR}/task2/tasktext \
			-k ${CURKEY} \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-V 1>> $LOG 2>&1

			if [ $? -ne 0 ];then
				return 3
			fi
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 3 
#
#-----------------------------------------#
function task3()
{
	taskflag 3

	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		# the same participant does the second block of task data
		sfb=(cst cg cc ifo ilf)
		dices=( $(echo "0 1 2 3 4" | sed -r 's/(.[^;]*;)/ \1 /g' | tr " " "\n" | shuf | tr -d " " ) )
		FBARRARY=( ${sfb[${dices[0]}]} ${sfb[${dices[1]}]} ${sfb[${dices[2]}]} ${sfb[${dices[3]}]} ${sfb[${dices[4]}]} )
		echo "focus order:" ${FBARRARY[*]} >> $LOG
		di=0
		for n in ${CURORDER}
		do
			updatetaskprog
			CURFB=${FBARRARY[$di]}

			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
			${BINDIR}/task3/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/pos1/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/pos1/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/pos1/fiberidx_*_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-k $((${dices[$di]}+1)) \
			-t ${BINDIR}/task3/tasktext \
			-V 1>> $LOG 2>&1

			if [ $? -ne 0 ];then
				return 3
			fi

			let "di += 1"
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 4 
#
#-----------------------------------------#
function task4()
{
	taskflag 4

	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		# the same participant does the second block of task data
		FBARRARY=( $(echo "cst cg cc ifo ilf" | sed -r 's/(.[^;]*;)/ \1 /g' | tr " " "\n" | shuf | tr -d " " ) )
		echo "focus order:" ${FBARRARY[*]} >> $LOG
		di=0
		for n in ${CURORDER}
		do
			updatetaskprog
			CURFB=${FBARRARY[$di]}

			let "di += 1"

			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
			${BINDIR}/task4/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/pos1/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/pos1/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/pos1/tumorbox_0_region_s${n}.data \
			-t ${BINDIR}/task4/tasktext \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-V 1>> $LOG 2>&1

			if [ $? -ne 0 ];then
				return 3
			fi
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 5 
#
#-----------------------------------------#
function task5()
{
	taskflag 5

	#local binchoices=(no yes yes no yes)
	local binchoices1=(yes no no no yes)
	local binchoices2=(yes yes no yes yes)
	local binchoices3=(no no no no yes)
	local binchoices4=(yes yes yes yes no)
	local binchoices5=(no no no no yes)
	local binchoices6=(yes yes no yes no)
	local binchoices7=(no no no yes yes)
	local binchoices8=(yes yes no no yes)
	local binchoices9=(yes no yes no yes)
	local binchoices10=(yes yes no yes yes)

	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		let "pidx = pi + 1"
		let "pidx2 = pidx + $NUMCOL"

		# the same participant does the second block of task data
		FBARRARY=( $(echo "cst cg cc ifo ilf" | sed -r 's/(.[^;]*;)/ \1 /g' | tr " " "\n" | shuf | tr -d " " ) )
		echo "focus order:" ${FBARRARY[*]} >> $LOG
		di=0
		for ((i=0;i<$NUMCOL;i++))
		do
			updatetaskprog
			let "index = $pi * $NUMCOL + $i"
			n=${ORDERARRAY[index]}

			CURFB=${FBARRARY[$di]}
			CURCHOICE=$(eval echo \${binchoices${pidx2}[$i]})

			let "di += 1"

			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
			echo "#####    when the answer is $CURCHOICE #### ">> $LOG
			FIBERIDXFILE="fiberidx_*_region_s${n}.data"
			if [ "$CURCHOICE" == "no" ];then
				FIBERIDXFILE="fiberidx_compound_region_s${n}.data"
			fi

			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/pos1/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/pos1/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/pos1/${CURCHOICE}/${FIBERIDXFILE} \
			-t ${BINDIR}/task5/tasktext \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-V 1>> $LOG 2>&1

			if [ $? -ne 0 ];then
				return 3
			fi
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 6
#
#-----------------------------------------#
function task6()
{
	taskflag 6

	for NORMALCASEDATADIR in  ${ALLNORMALCASEDATADIRS}
	do
		# the same participant does the second block of task data
		FBARRARY=( $(echo "cst cg cc ifo ilf" | sed -r 's/(.[^;]*;)/ \1 /g' | tr " " "\n" | shuf | tr -d " " ) )
		echo "focus order:" ${FBARRARY[*]} >> $LOG
		di=0
		for n in ${CURORDER}
		do
			updatetaskprog
			CURFB=${FBARRARY[$di]}

			let "di += 1"

			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
			m=`expr $RANDOM % 5`
			let "m++"
			echo "compared with seeding resolution of ${m}x${m}x${m}, bundle of $CURFB" >> $LOG
			key=1
			if [ $m -ne $n ];then
				key=2
			fi
			${BINDIR}/task6/multiviewitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/pos1/region_s${n}.data \
			-f \
			${NORMALCASEDATADIR}/region_for_task4/s${m}/${CURFB}/region_s${m}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/pos1/skeleton_region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task4/s${m}/${CURFB}/skeleton_region_s${m}.data \
			-t ${BINDIR}/task6/tasktext \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-k ${key} \
			-V 1>> $LOG 2>&1

			if [ $? -ne 0 ];then
				return 3
			fi
		done
	done
	return 0
}

function costOfThisTask()
{
	s=$1
	e=$2
	((d=e-s))
	((h=d/3600))
	((m=d%3600/60))
	((s=d%3600%60))
	echo " Time cost of this task: $h hours $m minutes $s seconds." >> $LOG
}

function execTasks()
{
	> $LOG

	bstart=`date +%s`

	for curtask in ${TASKDIRS}
	do 
		curstart=`date +%s`
		$curtask	
		ret=$?
		curend=`date +%s`
		costOfThisTask ${curstart} ${curend}

		if [ $ret -ne 0 ];then
			return 3
		fi
	done

	end=`date +%s`
	echo -e "\n################## ALL FINISHED #######################" >> $LOG
	((d=end-bstart))
	((h=d/3600))
	((m=d%3600/60))
	((s=d%3600%60))
	echo " Time cost: $h hours $m minutes $s seconds." >> $LOG
	return 0
}

#####################################################################################
###    Task ordering and main flow control
###
#####################################################################################

checkbins
if [ $? -ne 0 ];then
	exit 1
fi

checkdata
if [ $? -ne 0 ];then
	exit 1
fi

if [ $# -lt 2 ];then
	echo "Usage: $0 <taskorder> <participant index>"
	exit 1
fi

orderfn=$1
pi=$2
readorder ${orderfn}
if [ $? -ne 0 ];then
	exit 1
fi

#print_order

if [ $pi -gt $NUMROW ];then
	echo "Participant index out of bound."
	exit 1
fi
LOG="${LOG}_p${pi}"
let "pi -= 1"

for((i=0;i<$NUMCOL;i++))
do
	let "index = $pi * $NUMCOL + $i"
	CURORDER="$CURORDER ${ORDERARRAY[index]}"
done

execTasks
ret=$?

rm -rf ${TASKPROGIDFILE}

if [ $ret -eq 0 ];then
	echo "All finished, thank you!"
else
	echo "Study terminated in advance, thank you all the same."
fi
echo

exit 0

