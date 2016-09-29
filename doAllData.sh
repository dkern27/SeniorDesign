echo Name the directory you wish to output to: 
read outDirectory

for file in ${1}*.dat; do
        filename=$( echo $file |tr "/" "\n"| tail -n 1 )
        echo $filename
        #/anamu -o  -i $file  
        done
