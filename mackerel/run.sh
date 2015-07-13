#/bin/bash

pathname=$(dirname $0)
mackerel=$pathname/Main.hs
out_directory=generated

echo "mackerel $1 > $out_directory/$2"
runghc $mackerel -v $1 > $out_directory/$2
