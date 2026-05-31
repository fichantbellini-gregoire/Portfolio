#!/usr/bin/bash

for fichier in *.jpg *.png 
do
    widthImage=$(identify "$fichier" | cut -d ' ' -f 3 | cut -d 'x' -f 1)
    heightImage=$(identify "$fichier" | cut -d ' ' -f 3 | cut -d 'x' -f 2)
    widthMaxi=900
    heightMaxi=620
    widthMini=350
    heightMini=250
    poidsMax=180000


    if [[ $widthImage -lt $widthMini ]] || [[ $heightImage -lt $heightMini ]]; then
        echo "l'image est trop petite"
        mv "./$fichier" "./Images_non_conformes/"$fichier""
    fi
    
    if [[ $widthImage -gt $widthMaxi ]] || [[ $heightImage -gt $heightMaxi ]]
        then
        ratioWidth=$(echo "$widthImage / $widthMaxi" | bc -l)
        ratioHeight=$(echo "$heightImage / $heightMaxi" | bc -l)
        test=$(echo "$ratioWidth < $ratioHeight" | bc -l)

        if [[ $test -eq 1 ]]; then 
            newWidthImage=$(echo "$widthImage / $ratioHeight" | bc -l)
            newHeightImage=$(echo "$heightImage / $ratioHeight" | bc -l)
        else
            newWidthImage=$(echo "$widthImage / $ratioWidth" | bc -l)
            newHeightImage=$(echo "$heightImage / $ratioWidth" | bc -l)
        fi

        newWidthImage=$(echo "$newWidthImage" | cut -d '.' -f 1)
        newHeightImage=$(echo "$newHeightImage" | cut -d '.' -f 1)
        test2=$(echo "$newWidthImage <= $widthMaxi && $newHeightImage <= $heightMaxi" | bc -l)
        if [[ $test2 -eq 0 ]]; then
            mv "./$fichier" "./Images_non_conformes/"$fichier""
        else
            convert "$fichier" -resize "${newHeightImage}"x"${newWidthImage}" "${fichier%%.*}.webp"
        fi
    fi


    if [[ $widthImage -le $widthMaxi ]] && [[ $widthImage -ge $widthMini ]] && [[ $heightImage -le $heightMaxi ]] && [[ $heightImage -ge $heightMini ]]
    
    then
        convert "$fichier" "${fichier%%.*}.webp"
    fi

done

for i in *.webp
do
    poidsMax=180000
    poidsImage=$(stat -c%s $i)
    if [[ $poidsImage -le $poidsMax ]]
    then
        mv ./"$i" ./Images_conformes
    else
        mv ./"$i" ./Images_non_conformes
    fi
done