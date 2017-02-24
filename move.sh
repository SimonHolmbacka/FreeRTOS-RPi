mount -t vfat /dev/sdf1 /mnt/FreeRTOS
sleep 1
rm /mnt/FreeRTOS/kernel.img
sleep 1
cp kernel7.img /mnt/FreeRTOS/kernel.img
sleep 1
umount /mnt/FreeRTOS
sync
