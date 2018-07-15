#!/bin/bash

if [[ -z $1 ]] || [[ -z $2 ]] || [[ -z $3 ]] || [[ -z $4 ]]
then
	echo "Error: Error at script arguments"
	exit 1
fi

if [ ! -d $1 ]; then 
	echo "Error: Directory does not exist"
	exit 1
fi

if [ ! -e $2 ]; then
	echo "Error: File does not exist"
	exit 1
fi

if [ ! -f $2 ]; then 
	echo "Error: File is not regular"
	exit 1
fi

lines=$(wc -l < $2)
limit=10000
if [ "$lines" -lt "$limit" ]; then 
	echo "Error: Fines not enough"
	exit 1
fi
re='^[0-9]+$'
if ! [[ $3 =~ $re ]] || ! [[ $4 =~ $re ]] ; then
	echo "Error: Parameter not a positive number"
	exit 1
fi

if [ "$(ls -A $1)" ]; then
     echo "Warning: Directory is full, purging.. "
fi

root_dir=$1
fileName=$2
websites=$3
pages=$4

let f=($pages/2)+1 #size of the f set
let q=($websites/2)+1 #size of the q set
let linksPerPage=$f+$q

declare -A page_names #2d array, with page names of each website
declare -A site_names #1d array, with the names of the websites
count=0
to_delete="$root_dir/*"
rm -rf $to_delete #purge the whole directory 
while [ $count -lt "$websites" ]; do #put the names of the pages in arrays
		# we do this because we cant create names and get f,q sets at the same loop
		# we need to know all the page names first
	direc="site$count"
	newEntry="$root_dir/$direc"
	site_names[${#site_names[@]}]=$newEntry
	count1=0
	while [ $count1 -lt "$pages" ]; do
		myfile="$newEntry/page${count}_${RANDOM}.html"
		page_names[$count,$count1]=$myfile
		count1=$(($count1+1))
	done
	count=$(($count+1))
done

declare -A linksToSites #2d, represents the random selected links of each page
declare -A allpages_1d #1d array which has all the page names, not depending on website
					   #it is used to see wether all pages have internal links
count=0
page_count=0
while [ $count -lt "$websites" ]; do
	echo "Creating web site $count ..."
	mkdir -p ${site_names[$count]}
	count1=0
		while [ $count1 -lt "$pages" ]; do
			let mylimit=$lines-2000
			k=$[ ( RANDOM % $mylimit  )  + 1 ]
			m=$[ ( RANDOM % 1000  )  + 1 + 1000 ]
			myfile=${page_names[$count,$count1]} #name of page created
			echo "   Creating page $myfile with $lines lines starting at line $k"
			touch $myfile

			declare -A MyFiles #array which represents the f set

			for ((i=0;i<$pages;i++)) do #first put together all website page names
				MyFiles[$i]=${page_names[$count,$i]}
			done

			
			size=${#MyFiles[@]}
			if [ ! "${#MyFiles[@]}" -eq "$f" ]; then #if f not equal to html files
													#do not make myself link
				unset MyFiles[$count1]
			fi
			counter=0
			
			declare -A linksToWrite #array with all the links i'm gonna write

			while [ $counter -lt "$f" ]; do #while I have not as many pages I want
				randnum=$[ ( RANDOM % $size  ) ] #get random page in set

				while [ -z ${MyFiles[$randnum]} ]; do #while the file rand returned
													#is already taken or is myself
					randnum=$[ ( RANDOM % $size  ) ] #get another 
				done
				linksToWrite[${#linksToWrite[@]}]=${MyFiles[$randnum]}
				#echo "       selected ${MyFiles[$randnum]}"
				unset MyFiles[$randnum] #delete from array-mark as taken
				counter=$(($counter+1))
			done
			
			declare -A outsideWebsites #this is an array with alll external links
			for((i=0;i<$websites;i++)) do  #first put together all exteernal links
				if [ ! "$i" -eq "$count" ]; then
					for((j=0;j<$pages;j++)) do
						outsideWebsites[${#outsideWebsites[@]}]=${page_names[$i,$j]}
					done
				fi
			done
			counter=0
			mysize=${#outsideWebsites[@]}

			while [ $counter -lt "$f" ]; do #while I have not as many pages I want
				randnum=$[ ( RANDOM % $mysize  ) ] #get random page in set
				while [ -z ${outsideWebsites[$randnum]} ]; do #while the file rand returned
					randnum=$[ ( RANDOM % $mysize  ) ] #get another 
				done
				linksToWrite[${#linksToWrite[@]}]=${outsideWebsites[$randnum]}
				unset outsideWebsites[$randnum] #delete from array-mark as taken
				counter=$(($counter+1))
			done

			echo "<!DOCTYPE html>" >> $myfile
			echo "<html>" >> $myfile
			echo "<body>" >> $myfile
			
			
			let start=$k
			let range=$m/$linksPerPage
			let mylimit=$start+$range
			numoflink=1
			for var in ${linksToWrite[@]}; do
				echo "     Adding link to root $var"
				#get the random content from file, starting at start until mylimit
				content=$(sed -n -e "$start,$mylimit p" -e "$mylimit q" $fileName)	
				newVar="${var#*/}"
				echo "$content <a href=\"/$newVar\">" $var "</a><br>" >> $myfile
				let start=$mylimit		#go to the nexgt m/(f+q) lines
				let mylimit=$mylimit+$range
				let index=numoflink-1
				linksToSites[$page_count,$index]=$var
				numoflink=$(($numoflink+1))
			done
			echo "</body>" >> $myfile
			echo "</html>" >> $myfile
			allpages_1d[${#allpages_1d[@]}]=$myfile
			#clear arrays
			outsideWebsites=()
			linksToWrite=()
			count1=$(($count1+1))
			page_count=$(($page_count+1))
		done
		count=$(($count+1))
done

for((i=0;i<$page_count;i++)); do #for each page
	found=0
	myName=${allpages_1d[$i]}
	for((k=0;k<$page_count;k++)); do #check all pages
		if [ ! "$k" -eq "$i" ]; then  #except for myself
			for((j=0;j<$linksPerPage;j++)); do #check their links
				if [ "$myName" == "${linksToSites[$k,$j]}" ]; then #if i am link
					let found=$found+1 # i'm ok
				fi
			done
		fi
	done
	if [ "$found" -eq 0 ]; then  #i'm not link anywhere, report message and exit
		echo "Not all pages have at least one incoming link"
		echo "Done."
		exit 1
	fi
done
echo "All pages have at least one incoming link"
echo "Done."

