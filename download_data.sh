mkdir -p data
wget https://qaplib.mgi.polymtl.ca/data.d/qapdata.tar.gz -O data/qapdata.tar.gz
wget https://qaplib.mgi.polymtl.ca/soln.d/qapsoln.tar.gz -O data/qapsoln.tar.gz
cd data
tar xf qapdata.tar.gz
tar xf qapsoln.tar.gz

# some instances are invalid for some reason 
# e.g. lower bound score given doesnt match feasible solution score 
mkdir blacklist -p
for inst in esc128 kra30a kra30b kra32 ste36c tai40a tai60a tai80a tho150 tho30; do
    mv $inst.dat blacklist
    mv $inst.sln blacklist
done

