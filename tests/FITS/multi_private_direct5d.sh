~/../perma/drop_buffer_cache
File=/mnt/intel/xiao/direct/test
File1=/mnt/intel/xiao/real/asteroid_sim_epoch
env LD_LIBRARY_PATH=/home/liu61/qfits/lib ~/develop/install/bin/private5d -f $File --directio -p 1048580 -n 100 -b 2000000 -t 8
#env LD_LIBRARY_PATH=/home/liu61/qfits/lib ~/develop/install/bin/private -f $File1 -p 41000 -n 10 -b 100000 -t 8
#r -f /mnt/intel/xiao/asteroid_sim_epoch -p 1050000 -n 50 -b 100000 -t 8
#r -f /mnt/intel/xiao/real/test --directio -p 1048580 -n 100 -b 1700000 -t 8
