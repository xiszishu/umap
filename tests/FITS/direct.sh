~/../perma/drop_buffer_cache
File=/mnt/intel/xiao/direct/test
File1=/mnt/intel/xiao/real/asteroid_sim_epoch
env LD_LIBRARY_PATH=/home/liu61/qfits/lib ~/develop/install/bin/direct --directio -f $File -p 2100000 -n 100 -b 270000 -t 8
#env LD_LIBRARY_PATH=/home/liu61/qfits/lib ~/develop/install/bin/private -f $File1 -p 41000 -n 10 -b 100000 -t 8
#r -f /mnt/intel/xiao/direct/test --directio -p 1050000 -n 100 -b 100000 -t 8
