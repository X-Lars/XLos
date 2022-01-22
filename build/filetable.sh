filename="$1"
filesize="$2"

filetable=bin/filetable.bin

printf "%s" "$filename" >> $filetable

i=${#filename} # Get length of variable

# While i less than 10
while [ $i -lt 10 ]
    do
        echo -n " " >> $filetable
        i=$((i + 1))
    done

# File Extension
case "$filename" in
    filetable) # Case filename = filetable
        printf "%s" "txt" >> $filetable
        ;;
    font) # case filename = font
        printf "%s" "fnt" >> $filetable
        ;;
    *)  # case else
        printf "%s" "bin" >> $filetable
        ;;
esac

# Directory entry
printf "%b" "\0" >> $filetable

# Sector

sector=$(cat bin/sector.tmp) # Get decimal string
sector=$(printf "%o" $sector) # Convert decimal to hex
printf "%b" "\0$sector" >> $filetable # Output hex string as numeric byte

sector=$(printf "%d" 0$sector) # Convert hex string to decimal string
sector=$((sector + filesize)) # Set next starting sector (current + file size)
printf "%d" $sector > bin/sector.tmp # Store 

# Sectors
filesize=$(printf "%o" $filesize) # Convert decimal to hexadesimal
printf "%b" "\0$filesize" >> $filetable