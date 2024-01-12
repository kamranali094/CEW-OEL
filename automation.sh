#!/bin/bash

# Set the path to your compiled C program
EXECUTABLE_PATH="./oel"

# Run the program every hour for 24 hours
for ((i=0; i<24; i++))
do
    echo "Running program - Hour: $((i+1))"
    $EXECUTABLE_PATH
    sleep 3600  # Sleep for an hour (3600 seconds)

    highestTemp=-999.0
    lowestTemp=999.0
    totalTemp=0.0
    count=0

    # Read temperature data from processedData.txt
    temperatureData=($(cat processedData.txt))

    # Variables for real-time alerts
    criticalLow=10.0
    criticalHigh=35.0

    # Flag to track whether an alert needs to be sent
    sendAlert=false
    alertMessage=""

    for temp in "${temperatureData[@]}"
    do
        # Update highest and lowest temperatures
        if (( $(echo "$temp > $highestTemp" | bc -l) )); then
            highestTemp=$temp
        fi

        if (( $(echo "$temp < $lowestTemp" | bc -l) )); then
            lowestTemp=$temp
        fi

        # Accumulate temperatures and count for average calculation
        totalTemp=$(echo "$totalTemp + $temp" | bc -l)
        ((count++))

        # Check for critical temperature conditions
        if (( $(echo "$temp < $criticalLow" | bc -l) )); then
            sendAlert=true
            alertMessage="Weather is too cold. Take necessary precautions!"
        elif (( $(echo "$temp > $criticalHigh" | bc -l) )); then
            sendAlert=true
            alertMessage="Weather is too hot. Take necessary precautions!"
        fi
    done

    # Calculate average temperature using bc
    averageTemp=$(echo "$totalTemp / $count" | bc -l)

    # Save temperature report to a file
    report="Country: Pakistan\nCity: Karachi\nHighest Temperature: $highestTemp\nLowest Temperature: $lowestTemp\nAverage Temperature: $averageTemp"

    echo -e "$report" > temperatureReport.txt

    if [ "$sendAlert" = true ]; then
        if (( $(echo "$averageTemp < $criticalLow" | bc -l) )); then
            alertMessage="Weather is too cold, Take necessary precautions"
        elif (( $(echo "$averageTemp > $criticalHigh" | bc -l) )); then
            alertMessage="Weather is too hot, Take necessary precautions"
        else
            alertMessage="Critical environmental readings detected!"
        fi

        # Prepare the email content in mail.txt
        echo -e "Subject: Temperature Alert\nFrom: maaz24400@gmail.com\nTo: tslmn12345@gmail.com\n\nALERT: $alertMessage" > mail.txt

        # Display the alert message on the console and append it to log.txt
        echo "ALERT: $alertMessage" | tee -a log.txt

        # Use curl to send an email
        curl --ssl-reqd --url 'smtps://smtp.gmail.com:465' --user 'maaz24400@gmail.com:lceg vpif mhpk uvql' --mail-from 'maaz24400@gmail.com' --mail-rcpt 'tslmn12345@gmail.com' --upload-file mail.txt
    fi
done

# After each run, compute and save temperature report
    echo "Generating Temperature Report..."

echo "Temperature Reports Generated."
