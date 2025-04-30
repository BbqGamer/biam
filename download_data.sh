mkdir -p data
wget https://qaplib.mgi.polymtl.ca/data.d/qapdata.tar.gz -O data/qapdata.tar.gz
wget https://qaplib.mgi.polymtl.ca/soln.d/qapsoln.tar.gz -O data/qapsoln.tar.gz
cd data
tar xf qapdata.tar.gz
tar xf qapsoln.tar.gz

