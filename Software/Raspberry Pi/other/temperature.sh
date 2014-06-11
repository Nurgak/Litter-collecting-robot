/opt/vc/bin/vcgencmd measure_temp
echo "Temperature: "
cat /sys/class/thermal/thermal_zone0/temp
